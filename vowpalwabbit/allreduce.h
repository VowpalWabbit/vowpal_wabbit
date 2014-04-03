/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
// This implements the allreduce function of MPI.  

#ifndef ALLREDUCE_H
#define ALLREDUCE_H
#include <string>
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef int socklen_t;
typedef SOCKET socket_t;
#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
typedef int socket_t;
#endif

using namespace std;

const int ar_buf_size = 1<<16;

struct node_socks {
  std::string current_master;
  socket_t parent;
  socket_t children[2];
  ~node_socks()
  {
    if(current_master != "") {
      if(parent != -1)
	shutdown(this->parent, SHUT_RDWR);
      if(children[0] != -1) 
	shutdown(this->children[0], SHUT_RDWR);
      if(children[1] != -1)
	shutdown(this->children[1], SHUT_RDWR);  
    }
  }
  node_socks ()
  {
    current_master = "";
  }
};


template <class T> void addbufs(T* buf1, const T* buf2, const int n) {
  for(int i = 0;i < n;i++) 
    buf1[i] += buf2[i];
}

void all_reduce_init(const string master_location, const size_t unique_id, const size_t total, const size_t node, node_socks& socks);

template <class T> void pass_up(char* buffer, int left_read_pos, int right_read_pos, int& parent_sent_pos, socket_t parent_sock, int n) {
  int my_bufsize = min(ar_buf_size, ((int)(floor(left_read_pos/((float)sizeof(T)))*sizeof(T)) - parent_sent_pos));
  my_bufsize = min(my_bufsize, ((int)(floor(right_read_pos/((float)sizeof(T)))*sizeof(T)) - parent_sent_pos));
  
  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the parent
    int write_size = send(parent_sock, buffer+parent_sent_pos, my_bufsize, 0);
    if(write_size < my_bufsize) 
      cerr<<"Write to parent failed "<<my_bufsize<<" "<<write_size<<" "<<parent_sent_pos<<" "<<left_read_pos<<" "<<right_read_pos<<endl ;
    parent_sent_pos += my_bufsize;
  }

}

template <class T>void reduce(char* buffer, const int n, const socket_t parent_sock, const socket_t* child_sockets) {

  fd_set fds;
  FD_ZERO(&fds);
  if(child_sockets[0] != -1)
    FD_SET(child_sockets[0],&fds);
  if(child_sockets[1] != -1)
    FD_SET(child_sockets[1],&fds);

  socket_t max_fd = max(child_sockets[0],child_sockets[1])+1;
  int child_read_pos[2] = {0,0}; //First unread float from left and right children
  int child_unprocessed[2] = {0,0}; //The number of bytes sent by the child but not yet added to the buffer
  char child_read_buf[2][ar_buf_size+sizeof(T)-1];
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
	pass_up<T>(buffer, child_read_pos[0], child_read_pos[1], parent_sent_pos, parent_sock, n);

      if(parent_sent_pos >= n && child_read_pos[0] >= n && child_read_pos[1] >= n) break;

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
	      throw exception();
	    }
	  
	
	    size_t count = min(ar_buf_size,n - child_read_pos[i]);
	    int read_size = recv(child_sockets[i], child_read_buf[i] + child_unprocessed[i], (int)count, 0);
	    if(read_size == -1) {
	      cerr <<" Read from child failed\n";
	      perror(NULL);
	      throw exception();
	    }
	    
	    addbufs((T*)buffer + child_read_pos[i]/sizeof(T), (T*)child_read_buf[i], (child_read_pos[i] + read_size)/sizeof(T) - child_read_pos[i]/sizeof(T));
	    
	    child_read_pos[i] += read_size;
	    int old_unprocessed = child_unprocessed[i];
	    child_unprocessed[i] = child_read_pos[i] % (int)sizeof(T);
	    for(int j = 0;j < child_unprocessed[i];j++) {
	      child_read_buf[i][j] = child_read_buf[i][((old_unprocessed + read_size)/(int)sizeof(T))*sizeof(T)+j];
	    }
	  
	    if(child_read_pos[i] == n) //Done reading parent
	      FD_CLR(child_sockets[i],&fds);
	  }
	  else if(child_sockets[i] != -1 && child_read_pos[i] != n)
	    FD_SET(child_sockets[i],&fds);      
	}
      }
      if(parent_sock == -1 && child_read_pos[0] == n && child_read_pos[1] == n) 
	parent_sent_pos = n;

    }  
  
}

void broadcast(char* buffer, const int n, const socket_t parent_sock, const socket_t * child_sockets);


template <class T> void all_reduce(T* buffer, const int n, const std::string master_location, const size_t unique_id, const size_t total, const size_t node, node_socks& socks) 
{
  if(master_location != socks.current_master) 
    all_reduce_init(master_location, unique_id, total, node, socks);
  reduce<T>((char*)buffer, n*sizeof(T), socks.parent, socks.children);
  broadcast((char*)buffer, n*sizeof(T), socks.parent, socks.children);
}





#endif
