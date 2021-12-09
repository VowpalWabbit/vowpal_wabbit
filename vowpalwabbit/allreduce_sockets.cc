// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

/*
This implements the allreduce function of MPI.  Code primarily by
Alekh Agarwal and John Langford, with help Olivier Chapelle.
 */
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#  define NOMINMAX
#  define _WINSOCK_DEPRECATED_NO_WARNINGS
#  include <WinSock2.h>
#  include <Windows.h>
#  include <WS2tcpip.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <arpa/inet.h>
#endif
#include <sys/timeb.h>
#include "allreduce.h"
#include "vw_exception.h"

#include "io/logger.h"

// port is already in network order
socket_t AllReduceSockets::sock_connect(const uint32_t ip, const int port, VW::io::logger& logger)
{
  socket_t sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) THROWERRNO("socket");

  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = static_cast<u_short>(port);

  far_end.sin_addr = *(in_addr*)&ip;
  memset(&far_end.sin_zero, '\0', 8);

  {
    char dotted_quad[INET_ADDRSTRLEN];
    if (nullptr == inet_ntop(AF_INET, &(far_end.sin_addr), dotted_quad, INET_ADDRSTRLEN)) THROWERRNO("inet_ntop");

    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    if (getnameinfo(reinterpret_cast<sockaddr*>(&far_end), sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV,
            NI_NUMERICSERV))
      THROWERRNO("getnameinfo(" << dotted_quad << ")");

    logger.err_info("connecting to {0} = {1}:{2}", dotted_quad, hostname, ntohs(static_cast<u_short>(port)));
  }

  size_t count = 0;
  int ret;
  while ((ret = connect(sock, reinterpret_cast<sockaddr*>(&far_end), sizeof(far_end))) == -1 && count < 100)
  {
    count++;
    logger.err_error("connection attempt {0} failed: {1}", count, VW::strerror_to_string(errno));
#ifdef _WIN32
    Sleep(1);
#else
    sleep(1);
#endif
  }
  if (ret == -1) THROW("cannot connect");
  return sock;
}

socket_t AllReduceSockets::getsock(VW::io::logger& logger)
{
  socket_t sock = socket(PF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
  if (sock == INVALID_SOCKET)
#else
  if (sock < 0)
#endif
    THROWERRNO("socket");

    // SO_REUSEADDR will allow port rebinding on Windows, causing multiple instances
    // of VW on the same machine to potentially contact the wrong tree node.
#ifndef _WIN32
  int on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on)) < 0)
  {
    logger.err_error("setsockopt SO_REUSEADDR: {}", VW::strerror_to_string(errno));
  }
#endif

  // Enable TCP Keep Alive to prevent socket leaks
  int enableTKA = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&enableTKA), sizeof(enableTKA)) < 0)
  {
    logger.err_error("setsockopt SO_KEEPALIVE: {}", VW::strerror_to_string(errno));
  }

  return sock;
}

void AllReduceSockets::all_reduce_init(VW::io::logger& logger)
{
#ifdef _WIN32
  WSAData wsaData;
  int lastError = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (lastError != 0) THROWERRNO("WSAStartup() returned error:" << lastError);
#endif

  struct hostent* master = gethostbyname(span_server.c_str());

  if (master == nullptr) THROWERRNO("gethostbyname(" << span_server << ")");

  socks.current_master = span_server;

  uint32_t master_ip = *(reinterpret_cast<uint32_t*>(master->h_addr));

  socket_t master_sock = sock_connect(master_ip, htons(static_cast<u_short>(port)), logger);
  if (send(master_sock, reinterpret_cast<const char*>(&unique_id), sizeof(unique_id), 0) <
      static_cast<int>(sizeof(unique_id)))
  { THROW("Write unique_id=" << unique_id << " to span server failed"); }
  else
  {
    logger.err_info("wrote unique_id={}", unique_id);
  }
  if (send(master_sock, reinterpret_cast<const char*>(&total), sizeof(total), 0) < static_cast<int>(sizeof(total)))
  { THROW("Write total=" << total << " to span server failed"); }
  else
  {
    logger.err_info("wrote total={}", total);
  }
  if (send(master_sock, (char*)&node, sizeof(node), 0) < static_cast<int>(sizeof(node)))
  { THROW("Write node=" << node << " to span server failed"); }
  else
  {
    logger.err_info("wrote node={}", node);
  }
  int ok;
  if (recv(master_sock, reinterpret_cast<char*>(&ok), sizeof(ok), 0) < static_cast<int>(sizeof(ok)))
  { THROW("Read ok from span server failed"); }
  else
  {
    logger.err_info("Read ok={}", ok);
  }
  if (!ok) THROW("Mapper already connected");

  uint16_t kid_count;
  uint16_t parent_port;
  uint32_t parent_ip;

  if (recv(master_sock, reinterpret_cast<char*>(&kid_count), sizeof(kid_count), 0) <
      static_cast<int>(sizeof(kid_count)))
  { THROW("Read kid_count from span server failed"); }
  else
  {
    logger.err_info("Read kid_count={}", kid_count);
  }

  auto sock = static_cast<socket_t>(-1);
  short unsigned int netport = htons(26544);
  if (kid_count > 0)
  {
    sock = getsock(logger);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = netport;

    bool listening = false;
    while (!listening)
    {
      if (::bind(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
      {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEADDRINUSE)
#else
        if (errno == EADDRINUSE)
#endif
        {
          netport = htons(ntohs(netport) + 1);
          address.sin_port = netport;
        }
        else
          THROWERRNO("bind");
      }
      else
      {
        if (listen(sock, kid_count) < 0)
        {
          logger.err_error("Listen: {}", VW::strerror_to_string(errno));
          CLOSESOCK(sock);
          sock = getsock(logger);
        }
        else
        {
          listening = true;
        }
      }
    }
  }

  if (send(master_sock, reinterpret_cast<const char*>(&netport), sizeof(netport), 0) <
      static_cast<int>(sizeof(netport)))
    THROW("Write netport failed");

  if (recv(master_sock, reinterpret_cast<char*>(&parent_ip), sizeof(parent_ip), 0) <
      static_cast<int>(sizeof(parent_ip)))
  { THROW("Read parent_ip failed"); }
  else
  {
    char dotted_quad[INET_ADDRSTRLEN];
    if (nullptr == inet_ntop(AF_INET, reinterpret_cast<char*>(&parent_ip), dotted_quad, INET_ADDRSTRLEN))
    {
      logger.err_error("Read parent_ip={0}(inet_ntop: {1})", parent_ip, VW::strerror_to_string(errno));
    }
    else
    {
      logger.err_info("Read parent_ip={}", dotted_quad);
    }
  }
  if (recv(master_sock, reinterpret_cast<char*>(&parent_port), sizeof(parent_port), 0) <
      static_cast<int>(sizeof(parent_port)))
  { THROW("Read parent_port failed"); }
  else
  {
    logger.err_info("Read parent_port={}", parent_port);
  }

  CLOSESOCK(master_sock);

  if (parent_ip != static_cast<uint32_t>(-1)) { socks.parent = sock_connect(parent_ip, parent_port, logger); }
  else
    socks.parent = static_cast<socket_t>(-1);

  socks.children[0] = static_cast<socket_t>(-1);
  socks.children[1] = static_cast<socket_t>(-1);
  for (int i = 0; i < kid_count; i++)
  {
    sockaddr_in child_address;
    socklen_t size = sizeof(child_address);
    socket_t f = accept(sock, reinterpret_cast<sockaddr*>(&child_address), &size);
#ifdef _WIN32
    if (f == INVALID_SOCKET)
#else
    if (f < 0)
#endif
      THROWERRNO("accept");

    // char hostname[NI_MAXHOST];
    // char servInfo[NI_MAXSERV];
    // getnameinfo((sockaddr *) &child_address, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV,
    // NI_NUMERICSERV); cerr << "connected to " << hostname << ':' << ntohs(port) << endl;
    socks.children[i] = f;
  }

  if (kid_count > 0) CLOSESOCK(sock);
}

void AllReduceSockets::pass_down(char* buffer, const size_t parent_read_pos, size_t& children_sent_pos)
{
  size_t my_bufsize = std::min(ar_buf_size, (parent_read_pos - children_sent_pos));

  if (my_bufsize > 0)
  {
    // going to pass up this chunk of data to the children
    if (socks.children[0] != -1 &&
        send(socks.children[0], buffer + children_sent_pos, static_cast<int>(my_bufsize), 0) <
            static_cast<int>(my_bufsize))
    { THROW("Write to left child failed"); }
    if (socks.children[1] != -1 &&
        send(socks.children[1], buffer + children_sent_pos, static_cast<int>(my_bufsize), 0) <
            static_cast<int>(my_bufsize))
    { THROW("Write to right child failed"); }

    children_sent_pos += my_bufsize;
  }
}

void AllReduceSockets::broadcast(char* buffer, const size_t n)
{
  size_t parent_read_pos = 0;    // First unread float from parent
  size_t children_sent_pos = 0;  // First unsent float to children
  // parent_sent_pos <= left_read_pos
  // parent_sent_pos <= right_read_pos

  if (socks.parent == -1) { parent_read_pos = n; }
  if (socks.children[0] == -1 && socks.children[1] == -1) children_sent_pos = n;

  while (parent_read_pos < n || children_sent_pos < n)
  {
    pass_down(buffer, parent_read_pos, children_sent_pos);
    if (parent_read_pos >= n && children_sent_pos >= n) break;

    if (socks.parent != -1)
    {
      // there is data to be read from the parent
      if (parent_read_pos == n) THROW("I think parent has no data to send but they think they do.");

      size_t count = std::min(ar_buf_size, n - parent_read_pos);
      int read_size = recv(socks.parent, buffer + parent_read_pos, static_cast<int>(count), 0);
      if (read_size == -1) { THROW("recv from parent: " << VW::strerror_to_string(errno)); }
      parent_read_pos += read_size;
    }
  }
}
