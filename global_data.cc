#include <pthread.h>
#include "global_data.h"
#include "multisource.h"
#include "message_relay.h"

global_data global;
pthread_mutex_t io = PTHREAD_MUTEX_INITIALIZER;

void binary_print_result(int f, float res, v_array<char> tag)
{
  if (f >= 0)
    {
      prediction ps = {res, -1};
      send_prediction(f, ps);
    }
}

void print_result(int f, float res, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      int num = sprintf(temp, "%f", res);
      ssize_t t;
      t = write(f, temp, num);
      if (t != num) 
	cerr << "write error" << endl;
      if (tag.begin != tag.end){
        temp[0] = ' ';
        t = write(f, temp, 1);
	if (t != 1)
	  cerr << "write error" << endl;
        t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  cerr << "write error" << endl;
      }
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}



