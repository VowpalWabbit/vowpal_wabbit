// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

using std::cerr;
using std::endl;

int open_socket(const char* host, unsigned short port)
{
  hostent* he;
  he = gethostbyname(host);

  if (he == nullptr)
  {
    std::stringstream msg;
    msg << "gethostbyname(" << host << "): " << strerror(errno);
    cerr << msg.str() << endl;
    throw std::runtime_error(msg.str().c_str());
  }
  int sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd == -1)
  {
    std::stringstream msg;
    msg << "socket: " << strerror(errno);
    cerr << msg.str() << endl;
    throw std::runtime_error(msg.str().c_str());
  }
  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = htons(port);
  far_end.sin_addr = *(in_addr*)(he->h_addr);
  memset(&far_end.sin_zero, '\0', 8);
  if (connect(sd, (sockaddr*)&far_end, sizeof(far_end)) == -1)
  {
    std::stringstream msg;
    msg << "connect(" << host << ':' << port << "): " << strerror(errno);
    cerr << msg.str() << endl;
    throw std::runtime_error(msg.str().c_str());
  }
  return sd;
}

int recvall(int s, char* buf, int n)
{
  int total = 0;
  int ret = recv(s, buf, n, 0);
  while (ret > 0 && total < n)
  {
    total += ret;
    if (buf[total - 1] == '\n')
      break;
    ret = recv(s, buf + total, n, 0);
  }
  return total;
}

int main(int argc, char* argv[])
{
  char buf[256];
  char *toks, *itok, *ttag;
  std::string tag;
  const char* host = "localhost";
  unsigned short port = ~0;
  ssize_t pos;
  int s, ret, queries = 0;
  std::string line;

  if (argc > 1)
  {
    host = argv[1];
  }
  if (argc > 2)
  {
    port = atoi(argv[2]);
  }
  if (port <= 1024 || port == (unsigned short)(~0u))
  {
    port = 26542;
  }

  s = open_socket(host, port);
  size_t id = 0;
  ret = send(s, &id, sizeof(id), 0);
  if (ret < 0)
  {
    const char* msg = "Could not perform handshake!";
    cerr << msg << endl;
    throw std::runtime_error(msg);
  }

  while (getline(std::cin, line))
  {
    line.append("\n");
    int len = line.size();
    const char* cstr = line.c_str();
    const char* sp = strchr(cstr, ' ');
    ret = send(s, sp + 1, len - (sp + 1 - cstr), 0);
    if (ret < 0)
    {
      const char* msg = "Could not send unlabeled data!";
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }
    ret = recvall(s, buf, 256);
    if (ret < 0)
    {
      const char* msg = "Could not receive queries!";
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }
    buf[ret] = '\0';
    toks = &buf[0];
    strsep(&toks, " ");
    ttag = strsep(&toks, " ");
    tag = ttag ? std::string(ttag) : std::string("'empty");
    itok = strsep(&toks, "\n");
    if (itok == nullptr || itok[0] == '\0')
    {
      continue;
    }

    queries += 1;
    std::string imp = std::string(itok) + " " + tag + " |";
    pos = line.find_first_of('|');
    line.replace(pos, 1, imp);
    cstr = line.c_str();
    len = line.size();
    ret = send(s, cstr, len, 0);
    if (ret < 0)
    {
      const char* msg = "Could not send labeled data!";
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }
    ret = recvall(s, buf, 256);
    if (ret < 0)
    {
      const char* msg = "Could not receive predictions!";
      cerr << msg << endl;
      throw std::runtime_error(msg);
    }
  }
  close(s);
  std::cout << "Went through the data by doing " << queries << " queries" << endl;
  return 0;
}
