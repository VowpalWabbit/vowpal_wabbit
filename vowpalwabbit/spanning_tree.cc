// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "spanning_tree.h"
#include "vw_exception.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <future>

struct client
{
  uint32_t client_ip;
  socket_t socket;
};

struct partial
{
  client* nodes;
  size_t filled;
};

static int socket_sort(const void* s1, const void* s2)
{
  client* socket1 = (client*)s1;
  client* socket2 = (client*)s2;
  if (socket1->client_ip != socket2->client_ip)
    return socket1->client_ip - socket2->client_ip;
  else
    return (int)(socket1->socket - socket2->socket);
}

int build_tree(int* parent, uint16_t* kid_count, size_t source_count, int offset)
{
  if (source_count == 1)
  {
    kid_count[offset] = 0;
    return offset;
  }

  int height = (int)floor(log((double)source_count) / log(2.0));
  int root = (1 << height) - 1;
  int left_count = root;
  int left_offset = offset;
  int left_child = build_tree(parent, kid_count, left_count, left_offset);
  int oroot = root + offset;
  parent[left_child] = oroot;

  size_t right_count = source_count - left_count - 1;
  if (right_count > 0)
  {
    int right_offset = oroot + 1;

    int right_child = build_tree(parent, kid_count, right_count, right_offset);
    parent[right_child] = oroot;
    kid_count[oroot] = 2;
  }
  else
    kid_count[oroot] = 1;

  return oroot;
}

void fail_send(const socket_t fd, const void* buf, const int count)
{
  if (send(fd, (char*)buf, count, 0) == -1)
    THROWERRNO("send: ");
}

namespace VW
{
SpanningTree::SpanningTree(uint16_t port, bool quiet) : m_stop(false), m_port(port), m_future(nullptr), m_quiet(quiet)
{
#ifdef _WIN32
  WSAData wsaData;
  int lastError = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (lastError != 0)
    THROWERRNO("WSAStartup() returned error:" << lastError);
#endif

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    THROWERRNO("socket: ");

  int on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
    THROWERRNO("setsockopt SO_REUSEADDR: ");

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  address.sin_port = htons(port);
  if (::bind(sock, (sockaddr*)&address, sizeof(address)) < 0)
    THROWERRNO("bind failed for " << inet_ntoa(address.sin_addr));

  sockaddr_in bound_addr;
  memset(&bound_addr, 0, sizeof(bound_addr));
  socklen_t len = sizeof(bound_addr);
  if (::getsockname(sock, (sockaddr*)&bound_addr, &len) < 0)
    THROWERRNO("getsockname: " << inet_ntoa(bound_addr.sin_addr));

  // which port did we bind too (if m_port is 0 this will give us the actual port)
  m_port = ntohs(bound_addr.sin_port);
}

SpanningTree::~SpanningTree()
{
  Stop();
  delete m_future;
}

short unsigned int SpanningTree::BoundPort() { return m_port; }

void SpanningTree::Start()
{
  // launch async
  if (m_future == nullptr)
  {
    m_future = new std::future<void>;
  }

  *m_future = std::async(std::launch::async, &SpanningTree::Run, this);
}

void SpanningTree::Stop()
{
  m_stop = true;
#ifndef _WIN32
  // just close won't unblock the accept
  shutdown(sock, SHUT_RD);
#endif
  CLOSESOCK(sock);

  // wait for run to stop
  if (m_future != nullptr)
  {
    m_future->get();
  }
}

void SpanningTree::Run()
{
  std::map<size_t, partial> partial_nodesets;
  while (!m_stop)
  {
    if (listen(sock, 1024) < 0)
      THROWERRNO("listen: ");

    sockaddr_in client_address;
    socklen_t size = sizeof(client_address);
    socket_t f = accept(sock, (sockaddr*)&client_address, &size);
#ifdef _WIN32
    if (f == INVALID_SOCKET)
    {
#else
    if (f < 0)
    {
#endif
      break;
    }

    char dotted_quad[INET_ADDRSTRLEN];
    if (NULL == inet_ntop(AF_INET, &(client_address.sin_addr), dotted_quad, INET_ADDRSTRLEN))
      THROWERRNO("inet_ntop: ");

    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    if (getnameinfo((sockaddr*)&client_address, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0))
      THROWERRNO("getnameinfo: ");

    if (!m_quiet)
      std::cerr << "inbound connection from " << dotted_quad << "(" << hostname << ':' << ntohs(m_port)
                << ") serv=" << servInfo << std::endl;

    size_t nonce = 0;
    if (recv(f, (char*)&nonce, sizeof(nonce), 0) != sizeof(nonce))
    {
      THROW(dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): nonce read failed, exiting");
    }
    else
    {
      if (!m_quiet)
        std::cerr << dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): nonce=" << nonce << std::endl;
    }
    size_t total = 0;
    if (recv(f, (char*)&total, sizeof(total), 0) != sizeof(total))
    {
      THROW(dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): total node count read failed, exiting");
    }
    else
    {
      if (!m_quiet)
        std::cerr << dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): total=" << total << std::endl;
    }
    size_t id = 0;
    if (recv(f, (char*)&id, sizeof(id), 0) != sizeof(id))
    {
      THROW(dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): node id read failed, exiting");
    }
    else
    {
      if (!m_quiet)
        std::cerr << dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): node id=" << id << std::endl;
    }

    int ok = true;
    if (id >= total)
    {
      if (!m_quiet)
        std::cout << dotted_quad << "(" << hostname << ':' << ntohs(m_port) << "): invalid id=" << id
                  << " >=  " << total << " !" << std::endl;
      ok = false;
    }
    partial partial_nodeset;

    if (partial_nodesets.find(nonce) == partial_nodesets.end())
    {
      partial_nodeset.nodes = (client*)calloc(total, sizeof(client));
      for (size_t i = 0; i < total; i++) partial_nodeset.nodes[i].client_ip = (uint32_t)-1;
      partial_nodeset.filled = 0;
    }
    else
    {
      partial_nodeset = partial_nodesets[nonce];
      partial_nodesets.erase(nonce);
    }

    if (ok && partial_nodeset.nodes[id].client_ip != (uint32_t)-1)
      ok = false;
    fail_send(f, &ok, sizeof(ok));

    if (ok)
    {
      partial_nodeset.nodes[id].client_ip = client_address.sin_addr.s_addr;
      partial_nodeset.nodes[id].socket = f;
      partial_nodeset.filled++;
    }
    if (partial_nodeset.filled != total)  // Need to wait for more connections
    {
      partial_nodesets[nonce] = partial_nodeset;
      for (size_t i = 0; i < total; i++)
      {
        if (partial_nodeset.nodes[i].client_ip == (uint32_t)-1)
        {
          if (!m_quiet)
            std::cout << "nonce " << nonce << " still waiting for " << (total - partial_nodeset.filled)
                      << " nodes out of " << total << " for example node " << i << std::endl;
          break;
        }
      }
    }
    else
    {
      // Time to make the spanning tree
      qsort(partial_nodeset.nodes, total, sizeof(client), socket_sort);

      int* parent = (int*)calloc(total, sizeof(int));
      uint16_t* kid_count = (uint16_t*)calloc(total, sizeof(uint16_t));

      int root = build_tree(parent, kid_count, total, 0);
      parent[root] = -1;

      for (size_t i = 0; i < total; i++)
      {
        fail_send(partial_nodeset.nodes[i].socket, &kid_count[i], sizeof(kid_count[i]));
      }

      uint16_t* client_ports = (uint16_t*)calloc(total, sizeof(uint16_t));

      for (size_t i = 0; i < total; i++)
      {
        int done = 0;
        if (recv(partial_nodeset.nodes[i].socket, (char*)&(client_ports[i]), sizeof(client_ports[i]), 0) <
            (int)sizeof(client_ports[i]))

          if (!m_quiet)
            std::cerr << " Port read failed for node " << i << " read " << done << std::endl;
      }  // all clients have bound to their ports.

      for (size_t i = 0; i < total; i++)
      {
        if (parent[i] >= 0)
        {
          fail_send(partial_nodeset.nodes[i].socket, &partial_nodeset.nodes[parent[i]].client_ip,
              sizeof(partial_nodeset.nodes[parent[i]].client_ip));
          fail_send(partial_nodeset.nodes[i].socket, &client_ports[parent[i]], sizeof(client_ports[parent[i]]));
        }
        else
        {
          int bogus = -1;
          uint32_t bogus2 = -1;
          fail_send(partial_nodeset.nodes[i].socket, &bogus2, sizeof(bogus2));
          fail_send(partial_nodeset.nodes[i].socket, &bogus, sizeof(bogus));
        }
        CLOSESOCK(partial_nodeset.nodes[i].socket);
      }
      free(client_ports);
      free(partial_nodeset.nodes);
      free(parent);
      free(kid_count);
    }
  }

#ifdef _WIN32
  WSACleanup();
#endif
}
}  // namespace VW
