// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "vw_exception.h"

int open_socket(const char* host)
{
#ifdef _WIN32
  const char* colon = strchr(host, ':');
#else
  const char* colon = index(host, ':');
#endif
  short unsigned int port = 26542;
  hostent* he;
  if (colon != nullptr)
  {
    port = atoi(colon + 1);
    std::string hostname(host, colon - host);
    he = gethostbyname(hostname.c_str());
  }
  else
    he = gethostbyname(host);

  if (he == nullptr)
    THROWERRNO("gethostbyname(" << host << ")");

  int sd = (int)socket(PF_INET, SOCK_STREAM, 0);
  if (sd == -1)
    THROWERRNO("socket");

  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = htons(port);
  far_end.sin_addr = *(in_addr*)(he->h_addr);
  memset(&far_end.sin_zero, '\0', 8);
  if (connect(sd, (sockaddr*)&far_end, sizeof(far_end)) == -1)
    THROWERRNO("connect(" << host << ':' << port << ")");

  char id = '\0';
  if (
#ifdef _WIN32
      _write(sd, &id, sizeof(id)) < (int)sizeof(id)
#else
      write(sd, &id, sizeof(id)) < (int)sizeof(id)
#endif
  )
    std::cerr << "write failed!" << std::endl;
  return sd;
}
