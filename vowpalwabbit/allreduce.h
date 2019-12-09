// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This implements the allreduce function of MPI.
#pragma once

#include <string>
#include <algorithm>

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;
#define CLOSESOCK closesocket
namespace std
{
// forward declare promise as C++/CLI doesn't allow usage in header files
template <typename T>
class promise;

class condition_variable;

class mutex;
}  // namespace std
#else
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
typedef int socket_t;
#define CLOSESOCK close
#include <future>
#endif
#include "vw_exception.h"
#include <cassert>

constexpr size_t ar_buf_size = 1 << 16;

struct node_socks
{
  std::string current_master;
  socket_t parent;
  socket_t children[2];
  ~node_socks()
  {
    if (current_master != "")
    {
      if (parent != -1)
        CLOSESOCK(this->parent);
      if (children[0] != -1)
        CLOSESOCK(this->children[0]);
      if (children[1] != -1)
        CLOSESOCK(this->children[1]);
    }
  }
  node_socks() { current_master = ""; }
};

template <class T, void (*f)(T&, const T&)>
void addbufs(T* buf1, const T* buf2, const size_t n)
{
  for (size_t i = 0; i < n; i++) f(buf1[i], buf2[i]);
}

class AllReduce
{
 public:
  const size_t total;  // total number of nodes
  const size_t node;   // node id number
  bool quiet;

  AllReduce(size_t ptotal, const size_t pnode, bool pquiet = false) : total(ptotal), node(pnode), quiet(pquiet)
  {
    assert(node < total);
  }

  virtual ~AllReduce() = default;
};

struct Data
{
  void* buffer;
  size_t length;
};

class AllReduceSync
{
 private:
  std::mutex* m_mutex;
  std::condition_variable* m_cv;

  // total number of threads we wait for
  size_t m_total;

  // number of threads reached the barrier
  uint32_t m_count;

  // current wait-barrier-run required to protect against spurious wakeups of m_cv->wait(...)
  bool m_run;

 public:
  AllReduceSync(const size_t total);

  ~AllReduceSync();

  void waitForSynchronization();

  void** buffers;
};

class AllReduceThreads : public AllReduce
{
 private:
  AllReduceSync* m_sync;
  bool m_syncOwner;

 public:
  AllReduceThreads(AllReduceThreads* root, const size_t ptotal, const size_t pnode, bool quiet = false);

  AllReduceThreads(const size_t ptotal, const size_t pnode, bool quiet = false);

  virtual ~AllReduceThreads();

  template <class T, void (*f)(T&, const T&)>
  void all_reduce(T* buffer, const size_t n)
  {  // register buffer
    T** buffers = (T**)m_sync->buffers;
    buffers[node] = buffer;
    m_sync->waitForSynchronization();

    size_t blockSize = n / total;
    size_t index;
    size_t end;

    if (blockSize == 0)
    {
      if (node < n)
      {
        index = node;
        end = node + 1;
      }
      else
      {  // more threads than bytes --> don't do any work
        index = end = 0;
      }
    }
    else
    {
      index = node * blockSize;
      end = node == total - 1 ? n : (node + 1) * blockSize;
    }

    for (; index < end; index++)
    {  // Perform transposed AllReduce to help data locallity
      T& first = buffers[0][index];

      for (size_t i = 1; i < total; i++) f(first, buffers[i][index]);

      // Broadcast back
      for (size_t i = 1; i < total; i++) buffers[i][index] = first;
    }

    m_sync->waitForSynchronization();
  }
};

class AllReduceSockets : public AllReduce
{
 private:
  node_socks socks;
  std::string span_server;
  int port;
  size_t unique_id;  // unique id for each node in the network, id == 0 means extra io.

  void all_reduce_init();

  template <class T>
  void pass_up(char* buffer, size_t left_read_pos, size_t right_read_pos, size_t& parent_sent_pos)
  {
    size_t my_bufsize =
        std::min(ar_buf_size, std::min(left_read_pos, right_read_pos) / sizeof(T) * sizeof(T) - parent_sent_pos);

    if (my_bufsize > 0)
    {  // going to pass up this chunk of data to the parent
      int write_size = send(socks.parent, buffer + parent_sent_pos, (int)my_bufsize, 0);
      if (write_size < 0)
        THROW("Write to parent failed " << my_bufsize << " " << write_size << " " << parent_sent_pos << " "
                                        << left_read_pos << " " << right_read_pos);

      parent_sent_pos += write_size;
    }
  }

  template <class T, void (*f)(T&, const T&)>
  void reduce(char* buffer, const size_t n)
  {
    fd_set fds;
    FD_ZERO(&fds);
    if (socks.children[0] != -1)
      FD_SET(socks.children[0], &fds);
    if (socks.children[1] != -1)
      FD_SET(socks.children[1], &fds);

    socket_t max_fd = std::max(socks.children[0], socks.children[1]) + 1;
    size_t child_read_pos[2] = {0, 0};  // First unread float from left and right children
    int child_unprocessed[2] = {0, 0};  // The number of bytes sent by the child but not yet added to the buffer
    char child_read_buf[2][ar_buf_size + sizeof(T) - 1];
    size_t parent_sent_pos = 0;  // First unsent float to parent
    // parent_sent_pos <= left_read_pos
    // parent_sent_pos <= right_read_pos

    if (socks.children[0] == -1)
    {
      child_read_pos[0] = n;
    }
    if (socks.children[1] == -1)
    {
      child_read_pos[1] = n;
    }

    while (parent_sent_pos < n || child_read_pos[0] < n || child_read_pos[1] < n)
    {
      if (socks.parent != -1)
        pass_up<T>(buffer, child_read_pos[0], child_read_pos[1], parent_sent_pos);

      if (parent_sent_pos >= n && child_read_pos[0] >= n && child_read_pos[1] >= n)
        break;

      if (child_read_pos[0] < n || child_read_pos[1] < n)
      {
        if (max_fd > 0 && select((int)max_fd, &fds, nullptr, nullptr, nullptr) == -1)
          THROWERRNO("select");

        for (int i = 0; i < 2; i++)
        {
          if (socks.children[i] != -1 && FD_ISSET(socks.children[i], &fds))
          {  // there is data to be left from left child
            if (child_read_pos[i] == n)
              THROW("I think child has no data to send but he thinks he has "
                  << FD_ISSET(socks.children[0], &fds) << " " << FD_ISSET(socks.children[1], &fds));

            size_t count = std::min(ar_buf_size, n - child_read_pos[i]);
            int read_size = recv(socks.children[i], &child_read_buf[i][child_unprocessed[i]], (int)count, 0);
            if (read_size == -1)
              THROWERRNO("recv from child");

            addbufs<T, f>((T*)buffer + child_read_pos[i] / sizeof(T), (T*)child_read_buf[i],
                (child_read_pos[i] + read_size) / sizeof(T) - child_read_pos[i] / sizeof(T));

            child_read_pos[i] += read_size;
            int old_unprocessed = child_unprocessed[i];
            child_unprocessed[i] = child_read_pos[i] % (int)sizeof(T);
            for (int j = 0; j < child_unprocessed[i]; j++)
            {
              child_read_buf[i][j] =
                  child_read_buf[i][((old_unprocessed + read_size) / (int)sizeof(T)) * sizeof(T) + j];
            }

            if (child_read_pos[i] == n)  // Done reading parent
              FD_CLR(socks.children[i], &fds);
          }
          else if (socks.children[i] != -1 && child_read_pos[i] != n)
            FD_SET(socks.children[i], &fds);
        }
      }
      if (socks.parent == -1 && child_read_pos[0] == n && child_read_pos[1] == n)
        parent_sent_pos = n;
    }
  }

  void pass_down(char* buffer, const size_t parent_read_pos, size_t& children_sent_pos);
  void broadcast(char* buffer, const size_t n);

  socket_t sock_connect(const uint32_t ip, const int port);
  socket_t getsock();

 public:
  AllReduceSockets(std::string pspan_server, const int pport, const size_t punique_id, size_t ptotal,
      const size_t pnode, bool pquiet)
      : AllReduce(ptotal, pnode, pquiet), span_server(pspan_server), port(pport), unique_id(punique_id)
  {
  }

  virtual ~AllReduceSockets() = default;

  template <class T, void (*f)(T&, const T&)>
  void all_reduce(T* buffer, const size_t n)
  {
    if (span_server != socks.current_master)
      all_reduce_init();
    reduce<T, f>((char*)buffer, n * sizeof(T));
    broadcast((char*)buffer, n * sizeof(T));
  }
};
