#include <pthread.h>
#include <stdio.h>
#include <float.h>
#include <iostream>
#include "global_data.h"
#include "config.h"

using namespace std;

global_data global;
string version = PACKAGE_VERSION;

pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t output_done = PTHREAD_COND_INITIALIZER;

struct global_prediction {
  float p;
  float weight;
};

int really_read(int sock, void* in, size_t count)
{
  char* buf = (char*)in;
  size_t done = 0;
  int r = 0;
  while (done < count)
    {
      if ((r = read(sock,buf,count-done)) == 0)
	return 0;
      else
	if (r < 0)
	  {
	    cerr << "argh! bad read! on message from " << sock << endl;
	    perror(NULL);
	    exit(0);
	  }
	else
	  {
	    done += r;
	    buf += r;
	  }
    }
  return done;
}

void get_prediction(int sock, float& res, float& weight)
{
  global_prediction p;
  int count = really_read(sock, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
  
  assert(count == sizeof(p));
}

void send_prediction(int sock, global_prediction p)
{
  if (write(sock, &p, sizeof(p)) < (int)sizeof(p))
    {
      cerr << "argh! bad global write! " << sock << endl;
      perror(NULL);
      exit(0);
    }
}

void binary_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      global_prediction ps = {res, weight};
      send_prediction(f, ps);
    }
}

void print_result(int f, float res, float weight, v_array<char> tag)
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
      if(global.active && weight >= 0)
	{
	  num = sprintf(temp, " %f", weight);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}

void print_lda_result(int f, float* res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      char temp[30];
      ssize_t t;
      int num;
      for (size_t k = 0; k < global.lda; k++)
	{
	  num = sprintf(temp, "%f ", res[k]);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      if (tag.begin != tag.end){
	temp[0] = ' ';
	t = write(f, temp, 1);
	if (t != 1)
	  cerr << "write error" << endl;
	t = write(f, tag.begin, sizeof(char)*tag.index());
	if (t != (ssize_t) (sizeof(char)*tag.index()))
	  cerr << "write error" << endl;
      }
      if(global.active && weight >= 0)
	{
	  num = sprintf(temp, " %f", weight);
	  t = write(f, temp, num);
	  if (t != num)
	    cerr << "write error" << endl;
	}
      temp[0] = '\n';
      t = write(f, temp, 1);
      if (t != 1)
	cerr << "write error" << endl;
    }
}

void set_mm(double label)
{
  global.sd->min_label = min(global.sd->min_label, label);
  if (label != FLT_MAX)
    global.sd->max_label = max(global.sd->max_label, label);
}

void noop_mm(double label)
{}

void (*set_minmax)(double label) = set_mm;

