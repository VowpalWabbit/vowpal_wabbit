#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <float.h>
#include "v_array.h"
#include "example.h"
#include "global_data.h"
#include "parser.h"

v_array<size_t> delay_indicies;//thread specific state.

v_array<example*> delay_ring;//delay_ring state & mutexes
v_array<size_t> threads_to_use;
size_t master_index = 0;
size_t message_index = 0;
pthread_mutex_t delay = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t delay_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t delay_nonempty = PTHREAD_COND_INITIALIZER;

size_t mesg = 0;
size_t compute_threads = 0;

void initialize_delay_ring()
{
  if (global.unique_id == 0 && global.local_prediction > 0)
    mesg = 1;
  compute_threads = global.num_threads();
  size_t nt = compute_threads+mesg;
  reserve(delay_indicies, nt);
  for (size_t i = 0; i < nt; i++)
    delay_indicies[i] = 0;
  reserve(delay_ring,ring_size);
  reserve(threads_to_use, ring_size);
  for (size_t i = 0; i < ring_size; i++)
    delay_ring[i] = NULL;
}

void destroy_delay_ring()
{
  free(delay_indicies.begin);
  free(delay_ring.begin);
  free(threads_to_use.begin);
}

bool thread_done(size_t thread)
{
  bool ret;
  pthread_mutex_lock(&delay);
  ret = (delay_indicies[thread] == master_index);
  pthread_mutex_unlock(&delay);
  return ret;
}

example* get_delay_example(size_t thread)
{//nonblocking
  size_t index = delay_indicies[thread] % ring_size;
  example* ret = delay_ring[index];
  if (ret != NULL)
    {
      delay_indicies[thread]++;
      
      pthread_mutex_lock(&ret->lock);
      if (--threads_to_use[index] == 0) 
	{
	  pthread_mutex_lock(&delay);
	  delay_ring[index] = NULL;
	  pthread_cond_broadcast(&delay_empty);
	  pthread_mutex_unlock(&delay);
	}
      pthread_mutex_unlock(&ret->lock);
    }
  return ret;
}

example* blocking_get_delay_example(size_t thread)
{
  size_t index = delay_indicies[thread] % ring_size;
  pthread_mutex_lock(&delay);
  while(delay_ring[index] == NULL)
    pthread_cond_wait(&delay_nonempty, &delay);
  pthread_mutex_unlock(&delay);
  
  return get_delay_example(thread);
}

void delay_example(example* ex)
{
  size_t delay_count = mesg;
  if (global.training && ((label_data*)(ex->ld))->label != FLT_MAX)
    delay_count += compute_threads;
  
  if (delay_count == 0)
    {
      ex->threads_to_finish = 1;
      finish_example(ex);
    }
  else
    {
      size_t index = master_index % ring_size;
      
      pthread_mutex_lock(&delay);
      while (delay_ring[index] != NULL)
	pthread_cond_wait(&delay_empty, &delay);
      
      delay_ring[index] = ex;
      threads_to_use[index] = delay_count;
      ex->threads_to_finish = delay_count;
      
      master_index++;
      pthread_cond_broadcast(&delay_nonempty);
      pthread_mutex_unlock(&delay);
    }
}
