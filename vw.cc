/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

/*
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

A smaller change.
 */

#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "parse_regressor.h"
#include "parse_example.h"
#include "parse_args.h"
#include "gd.h"
#include "vw.h"

int sum_sock = -1;

void* go(void *in)
{
  go_params* params = (go_params*) in;
  regressor reg = params->reg;
  size_t thread_num = params->thread_num;
  example* ec = NULL;

  while ( (ec = get_example(ec,thread_num)) )
    {
      label_data* ld = (label_data*)ec->ld;
      if ( ec->num_features <= 1 && ((ld->tag).end - (ld->tag).begin == 4) 
	   && ((ld->tag)[0] == 's')&&((ld->tag)[1] == 'a')&&((ld->tag)[2] == 'v')&&((ld->tag)[3] == 'e'))
	{
	  if ((*(params->final_regressor_name)) != "") 
	    {
	      ofstream tempOut;
	      tempOut.open((*(params->final_regressor_name)).c_str());
	      dump_regressor(tempOut, reg);
	    }
	}
      else
	train_one_example(reg,ec,thread_num,*(params->vars));
    }

  return NULL;
}

gd_vars* vw(int argc, char *argv[])
{
  size_t numpasses;
  float eta_decay;
  ofstream final_regressor;
  string final_regressor_name;

  example_source source;
  parser* p = new_parser(&source,&simple_label);
  regressor regressor1;

  gd_vars *vars = new gd_vars;

  po::options_description desc("VW options");

  parse_args(argc, argv, desc, *vars, eta_decay, 
	     numpasses, regressor1, p, 
	     final_regressor_name, sum_sock);

  if (!vars->quiet)
    {
      const char * header_fmt = "%-10s %-10s %8s %8s %10s %8s %8s\n";
      fprintf(stderr, header_fmt,
	      "average", "since", "example", "example",
	      "current", "current", "current");
      fprintf(stderr, header_fmt,
	      "loss", "last", "counter", "weight", "label", "predict", "features");
      cerr.precision(5);
    }
  
  size_t num_threads = regressor1.global->num_threads();
  if (vars->daemon != -1)
    while(true)
      {
	sockaddr_in client_address;
	socklen_t size = sizeof(client_address);
	source.input.file = accept(vars->daemon,(sockaddr*)&client_address,&size);
	if (source.input.file < 0)
	  {
	    cerr << "bad client socket!" << endl;
	    exit (1);
	  }
	cerr << "reading data from port 39523" << endl;
	vars->predictions = source.input.file;

	setup_parser(num_threads, p);
	pthread_t threads[num_threads];
	
	go_params* passers[num_threads];
	
	for (size_t i = 0; i < num_threads; i++) 
	  {
	    passers[i] = (go_params*)calloc(1, sizeof(go_params));
	    passers[i]->init(vars,regressor1,&final_regressor_name,i);
	    pthread_create(&threads[i], NULL, go, (void *) passers[i]);
	  }
	
	for (size_t i = 0; i < num_threads; i++) 
	  {
	    pthread_join(threads[i], NULL);
	    free(passers[i]);
	  }
	destroy_parser(p);
      }
  else 
    for (; numpasses > 0; numpasses--) {
      setup_parser(num_threads, p);
      pthread_t threads[num_threads];
      
      go_params* passers[num_threads];
      
      for (size_t i = 0; i < num_threads; i++) 
	{
	  passers[i] = (go_params*)calloc(1, sizeof(go_params));
	  passers[i]->init(vars,regressor1,&final_regressor_name,i);
	  pthread_create(&threads[i], NULL, go, (void *) passers[i]);
	}
      
      for (size_t i = 0; i < num_threads; i++) 
	{
	  pthread_join(threads[i], NULL);
	  free(passers[i]);
	}
      destroy_parser(p);
      vars->eta *= eta_decay;
      reset_source(regressor1.global->num_bits, source);
    }
  
  if (final_regressor_name  != "")
    {
      final_regressor.open(final_regressor_name.c_str());
      dump_regressor(final_regressor, regressor1);
    }

  finalize_regressor(final_regressor,regressor1);
  finalize_source(source);
  source.global->pairs.~vector();
  free(source.global);
/*  float best_constant = vars.weighted_labels / vars.weighted_examples;
  float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
			 + (1.0 - best_constant)*best_constant*best_constant);

  if (!vars.quiet) 
    {
      cerr << endl << "finished run";
      cerr << endl << "number of examples = " << vars.example_number;
      cerr << endl << "weighted example sum = " << vars.weighted_examples;
      cerr << endl << "weighted label sum = " << vars.weighted_labels;
      cerr << endl << "average loss = " << vars.sum_loss / vars.weighted_examples;
      cerr << endl << "best constant's loss = " << constant_loss;
      cerr << endl << "total feature number = " << vars.total_features;
      cerr << endl;
    }

  return 0;
*/
  return vars;
}
