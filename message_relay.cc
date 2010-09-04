#include <stdlib.h>
#include <pthread.h>
#include <float.h>
#include "multisource.h"
#include "delay_ring.h"
#include "gd.h"

void* mesg_relay(void* v)
{
  gd_vars* vars =(gd_vars*)v;
  global_prediction ps;
  while (blocking_get_global_prediction(global.local_prediction,ps))
    {
      example *ec = blocking_get_delay_example(global.num_threads());
      ec->final_prediction = ps.p;
      ec->global_weight = ps.weight;
      label_data* ld = (label_data*)ec->ld;
      cout << "received " << ps.p << "\t" << ps.weight << endl;
      
      ec->loss = global.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;

      if (global.backprop)
	{
	  if (global.training && (ld->label != FLT_MAX))
	    {
	      ec->eta_round = global.reg->loss->getUpdate(ec->final_prediction, ld->label, vars->eta/pow(vars->t,vars->power_t), ec->total_sum_feat_sq, ps.weight);
	      cout << "delaying global example" << endl;
	      delay_global_example(ec,global.num_threads());
	    }
	  else
	    delay_global_example(ec,0);
	}

      finish_example(ec);
    }
  return NULL;
}

pthread_t relay_thread;

void setup_relay(void* v)
{
  pthread_create(&relay_thread, NULL, mesg_relay, v);
}

void destroy_relay()
{
  pthread_join(relay_thread, NULL);
}

