#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#ifdef _WIN32
#include <WinSock2.h>
#include <io.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#include <errno.h>
#ifndef _WIN32
#include <netdb.h>
#include <strings.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

using namespace std;

int open_socket(const char* host)
{
#ifdef _WIN32
  const char* colon = strchr(host,':');
#else
  const char* colon = index(host,':');
#endif
  short unsigned int port = 26542;
  hostent* he;
  if (colon != NULL)
    {
      port = atoi(colon+1);
      string hostname(host,colon-host);
      he = gethostbyname(hostname.c_str());
    }
  else
    he = gethostbyname(host);

  if (he == NULL)
    {
      cerr << "can't resolve hostname: " << host << endl;
      exit(1);
    }
  int sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd == -1)
    {
      cerr << "can't get socket " << endl;
      exit(1);
    }
  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = htons(port);
  far_end.sin_addr = *(in_addr*)(he->h_addr);
  memset(&far_end.sin_zero, '\0',8);
  if (connect(sd,(sockaddr*)&far_end, sizeof(far_end)) == -1)
    {
      cerr << "can't connect to: " << host << ':' << port << endl;
      exit(1);
    }
  char id = '\0';
  if (write(sd, &id, sizeof(id)) < (int)sizeof(id))
    cerr << "write failed!" << endl;
  return sd;
}
