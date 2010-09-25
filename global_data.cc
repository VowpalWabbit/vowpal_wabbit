#include <pthread.h>
#include <stdio.h>
#include "global_data.h"
#include "multisource.h"
#include "message_relay.h"

global_data global;
string version = "4.2";

void binary_print_result(int f, float res, float weight, v_array<char> tag)
{
  if (f >= 0)
    {
      global_prediction ps = {res, weight};
      send_global_prediction(f, ps);
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
      temp[0] = '\n';
      t = write(f, temp, 1);     
      if (t != 1) 
	cerr << "write error" << endl;
    }
}

void set_mm(double label)
{
  global.min_label = min(global.min_label, label);
  global.max_label = max(global.max_label, label);
}

void noop_mm(double label)
{}

void (*set_minmax)(double label) = set_mm;

