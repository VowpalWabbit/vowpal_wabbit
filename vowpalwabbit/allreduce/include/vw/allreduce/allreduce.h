// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This implements the allreduce function of MPI.
#pragma once

#include <algorithm>
#include <string>
#include <utility>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif

#  include <WS2tcpip.h>
#  include <WinSock2.h>
#  include <Windows.h>
#  include <io.h>
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;
#  define CLOSESOCK closesocket
namespace std
{
// forward declare promise as C++/CLI doesn't allow usage in header files
template <typename T>
class promise;

}  // namespace std
#else
#  include <netdb.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <sys/socket.h>
#  include <unistd.h>

#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
using socket_t = int;
#  define CLOSESOCK close
#  include <future>
#endif

#include "vw/allreduce/allreduce_type.h"
#include "vw/common/future_compat.h"
#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"

#include <cassert>

#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <condition_variable>
#  include <mutex>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <condition_variable>
#  include <mutex>
#endif

namespace VW
{
namespace details
{
constexpr size_t AR_BUF_SIZE = 1 << 16;
class node_socks
{
public:
  std::string current_master;
  socket_t parent;
  socket_t children[2];
  ~node_socks()
  {
    if (current_master != "")
    {
      if (parent != -1) { CLOSESOCK(this->parent); }
      if (children[0] != -1) { CLOSESOCK(this->children[0]); }
      if (children[1] != -1) { CLOSESOCK(this->children[1]); }
    }
  }
  node_socks() { current_master = ""; }
};

template <class T, void (*f)(T&, const T&)>
void addbufs(T* buf1, const T* buf2, const size_t n)
{
  for (size_t i = 0; i < n; i++) { f(buf1[i], buf2[i]); }
}

}  // namespace details

class all_reduce_base
{
public:
  const size_t total;  // total number of nodes
  const size_t node;   // node id number
  bool quiet;

  all_reduce_base(size_t ptotal, const size_t pnode, bool pquiet = false) : total(ptotal), node(pnode), quiet(pquiet)
  {
    assert(node < total);
  }

  virtual ~all_reduce_base() = default;
};

class all_reduce_sync
{
public:
  all_reduce_sync(size_t total);
  ~all_reduce_sync();

  void wait_for_synchronization();

  VW_DEPRECATED("Renamed to wait_for_synchronization")
  void waitForSynchronization()  // NOLINT
  {
    wait_for_synchronization();
  }

  void** buffers;

private:
  std::mutex _mutex;
  std::condition_variable _cv;

  // total number of threads we wait for
  size_t _total;

  // number of threads reached the barrier
  uint32_t _count;

  // current wait-barrier-run required to protect against spurious wakeups of _cv->wait(...)
  bool _run;
};

class all_reduce_threads : public all_reduce_base
{
public:
  all_reduce_threads(all_reduce_threads* root, size_t ptotal, size_t pnode, bool quiet = false);

  all_reduce_threads(size_t ptotal, size_t pnode, bool quiet = false);

  virtual ~all_reduce_threads();

  template <class T, void (*f)(T&, const T&)>
  void all_reduce(T* buffer, const size_t n)
  {  // register buffer
    T** buffers = (T**)_sync->buffers;
    buffers[node] = buffer;
    _sync->wait_for_synchronization();

    size_t block_size = n / total;
    size_t index;
    size_t end;

    if (block_size == 0)
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
      index = node * block_size;
      end = node == total - 1 ? n : (node + 1) * block_size;
    }

    for (; index < end; index++)
    {  // Perform transposed AllReduce to help data locallity
      T& first = buffers[0][index];

      for (size_t i = 1; i < total; i++) { f(first, buffers[i][index]); }

      // Broadcast back
      for (size_t i = 1; i < total; i++) { buffers[i][index] = first; }
    }

    _sync->wait_for_synchronization();
  }

private:
  all_reduce_sync* _sync;
  bool _sync_owner;
};

class all_reduce_sockets : public all_reduce_base
{
public:
  all_reduce_sockets(std::string pspan_server, const int pport, const size_t punique_id, size_t ptotal,
      const size_t pnode, bool pquiet)
      : all_reduce_base(ptotal, pnode, pquiet)
      , _span_server(std::move(pspan_server))
      , _port(pport)
      , _unique_id(punique_id)
  {
  }

  ~all_reduce_sockets() override = default;

  template <class T, void (*f)(T&, const T&)>
  void all_reduce(T* buffer, const size_t n, VW::io::logger& logger)
  {
    if (_span_server != _socks.current_master) { all_reduce_init(logger); }
    reduce<T, f>((char*)buffer, n * sizeof(T));
    broadcast((char*)buffer, n * sizeof(T));
  }

private:
  details::node_socks _socks;
  std::string _span_server;
  int _port;
  size_t _unique_id;  // unique id for each node in the network, id == 0 means extra io.

  void all_reduce_init(VW::io::logger& logger);

  template <class T>
  void pass_up(char* buffer, size_t left_read_pos, size_t right_read_pos, size_t& parent_sent_pos)
  {
    size_t my_bufsize = std::min(
        details::AR_BUF_SIZE, std::min(left_read_pos, right_read_pos) / sizeof(T) * sizeof(T) - parent_sent_pos);

    if (my_bufsize > 0)
    {  // going to pass up this chunk of data to the parent
      int write_size = send(_socks.parent, buffer + parent_sent_pos, static_cast<int>(my_bufsize), 0);
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
    if (_socks.children[0] != -1) { FD_SET(_socks.children[0], &fds); }
    if (_socks.children[1] != -1) { FD_SET(_socks.children[1], &fds); }

    socket_t max_fd = std::max(_socks.children[0], _socks.children[1]) + 1;
    size_t child_read_pos[2] = {0, 0};  // First unread float from left and right children
    int child_unprocessed[2] = {0, 0};  // The number of bytes sent by the child but not yet added to the buffer
    char child_read_buf[2][details::AR_BUF_SIZE + sizeof(T) - 1];
    size_t parent_sent_pos = 0;  // First unsent float to parent
    // parent_sent_pos <= left_read_pos
    // parent_sent_pos <= right_read_pos

    if (_socks.children[0] == -1) { child_read_pos[0] = n; }
    if (_socks.children[1] == -1) { child_read_pos[1] = n; }

    while (parent_sent_pos < n || child_read_pos[0] < n || child_read_pos[1] < n)
    {
      if (_socks.parent != -1) { pass_up<T>(buffer, child_read_pos[0], child_read_pos[1], parent_sent_pos); }

      if (parent_sent_pos >= n && child_read_pos[0] >= n && child_read_pos[1] >= n) { break; }

      if (child_read_pos[0] < n || child_read_pos[1] < n)
      {
        if (max_fd > 0 && select(static_cast<int>(max_fd), &fds, nullptr, nullptr, nullptr) == -1) THROWERRNO("select");

        for (int i = 0; i < 2; i++)
        {
          if (_socks.children[i] != -1 && FD_ISSET(_socks.children[i], &fds))
          {  // there is data to be left from left child
            if (child_read_pos[i] == n)
              THROW("I think child has no data to send but he thinks he has "
                  << FD_ISSET(_socks.children[0], &fds) << " " << FD_ISSET(_socks.children[1], &fds));

            size_t count = std::min(details::AR_BUF_SIZE, n - child_read_pos[i]);
            int read_size =
                recv(_socks.children[i], &child_read_buf[i][child_unprocessed[i]], static_cast<int>(count), 0);
            if (read_size == -1) THROWERRNO("recv from child");

            details::addbufs<T, f>((T*)buffer + child_read_pos[i] / sizeof(T), (T*)child_read_buf[i],
                (child_read_pos[i] + read_size) / sizeof(T) - child_read_pos[i] / sizeof(T));

            child_read_pos[i] += read_size;
            int old_unprocessed = child_unprocessed[i];
            child_unprocessed[i] = child_read_pos[i] % (int)sizeof(T);
            for (int j = 0; j < child_unprocessed[i]; j++)
            {
              child_read_buf[i][j] =
                  child_read_buf[i][((old_unprocessed + read_size) / (int)sizeof(T)) * sizeof(T) + j];
            }

            if (child_read_pos[i] == n)
            {  // Done reading parent
              FD_CLR(_socks.children[i], &fds);
            }
          }
          else if (_socks.children[i] != -1 && child_read_pos[i] != n) { FD_SET(_socks.children[i], &fds); }
        }
      }
      if (_socks.parent == -1 && child_read_pos[0] == n && child_read_pos[1] == n) { parent_sent_pos = n; }
    }
  }

  void pass_down(char* buffer, size_t parent_read_pos, size_t& children_sent_pos);
  void broadcast(char* buffer, size_t n);

  socket_t sock_connect(uint32_t ip, int port, VW::io::logger& logger);
  socket_t getsock(VW::io::logger& logger);
};

}  // namespace VW

using AllReduceType VW_DEPRECATED(
    "AllReduceType was moved into VW namespace. Use VW::all_reduce_type") = VW::all_reduce_type;
using AllReduce VW_DEPRECATED("AllReduce was moved into VW namespace. Use VW::all_reduce_base") = VW::all_reduce_base;
using AllReduceSync VW_DEPRECATED(
    "AllReduceSync was moved into VW namespace. Use VW::all_reduce_sync") = VW::all_reduce_sync;
using AllReduceSockets VW_DEPRECATED(
    "all_reduce_sockets was moved into VW namespace. Use VW::all_reduce_sockets") = VW::all_reduce_sockets;
using AllReduceThreads VW_DEPRECATED(
    "all_reduce_threads was moved into VW namespace. Use VW::all_reduce_threads") = VW::all_reduce_threads;
