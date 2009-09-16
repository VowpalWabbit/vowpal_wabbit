/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <netdb.h>
#include "fcntl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "cache.h"
#include "io.h"
#include "parse_regressor.h"
#include "parser.h"
#include "parse_args.h"
#include "sender.h"

void parse_args(int argc, char *argv[], boost::program_options::options_description& desc,
		gd_vars& vars, float& eta_decay_rate,
		size_t &passes, regressor &r, parser* par,
		string& final_regressor_name,
		int &sum_sock)
{
  string comment;
  parse_args(argc, argv, desc,
	     vars, eta_decay_rate,
	     passes, r, par,
	     final_regressor_name,
	     sum_sock, comment);
}

const float default_decay = 1. / sqrt(2.);

po::variables_map parse_args(int argc, char *argv[], boost::program_options::options_description& desc,
		gd_vars& vars, float& eta_decay_rate,
		size_t &passes, regressor &r, parser* par,
		string& final_regressor_name,
		int &sum_sock, string &comment)
{
  vars.init();
  size_t keep = 0;
  size_t of = 1;
  global_data* sd = (global_data*)calloc(1,sizeof(global_data));
  sd->program_name = argv[0];
  // Declare the supported options.
  desc.add_options()
    ("audit,a", "print weights of features")
    ("bit_precision,b", po::value<size_t>(&sd->num_bits)->default_value(18), 
     "number of bits in the feature table")
    ("cache,c", "Use a cache.  The default is <data>.cache")
    ("cache_file", po::value< vector<string> >(), "The location(s) of cache_file.")
    ("data,d", po::value< string >()->default_value(""), "Example Set")
    ("daemon", "read data from port 39523")
    ("decay_learning_rate",    po::value<float>(&eta_decay_rate)->default_value(default_decay), 
     "Set Decay factor for learning_rate between passes")
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("help,h","Output Arguments")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_t", po::value<float>(&vars.t)->default_value(1.), "initial t value")
    ("keep,k", po::value<size_t>(&keep)->default_value(0), "Features to keep")
    ("min_prediction", po::value<float>(&vars.min_prediction)->default_value(0), "Smallest prediction to output")
    ("max_prediction", po::value<float>(&vars.max_prediction)->default_value(1), "Largest prediction to output")
    ("multisource", po::value<float>(), "multiple sources for daemon input")
    ("of", po::value<size_t>(&of)->default_value(1), "keep k of <n> features")
    ("power_t", po::value<float>(&vars.power_t)->default_value(0.), "t power value")
    ("predictto", po::value< string > (), "host to send predictions to")
    ("learning_rate,l", po::value<float>(&vars.eta)->default_value(0.1), 
     "Set Learning Rate")
    ("passes", po::value<size_t>(&passes)->default_value(1), 
     "Number of Training Passes")
    ("predictions,p", po::value< string >(), "File to output predictions to")
    ("quadratic,q", po::value< vector<string> > (),
     "Create and use quadratic features")
    ("quiet", "Don't output diagnostics")
    ("raw_predictions,r", po::value< string >(), 
     "File to output unnormalized predictions to")
    ("sendto", po::value< vector<string> >(), "send example to <hosts>")
    ("testonly,t", "Ignore label information and just test")
    ("thread_bits", po::value<size_t>(&sd->thread_bits)->default_value(0), "log_2 threads")
    ("comment,z", po::value< string >(), "Comment field.")
    ("loss_function", po::value<string>()->default_value("squaredloss"), "Specify the loss function to be used, uses squaredloss by default. Currently available ones are squaredloss, hingeloss, logloss and quantilesloss.")
    ("quantiles_tau", po::value<double>()->default_value(0.0), "Parameter \\tau associated with Quantiles loss. Unless mentioned this parameter would default to a value of 0.0")
    ("unique_id", po::value<int>(&sd->unique_id)->default_value(0),"unique id used for cluster parallel");

  r.global=sd;
  par->global=sd;
  
  sd->example_number = 0;
  sd->weighted_examples = 0.;
  sd->old_weighted_examples = 0.;
  sd->weighted_labels = 0.;
  sd->total_features = 0;
  sd->sum_loss = 0.0;
  sd->sum_loss_since_last_dump = 0.0;
  sd->dump_interval = exp(1.);

  sd->predictions = -1;
  sd->raw_predictions = -1;
  sd->network_prediction = -1;
  sd->print = print_result;
  
  po::positional_options_description p;
  
  po::variables_map vm;

  po::store(po::command_line_parser(argc, argv).
	    options(desc).positional(p).run(), vm);
  po::notify(vm);
  
  if (vm.count("help") || argc == 1) {
    cerr << "\n" << desc << "\n";
    exit(1);
  }

  if (sd->num_bits > 31) {
    cerr << "The system limits at 31 bits of precision!\n" << endl;
    exit(1);
  }
  if (vm.count("quiet"))
    sd->quiet = true;
  else
    sd->quiet = false;

  if (vm.count("quadratic")) 
    {
      sd->pairs = vm["quadratic"].as< vector<string> >();
      if (!sd->quiet)
	{
	  cerr << "creating quadratic features for pairs: ";
	  for (vector<string>::iterator i = sd->pairs.begin(); i != sd->pairs.end();i++) {
	    cerr << *i << " ";
	    if (i->length() > 2)
	      cerr << endl << "warning, ignoring characters after the 2nd.\n";
	    if (i->length() < 2) {
	      cerr << endl << "error, quadratic features must involve two sets.\n";
	      exit(0);
	    }
	  }
	  cerr << endl;
	}
    }

  parse_regressor_args(vm, r, final_regressor_name, !sd->quiet);

  string loss_function;
  if(vm.count("loss_function")) 
	  loss_function = vm["loss_function"].as<string>();
  else
	  loss_function = "squaredloss";

  double loss_parameter = 0.0;
  if(vm.count("quantiles_tau"))
	  loss_parameter = vm["quantiles_tau"].as<double>();
  r.loss = getLossFunction(loss_function, loss_parameter);

  vars.eta *= pow(vars.t, vars.power_t);
  if (!sd->quiet)
    {
      cerr << "Num weight bits = " << r.global->num_bits << endl;
      cerr << "learning rate = " << vars.eta << endl;
      cerr << "initial_t = " << vars.t << endl;
      cerr << "power_t = " << vars.power_t << endl;
      cerr << "decay_learning_rate = " << eta_decay_rate << endl;
    }
  
  if (eta_decay_rate != default_decay && passes == 1)
    cerr << "Warning: decay_learning_rate has no effect when there is only one pass" << endl;

  if (pow(eta_decay_rate, passes) < 0.0001 )
    cerr << "Warning: the learning rate for the last pass is multiplied by: " << pow(eta_decay_rate, passes) 
	 << " adjust to --decay_learning_rate larger to avoid this." << endl;
  
  parse_source_args(vm,par,sd->quiet,passes);
  
  if (vm.count("predictions")) {
    if (!sd->quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      sd->predictions = 1;//stdout
    else 
      {
	const char* fstr = (vm["predictions"].as< string >().c_str());
	sd->predictions = fileno(fopen(fstr,"w"));
	if (sd->predictions < 0)
	  cerr << "Error opening the predictions file: " << fstr << endl;
      }
  }
  
  if (vm.count("raw_predictions")) {
    if (!sd->quiet)
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      sd->raw_predictions = 1;//stdout
    else
      sd->raw_predictions = fileno(fopen(vm["raw_predictions"].as< string >().c_str(), "w"));
  }

  if (vm.count("audit"))
    par->global->audit = true;
  else 
    par->global->audit = false;

  parse_send_args(vm, sd->pairs, sd->thread_bits);

  if (vm.count("comment")) 
    {
      comment = vm["comment"].as<string>();
    }
  
  if (vm.count("testonly"))
    {
      if (!sd->quiet)
	cerr << "only testing" << endl;
      sd->training = false;
    }
  else 
    {
      sd->training = true;
      if (!sd->quiet)
	cerr << "learning_rate set to " << vars.eta << endl;
    }

  if (vm.count("predictto"))
    {
      if (!sd->quiet)
	cerr << "predictto = " << vm["predictto"].as< string >() << endl;
      hostent* he = gethostbyname(vm["predictto"].as< string >().c_str());
      if (he == NULL)
	{
	  cerr << "can't resolve hostname" << endl;
	  exit(1);
	}
      sum_sock = socket(PF_INET, SOCK_STREAM, 0);
      if (sum_sock == -1)
	{
	  cerr << "can't get socket " << endl;
	  exit(1);
	}
      sockaddr_in far_end;
      far_end.sin_family = AF_INET;
      far_end.sin_port = htons(39524);
      far_end.sin_addr = *(in_addr*)(he->h_addr);
      memset(&far_end.sin_zero, '\0',8);
      if (connect(sum_sock,(sockaddr*)&far_end, sizeof(far_end)) == -1)
	{
	  cerr << "can't connect." << endl;
	  exit(1);
	}
      else 
	{
	  int flag = 1;
	  if (setsockopt(sum_sock,IPPROTO_TCP,TCP_NODELAY,(char*) &flag, sizeof(int)) == -1)
	    cerr << strerror(errno) << " " << errno << " " << IPPROTO_TCP << endl;

	  sd->network_prediction = sum_sock;
	  write(sum_sock,&(sd->unique_id),sizeof(sd->unique_id));
	  fsync(sum_sock);
	}
    }
  return vm;
}

