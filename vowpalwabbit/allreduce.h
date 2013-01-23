/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
// This implements the allreduce function of MPI.  

#ifndef ALLREDUCE_H
#define ALLREDUCE_H
#include <string>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;
#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
typedef int socket_t;
#endif

struct node_socks {
  std::string current_master;
  socket_t parent;
  socket_t children[2];
  ~node_socks()
  {
    if(current_master != "") {
      if(parent != -1)
	shutdown(this->parent, SHUT_RDWR);
      if(children[0] != -1) 
	shutdown(this->children[0], SHUT_RDWR);
      if(children[1] != -1)
	shutdown(this->children[1], SHUT_RDWR);  
    }
  }
  node_socks ()
  {
    current_master = "";
  }
};

void all_reduce(float* buffer, int n, std::string master_location, size_t unique_id, size_t total, size_t node, node_socks& socks);

#endif
