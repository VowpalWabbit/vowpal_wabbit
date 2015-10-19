#pragma once

#ifdef _WIN32

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <io.h>

#define CLOSESOCK closesocket
#define inet_ntop InetNtopA

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <strings.h>
#include <arpa/inet.h>

#define CLOSESOCK close

typedef int socket_t;

#endif

#ifdef __APPLE__
#include <future>
#else
namespace std
{
  template<typename T>
  class future;
}
#endif

namespace VW
{
  class SpanningTree
  {
  private:
    bool m_stop;
    socket_t sock;
    short unsigned int port;

    // future to signal end of thread running.
    // Need a pointer since C++/CLI doesn't like futures yet
    std::future<void>* m_future;

  public:
    SpanningTree();
    ~SpanningTree();

    void Start();
    void Run();
    void Stop();
  };
}
