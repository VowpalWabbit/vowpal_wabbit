#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>

#include <string>
#include <iostream>

using namespace std;

int open_socket(const char* host, size_t new_id)
{
  hostent* he = gethostbyname(host);
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
  far_end.sin_port = htons(39524);
  far_end.sin_addr = *(in_addr*)(he->h_addr);
  memset(&far_end.sin_zero, '\0',8);
  if (connect(sd,(sockaddr*)&far_end, sizeof(far_end)) == -1)
    {
      cerr << "can't connect to: " << host << endl;
      exit(1);
    }
  write(sd, &new_id, sizeof(new_id));
  fsync(sd);
  return sd;
}
