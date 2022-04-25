// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif

#  include <WS2tcpip.h>
#  include <WinSock2.h>
#  include <Windows.h>
#  include <io.h>

#  define CLOSESOCK closesocket
#  define inet_ntop InetNtopA

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;

namespace std
{
// forward declare promise as C++/CLI doesn't allow usage in header files
template <typename T>
class future;
}  // namespace std
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <strings.h>
#  include <sys/socket.h>
#  include <unistd.h>

#  define CLOSESOCK close

using socket_t = int;

#  include <future>
#endif

namespace VW
{
class SpanningTree
{
private:
  bool m_stop;
  socket_t sock;
  uint16_t m_port;

  // future to signal end of thread running.
  // Need a pointer since C++/CLI doesn't like futures yet
  std::future<void>* m_future;

  bool m_quiet;

public:
  SpanningTree(short unsigned int port = 26543, bool quiet = false);
  ~SpanningTree();

  short unsigned int BoundPort();

  void Start();
  void Run();
  void Stop();
};
}  // namespace VW
