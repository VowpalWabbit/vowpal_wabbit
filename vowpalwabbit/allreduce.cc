/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
/*
This implements the allreduce function of MPI.  Code primarily by
Alekh Agarwal and John Langford, with help Olivier Chapelle.
 */
#include <iostream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/timeb.h>
#include "allreduce.h"

using namespace std;

// port is already in network order
socket_t sock_connect(const uint32_t ip, const int port) {

  socket_t sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    {
      cerr << "can't get socket " << endl;
      throw exception();
    }
  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = port;
  
  far_end.sin_addr = *(in_addr*)&ip;
  memset(&far_end.sin_zero, '\0',8);
  
  {
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    getnameinfo((sockaddr *) &far_end, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);
    
    cerr << "connecting to " << hostname << ':' << ntohs(port) << endl;
  }
  
  size_t count = 0;
  int ret;
  while ( (ret =connect(sock,(sockaddr*)&far_end, sizeof(far_end))) == -1 && count < 100)
    {
#ifdef _WIN32
      int err_code = WSAGetLastError();
      cerr << "Windows Sockets error code: " << err_code << endl;
#endif
      cerr << "can't connect to: " ;
      uint32_t pip = ntohl(ip);
      unsigned char * pp = (unsigned char*)&pip;
      
      for (size_t i = 0; i < 4; i++)
	{
	  cerr << static_cast<unsigned int>(static_cast<unsigned short>(pp[3-i])) << ".";
	}
      cerr << ':' << ntohs(port) << endl;
      perror(NULL);
      count++;
      sleep(1);
    }
  if (ret == -1)
    throw exception();
  return sock;
}

socket_t getsock()
{
  socket_t sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
      cerr << "can't open socket!" << endl;
      throw exception();
  }

  // SO_REUSEADDR will allow port rebinding on Windows, causing multiple instances
  // of VW on the same machine to potentially contact the wrong tree node.
#ifndef _WIN32
    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
      perror("setsockopt SO_REUSEADDR");
#endif
  return sock;
}

void all_reduce_init(const string master_location, const size_t unique_id, const size_t total, const size_t node, node_socks& socks)
{
#ifdef _WIN32
  WSAData wsaData;
  WSAStartup(MAKEWORD(2,2), &wsaData);
  int lastError = WSAGetLastError();
#endif

  

  struct hostent* master = gethostbyname(master_location.c_str());

  if (master == NULL) {
    cerr << "can't resolve hostname: " << master_location << endl;
    throw exception();
  }
  socks.current_master = master_location;

  uint32_t master_ip = * ((uint32_t*)master->h_addr);
  int port = 26543;

  socket_t master_sock = sock_connect(master_ip, htons(port));
  if(send(master_sock, (const char*)&unique_id, sizeof(unique_id), 0) < (int)sizeof(unique_id))
    cerr << "write failed!" << endl; 
  if(send(master_sock, (const char*)&total, sizeof(total), 0) < (int)sizeof(total))
    cerr << "write failed!" << endl; 
  if(send(master_sock, (char*)&node, sizeof(node), 0) < (int)sizeof(node))
    cerr << "write failed!" << endl; 
  int ok;
  if (recv(master_sock, (char*)&ok, sizeof(ok), 0) < (int)sizeof(ok))
    cerr << "read 1 failed!" << endl;
  if (!ok) {
    cerr << "mapper already connected" << endl;
    throw exception();
  }

  uint16_t kid_count;
  uint16_t parent_port;
  uint32_t parent_ip;

  if(recv(master_sock, (char*)&kid_count, sizeof(kid_count), 0) < (int)sizeof(kid_count))
    cerr << "read 2 failed!" << endl;

  socket_t sock = -1;
  short unsigned int netport = htons(26544);
  if(kid_count > 0) {
    sock = getsock();
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = netport;

    bool listening = false;
    while(!listening)
    {
      if (bind(sock,(sockaddr*)&address, sizeof(address)) < 0)
      {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEADDRINUSE)
#else
        if (errno == EADDRINUSE)
#endif
        {
          netport = htons(ntohs(netport)+1);
          address.sin_port = netport;
        }
        else
        {
          perror("Bind failed ");
          throw exception();
        }
      }
      else
      {
        if (listen(sock, kid_count) < 0)
        {
          perror("listen failed! ");
          shutdown(sock, SHUT_RDWR);
          sock = getsock();
        }
        else
        {
          listening = true;
        }
      }
    }
  }

  if(send(master_sock, (const char*)&netport, sizeof(netport), 0) < (int)sizeof(netport))
    cerr << "write failed!" << endl;

  if(recv(master_sock, (char*)&parent_ip, sizeof(parent_ip), 0) < (int)sizeof(parent_ip))
    cerr << "read 3 failed!" << endl;
  if(recv(master_sock, (char*)&parent_port, sizeof(parent_port), 0) < (int)sizeof(parent_port))
    cerr << "read 4 failed!" << endl;

  shutdown(master_sock, SHUT_RDWR);

  if(parent_ip != (uint32_t)-1) {
    socks.parent = sock_connect(parent_ip, parent_port);
  }
  else
    socks.parent = -1;

  socks.children[0] = -1; socks.children[1] = -1;
  for (int i = 0; i < kid_count; i++)
  {
    sockaddr_in child_address;
    socklen_t size = sizeof(child_address);
    socket_t f = accept(sock,(sockaddr*)&child_address,&size);    
    if (f < 0)
    {
      cerr << "bad client socket!" << endl;
      throw exception();
    }
    // char hostname[NI_MAXHOST];
    // char servInfo[NI_MAXSERV];
    // getnameinfo((sockaddr *) &child_address, sizeof(sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);
    // cerr << "connected to " << hostname << ':' << ntohs(port) << endl;
    socks.children[i] = f;
  }

  if (kid_count > 0)
    shutdown(sock, SHUT_RDWR);
}


void pass_down(char* buffer, const int parent_read_pos, int&children_sent_pos, const socket_t * child_sockets, const int n) {

  int my_bufsize = min(ar_buf_size, (parent_read_pos - children_sent_pos));

  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the children
    if(child_sockets[0] != -1 && send(child_sockets[0], buffer+children_sent_pos, my_bufsize, 0) < my_bufsize) 
      cerr<<"Write to left child failed\n";
    if(child_sockets[1] != -1 && send(child_sockets[1], buffer+children_sent_pos, my_bufsize, 0) < my_bufsize) 
      cerr<<"Write to right child failed\n";

    children_sent_pos += my_bufsize;
  }
}



void broadcast(char* buffer, const int n, const socket_t parent_sock, const socket_t * child_sockets) {
 
   int parent_read_pos = 0; //First unread float from parent
   int children_sent_pos = 0; //First unsent float to children
  //parent_sent_pos <= left_read_pos
  //parent_sent_pos <= right_read_pos
  
   if(parent_sock == -1) {
     parent_read_pos = n;						 
   }
   if(child_sockets[0] == -1 && child_sockets[1] == -1) 
     children_sent_pos = n;

   while (parent_read_pos < n || children_sent_pos < n)
    {
      pass_down(buffer, parent_read_pos, children_sent_pos, child_sockets, n);
      if(parent_read_pos >= n && children_sent_pos >= n) break;

      if (parent_sock != -1) {
	//there is data to be read from the parent
	if(parent_read_pos == n) {
	  cerr<<"I think parent has no data to send but he thinks he has\n";
	  throw exception();
	}
	size_t count = min(ar_buf_size,n-parent_read_pos);
	int read_size = recv(parent_sock, buffer + parent_read_pos, (int)count, 0);
	if(read_size == -1) {
	  cerr <<" Read from parent failed\n";
	  perror(NULL);
	}
	parent_read_pos += read_size;	
      }
    }
}

