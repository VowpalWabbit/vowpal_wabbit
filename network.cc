#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

using namespace std;

int open_socket(const char* host, size_t new_id)
{
  const char* colon = index(host,':');
  short unsigned int port = 26542;
  hostent* he;
  if (colon != NULL)
    {
      port = atoi(colon+1);
      char hostname[colon-host+1];
      strncpy(hostname, host, colon-host);
      hostname[colon-host]='\0';
      he = gethostbyname(hostname);
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
  if (write(sd, &new_id, sizeof(new_id)) < (int)sizeof(new_id))
    cerr << "write failed!" << endl;
  return sd;
}
