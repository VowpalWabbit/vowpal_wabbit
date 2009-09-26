#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include "v_array.h"
#include "example.h"
#include "global_data.h"

v_array<size_t> thread_train_index;//thread specific state.

v_array<example*> train_ring;//train_ring state & mutexes
v_array<size_t> threads_to_train;
size_t master_index;
pthread_mutex_t train = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t train_nonempty = PTHREAD_COND_INITIALIZER;

void initialize_train_ring()
{
  size_t nt = global.num_threads();
  reserve(thread_train_index, nt);
  for (size_t i = 0; i < nt; i++)
    thread_train_index[i] = 0;
  reserve(train_ring,ring_size);
  reserve(threads_to_train,ring_size);
  for (size_t i = 0; i < ring_size; i++)
    train_ring[i] = NULL;
}

void destroy_train_ring()
{
  free(thread_train_index.begin);
  free(train_ring.begin);
  free(threads_to_train.begin);
}

bool thread_done(size_t thread)
{
  bool ret;
  pthread_mutex_lock(&train);
  ret = (thread_train_index[thread] == master_index);
  pthread_mutex_unlock(&train);
  return ret;
}

example* get_train_example(size_t thread)
{//nonblocking
  size_t index = thread_train_index[thread] % ring_size;
  example* ret = train_ring[index];
  if (ret != NULL)
    {
      thread_train_index[thread]++;

      pthread_mutex_lock(&ret->lock);
      if (--threads_to_train[index] == 0) 
	{
	  pthread_mutex_lock(&train);
	  train_ring[index] = NULL;
	  pthread_cond_broadcast(&train_nonempty);
	  pthread_mutex_unlock(&train);
	}
      pthread_mutex_unlock(&ret->lock);
    }
  else
    {
    }
  return ret;
}

void insert_example(example* ec)
{
  size_t index = master_index % ring_size;

  pthread_mutex_lock(&train);

  while (train_ring[index] != NULL)
    {
      pthread_cond_wait(&train_nonempty, &train);
    }

  train_ring[index] = ec;
  threads_to_train[index] = global.num_threads();

  master_index++;
  pthread_mutex_unlock(&train);
}
