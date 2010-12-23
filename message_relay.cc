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
     if (global.backprop || global.delayed_global || global.corrective)
	ec->global_prediction = ps.p;
      else
	ec->final_prediction = ps.p;
      ec->global_weight = ps.weight;
      label_data* ld = (label_data*)ec->ld;

      ec->loss = global.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      
      if (global.backprop)
	{
	  ec->eta_global = global.reg->loss->getUpdate(ec->global_prediction, ld->label, global.global_multiplier*global.eta/pow(ec->example_t,vars->power_t)*ps.weight, ec->total_sum_feat_sq);
	  delay_global_example(ec);
	}
      else if (global.delayed_global)
	{
	  ec->eta_global = global.reg->loss->getUpdate(ec->global_prediction, ld->label, global.global_multiplier*global.eta/pow(ec->example_t,vars->power_t)*ld->weight, ec->total_sum_feat_sq);
	  delay_global_example(ec);
	}
      else if (global.corrective)
	{
	  ec->eta_global = global.reg->loss->getUpdate(ec->global_prediction, ld->label, global.global_multiplier*global.eta/pow(ec->example_t,vars->power_t)*ld->weight, ec->total_sum_feat_sq) - ec->eta_round;
	  delay_global_example(ec);
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

