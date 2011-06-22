#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>
#include <strings.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <cmath>

using namespace std;

struct client {
  uint32_t client_ip;
  int socket;
};

static int socket_sort(const void* s1, const void* s2) {
  
  client* socket1 = (client*)s1;
  client* socket2 = (client*)s2;
  return socket1->client_ip - socket2->client_ip;
}

int build_tree(int*  parent, int* kid_count, int source_count, int offset) {

  if(source_count == 1) {
    kid_count[offset] = 0;
    return offset;
  }
    
  int height = (int)floor(log((double)source_count)/log(2.0));
  int root = (1 << height) - 1;
  int left_count = root;
  int left_offset = offset;
  int left_child = build_tree(parent, kid_count, left_count, left_offset);
  int oroot = root+offset;
  parent[left_child] = oroot;
  
  int right_count = source_count - left_count - 1;
  if (right_count > 0)
    {
      int right_offset = oroot+1;
      
      int right_child = build_tree(parent, kid_count, right_count, right_offset);
      parent[right_child] = oroot;
      kid_count[oroot] = 2;
    }
  else
      kid_count[oroot] = 1;

  return oroot;
}

int main(int argc, char* argv[]) {
  int source_count = atoi(argv[1]);

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    cerr << "can't open socket!" << endl;
    exit(1);
  }
  else cerr<<sock<<endl;

  int on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
    perror("setsockopt SO_REUSEADDR");

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  short unsigned int port = 39524;
      
  address.sin_port = htons(port);
  if (bind(sock,(sockaddr*)&address, sizeof(address)) < 0)
    {
      cerr << "failure to bind!" << endl;
      exit(1);
    }
  if (daemon(1,1))
    {
      cerr << "failure to background!" << endl;
      exit(1);
    }
      
  listen(sock, source_count);

  sockaddr_in client_address;
  client client_sockets[source_count];
  socklen_t size = sizeof(client_address);
  
  //sockaddr_in* slave_addrs = new sockaddr_in[source_count];
  cout<<"Source count is "<<source_count<<endl;

  for (int i = 0; i < source_count; i++)
    {
      //cerr << "calling accept" << endl;
      int f = accept(sock,(sockaddr*)&client_address,&size);
      if (f < 0)
	{
	  cerr << "bad client socket!" << endl;
	  exit (1);
	}
      client_sockets[i].client_ip = client_address.sin_addr.s_addr;
      client_sockets[i].socket = f;
    }
  //for (int i = 0; i < source_count; i++)
  //cout << "sock = " << client_sockets[i].socket << endl;
  qsort(client_sockets, source_count, sizeof(client), socket_sort);
  //for (int i = 0; i < source_count; i++)
  //cout << "sock2 = " << client_sockets[i].socket << endl;
  int client_ports[source_count];
  client_ports[0] = htons(port+1);
  for(int i = 1;i < source_count;i++) {
    if(client_sockets[i].client_ip == client_sockets[i-1].client_ip) 
      client_ports[i] = htons(ntohs(client_ports[i-1])+1);
    else
      client_ports[i] = htons(port+1);
  }

  int parent[source_count];
  int kid_count[source_count];

  int root = build_tree(parent, kid_count, source_count, 0);
  cout<<"Root is "<<root<<endl;
  parent[root] = -1;


  for (int i = 0; i < source_count; i++)
    {
      //cout << "i = " << i << " parent[i] = " << parent[i] << " my port = "<<ntohs(client_ports[i])<<" parent port = "<<ntohs(client_ports[parent[i]])<<endl;
      //cout<<client_sockets[i].socket<<endl;
      write(client_sockets[i].socket, &client_ports[i], sizeof(client_ports[i]));
      write(client_sockets[i].socket, &kid_count[i], sizeof(kid_count[i]));
      if (parent[i] >= 0)
	{
	  write(client_sockets[i].socket, &client_sockets[parent[i]].client_ip, sizeof(client_sockets[parent[i]].client_ip));
	  write(client_sockets[i].socket, &client_ports[parent[i]], sizeof(client_ports[parent[i]]));
	}
      else
	{
	  int bogus = -1;
	  uint32_t bogus2 = -1;
	  write(client_sockets[i].socket, &bogus2, sizeof(bogus2));
	  write(client_sockets[i].socket, &bogus, sizeof(bogus));
	}
    }

  for(int i = 0;i < source_count;i++) {
    int done;
    if(read(client_sockets[i].socket, &done, sizeof(done)) < (int) sizeof(done)) 
      cerr<<" Read failed for node "<<i<<endl;
  }
  cout<<"Finished reading at master\n";
  for(int i = 0;i < source_count;i++) {
    int done = 1;
    if(write(client_sockets[i].socket, &done, sizeof(done)) < (int) sizeof(done)) 
      cerr<<" Write failed\n";
    close(client_sockets[i].socket);
  }
}
