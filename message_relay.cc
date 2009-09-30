#include <stdlib.h>
#include <pthread.h>
#include "multisource.h"
#include "delay_ring.h"

void* mesg_relay(void* in)
{
  prediction ps;
  while (blocking_get_prediction(global.local_prediction,ps))
    {
      example *ec = blocking_get_delay_example(global.num_threads());
      ec->final_prediction = ps.p;
      label_data* ld = (label_data*)ec->ld;
      ec->loss = global.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      finish_example(ec);
    }
  return NULL;
}

pthread_t relay_thread;

void setup_relay()
{
  pthread_create(&relay_thread, NULL, mesg_relay, NULL);
}

void destroy_relay()
{
  pthread_join(relay_thread, NULL);
}

