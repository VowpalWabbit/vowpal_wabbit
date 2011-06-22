#ifndef ALLREDUCE_H
#define ALLREDUCE_H
#include <string>

struct node_socks {
  int parent;
  int children[2];
};
  
using namespace std;

const int buf_size = 1<<18;

void all_reduce_init(string master_location, node_socks* socks);
void all_reduce(char* buffer, int n, node_socks socks);
void all_reduce_close(node_socks socks);

#endif
