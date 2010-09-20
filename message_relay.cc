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
      //      cout << "ec->final_prediction local = " << ec->final_prediction << endl;
      ec->final_prediction = ps.p;
      //      cout << "ec->final_prediction global = " << ec->final_prediction << endl;
      ec->global_weight = ps.weight;
      label_data* ld = (label_data*)ec->ld;

      //      cout << "ld->label = " << ld->label << endl;
      ec->loss = global.loss->getLoss(ec->final_prediction, ld->label) * ld->weight;
      
      if (global.backprop)
	{
	  ec->eta_round = global.reg->loss->getUpdate(ec->final_prediction, ld->label, vars->eta/pow(ec->example_t,vars->power_t), ec->total_sum_feat_sq, ps.weight);
	  delay_global_example(ec,global.num_threads());
	}
      else if (global.delayed_global)
	{
	  ec->eta_round = global.reg->loss->getUpdate(ec->final_prediction, ld->label, vars->eta/pow(ec->example_t,vars->power_t), ec->total_sum_feat_sq, ld->weight);
	  delay_global_example(ec,global.num_threads());
	}
      else if (global.corrective)
	{
	  //cout << "ec->eta_round local = " << ec->eta_round << endl;
	  ec->eta_round = global.reg->loss->getUpdate(ec->final_prediction, ld->label, vars->eta/pow(ec->example_t,vars->power_t), ec->total_sum_feat_sq, ld->weight) - ec->eta_round;
	  //cout << "ec->eta_round final = " << ec->eta_round << endl;
	  delay_global_example(ec,global.num_threads());
	}
      else
	delay_global_example(ec,0);
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

