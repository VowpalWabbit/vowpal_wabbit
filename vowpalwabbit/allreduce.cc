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

const int buf_size = 1<<16;

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

  if (connect(sock,(sockaddr*)&far_end, sizeof(far_end)) == -1)
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
    throw exception();
  }
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

  //int parent_sock;
  if(parent_ip != (uint32_t)-1) 
    socks.parent = sock_connect(parent_ip, parent_port);
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
    socks.children[i] = f;
  }

  if (kid_count > 0)
    shutdown(sock, SHUT_RDWR);
}

void addbufs(float* buf1, const float* buf2, const int n) {
  for(int i = 0;i < n;i++) 
//     {
//       uint32_t first = *((uint32_t*)(buf1+i));
//       uint32_t second = *((uint32_t*)(buf2+i));
//       uint32_t xkindaor = first^second;
//       buf1[i] = *(float*)(&xkindaor);
//     }
    buf1[i] += buf2[i];
}


void pass_up(char* buffer, int left_read_pos, int right_read_pos, int& parent_sent_pos, socket_t parent_sock, int n) {
  int my_bufsize = min(buf_size, ((int)(floor(left_read_pos/((float)sizeof(float)))*sizeof(float)) - parent_sent_pos));
  my_bufsize = min(my_bufsize, ((int)(floor(right_read_pos/((float)sizeof(float)))*sizeof(float)) - parent_sent_pos));

  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the parent
    int write_size = send(parent_sock, buffer+parent_sent_pos, my_bufsize, 0);
    if(write_size < my_bufsize) 
      cerr<<"Write to parent failed "<<my_bufsize<<" "<<write_size<<" "<<parent_sent_pos<<" "<<left_read_pos<<" "<<right_read_pos<<endl ;
    parent_sent_pos += my_bufsize;
    //cerr<<"buf size = "<<my_bufsize<<" "<<parent_sent_pos<<endl;
  }

}

void pass_down(char* buffer, const int parent_read_pos, int&children_sent_pos, const socket_t * child_sockets, const int n) {

  int my_bufsize = min(buf_size, (parent_read_pos - children_sent_pos));

  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the children
    if(child_sockets[0] != -1 && send(child_sockets[0], buffer+children_sent_pos, my_bufsize, 0) < my_bufsize) 
      cerr<<"Write to left child failed\n";
    if(child_sockets[1] != -1 && send(child_sockets[1], buffer+children_sent_pos, my_bufsize, 0) < my_bufsize) 
      cerr<<"Write to right child failed\n";

    children_sent_pos += my_bufsize;
  }
}


void reduce(char* buffer, const int n, const socket_t parent_sock, const socket_t* child_sockets) {

  fd_set fds;
  FD_ZERO(&fds);
  if(child_sockets[0] != -1)
    FD_SET(child_sockets[0],&fds);
  if(child_sockets[1] != -1)
    FD_SET(child_sockets[1],&fds);

  socket_t max_fd = max(child_sockets[0],child_sockets[1])+1;
  int child_read_pos[2] = {0,0}; //First unread float from left and right children
  int child_unprocessed[2] = {0,0}; //The number of bytes sent by the child but not yet added to the buffer
  char child_read_buf[2][buf_size+sizeof(float)-1];
  int parent_sent_pos = 0; //First unsent float to parent
  //parent_sent_pos <= left_read_pos
  //parent_sent_pos <= right_read_pos
  
  if(child_sockets[0] == -1) {
    child_read_pos[0] = n;
  }
  if(child_sockets[1] == -1) {
    child_read_pos[1] = n;						 
  }

  while (parent_sent_pos < n || child_read_pos[0] < n || child_read_pos[1] < n)
    {
      if(parent_sock != -1)
	pass_up(buffer, child_read_pos[0], child_read_pos[1], parent_sent_pos, parent_sock, n);

      if(parent_sent_pos >= n && child_read_pos[0] >= n && child_read_pos[1] >= n) break;

      //      cout<<"Before select parent_sent_pos = "<<parent_sent_pos<<" child_read_pos[0] = "<<child_read_pos[0]<<" max fd = "<<max_fd<<endl;
      
      if(child_read_pos[0] < n || child_read_pos[1] < n) {
	if (max_fd > 0 && select((int)max_fd,&fds,NULL, NULL, NULL) == -1)
	  {
	    cerr << "Select failed!" << endl;
	    perror(NULL);
	    throw exception();
	  }
      
	for(int i = 0;i < 2;i++) {
	  if(child_sockets[i] != -1 && FD_ISSET(child_sockets[i],&fds)) {
	    //there is data to be left from left child
	    if(child_read_pos[i] == n) {
	      cerr<<"I think child has no data to send but he thinks he has "<<FD_ISSET(child_sockets[0],&fds)<<" "<<FD_ISSET(child_sockets[1],&fds)<<endl;
	      fflush(stderr);
	      throw exception();
	    }
	  
	
	    //float read_buf[buf_size];
	    size_t count = min(buf_size,n - child_read_pos[i]);
	    int read_size = recv(child_sockets[i], child_read_buf[i] + child_unprocessed[i], (int)count, 0);
	    if(read_size == -1) {
	      cerr <<" Read from child failed\n";
	      perror(NULL);
	      throw exception();
	    }
	    
	    //cout<<"Read "<<read_size<<" bytes\n";
// 	    char pattern[4] = {'A','B','C','D'};
// 	    for(int j = 0; j < (child_read_pos[i] + read_size)/sizeof(float) - child_read_pos[i]/sizeof(float);j++) {
// 	      if((buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j] != pattern[j%4] && buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j] != '\0') || (child_read_buf[i][j] != pattern[j%4]&& child_read_buf[i][j] != '\0')) {
// 		cerr<<"Wrong data "<<pattern[j%4]<<" "<<buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j]<<" "<<child_read_buf[i][j]<<endl;
// 		cerr<<"Reading from positions "<<child_read_pos[i]/sizeof(float)+j<<" "<<j<<" "<<child_unprocessed[i]<<" "<<child_read_pos[i]<<endl;
// 		cerr<<"Reading from child "<<i<<" "<<child_read_pos[0]<<" "<<child_read_pos[1]<<endl;
// 	      }
// 	    }

	    addbufs((float*)buffer + child_read_pos[i]/sizeof(float), (float*)child_read_buf[i], (child_read_pos[i] + read_size)/sizeof(float) - child_read_pos[i]/sizeof(float));
	    
	    child_read_pos[i] += read_size;
	    int old_unprocessed = child_unprocessed[i];
	    child_unprocessed[i] = child_read_pos[i] % (int)sizeof(float);
	    //cout<<"Unprocessed "<<child_unprocessed[i]<<" "<<(old_unprocessed + read_size)%(int)sizeof(float)<<" ";
	    for(int j = 0;j < child_unprocessed[i];j++) {
	      // cout<<(child_read_pos[i]/(int)sizeof(float))*(int)sizeof(float)+j<<" ";
	      child_read_buf[i][j] = child_read_buf[i][((old_unprocessed + read_size)/(int)sizeof(float))*sizeof(float)+j];
	    }
	    //cout<<endl;
	  
	    if(child_read_pos[i] == n) //Done reading parent
	      FD_CLR(child_sockets[i],&fds);
	    //cerr<<"Clearing socket "<<i<<" "<<FD_ISSET(child_sockets[i],&fds)<<endl;
	    //cerr<<"Child_unprocessed should be 0"<<child_unprocessed[i]<<endl;
	    //fflush(stderr);
	  }
	  else if(child_sockets[i] != -1 && child_read_pos[i] != n)
	    FD_SET(child_sockets[i],&fds);      
	}
      }
      if(parent_sock == -1 && child_read_pos[0] == n && child_read_pos[1] == n) 
	parent_sent_pos = n;

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
	size_t count = min(buf_size,n-parent_read_pos);
	int read_size = recv(parent_sock, buffer + parent_read_pos, (int)count, 0);
	if(read_size == -1) {
	  cerr <<" Read from parent failed\n";
	  perror(NULL);
	}
	parent_read_pos += read_size;	
      }
    }
}

void all_reduce(float* buffer, const int n, const string master_location, const size_t unique_id, const size_t total, const size_t node, node_socks& socks) 
{
  if(master_location != socks.current_master) 
    all_reduce_init(master_location, unique_id, total, node, socks);
  reduce((char*)buffer, n*sizeof(float), socks.parent, socks.children);
  broadcast((char*)buffer, n*sizeof(float), socks.parent, socks.children);
}

