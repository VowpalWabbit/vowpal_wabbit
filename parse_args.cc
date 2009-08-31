/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <netdb.h>
#include "fcntl.h"
#include "cache.h"
#include "io.h"
#include "parse_regressor.h"
#include "source.h"
#include "parse_args.h"
#include "sender.h"

//For OSX
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

void make_write_cache(size_t numbits, parser* par, string &newname, 
		      bool quiet)
{
  string temp = newname+string(".writing");
  push_many(par->output.currentname,temp.c_str(),temp.length()+1);
  
  par->output.file = open(temp.c_str(), O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
  if (par->output.file == -1) {
    cerr << "can't create cache file !" << endl;
    return;
  }
  char *p;
  buf_write(par->output, p, sizeof(size_t));
  
  *(size_t *)p = numbits;
  
  push_many(par->output.finalname,newname.c_str(),newname.length()+1);
  par->write_cache = true;
  if (!quiet)
    cerr << "creating cache_file = " << newname << endl;
}

void parse_cache(po::variables_map &vm, size_t numbits, string source,
		 parser* par, bool quiet, static_data* sd, size_t passes)
{
  par->global->mask = (1 << numbits) - 1;
  if (vm.count("cache") || vm.count("cache_file"))
    {
      string cache_string;
      if (vm.count("cache_file")) 
	cache_string = vm["cache_file"].as< string >();
      else 
	cache_string = source+string(".cache");
      par->input.file = 
	open(cache_string.c_str(), O_RDONLY|O_LARGEFILE);  
      if (par->input.file == -1)
	make_write_cache(numbits, par, cache_string, quiet);
      else {
	if (inconsistent_cache(numbits, par->input)) {
	  close(par->input.file);
	  par->input.space.erase();
	  make_write_cache(numbits, par, cache_string, quiet);
	}
	else {
	  if (!quiet)
	    cerr << "using cache_file = " << cache_string << endl;
	  par->reader = read_cached_features;
	  par->write_cache = false;
	}
      }
    }
  else
    {
      if (!quiet)
	cerr << "using no cache" << endl;
      if (passes > 1)
	cerr << sd->program_name << ": Warning only one pass will occur: try using --cache_file" << endl;
      reserve(par->output.space,0);
      par->output.file=-1;
      par->write_cache = false;
    }
}

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
  static_data* sd = (static_data*)calloc(1,sizeof(static_data));
  sd->program_name = argv[0];
  // Declare the supported options.
  desc.add_options()
    ("audit,a", "print weights of features")
    ("bit_precision,b", po::value<size_t>(&sd->num_bits)->default_value(18), 
     "number of bits in the feature table")
    ("cache,c", "Use a cache.  The default is <data>.cache")
    ("cache_file", po::value< string >(), "The location of a cache_file.")
    ("data,d", po::value< string >()->default_value(""), "Example Set")
    ("daemon", "read data from port 39523")
    ("decay_learning_rate",    po::value<float>(&eta_decay_rate)->default_value(default_decay), 
     "Set Decay factor for learning_rate between passes")
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("help,h","Output Arguments")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor")
    ("initial_t", po::value<float>(&vars.t)->default_value(1.), "initial t value")
    ("keep,k", po::value<size_t>(&keep)->default_value(0), "Features to keep")
    ("min_prediction", po::value<float>(&vars.min_prediction)->default_value(0), "Smallest prediction to output")
    ("max_prediction", po::value<float>(&vars.max_prediction)->default_value(1), "Largest prediction to output")
    ("of", po::value<size_t>(&of)->default_value(1), "keep k of <n> features")
    ("power_t", po::value<float>(&vars.power_t)->default_value(0.), "t power value")
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
    ("summer,s", po::value< string > (), "host to use as a summer")
    ("testonly,t", "Ignore label information and just test")
    ("thread_bits", po::value<size_t>(&sd->thread_bits)->default_value(0), "log_2 threads")
    ("comment,z", po::value< string >(), "Comment field.")
    ("loss_function", po::value<string>()->default_value("squaredloss"), "Specify the loss function to be used, uses squaredloss by default. Currently available ones are squaredloss, hingeloss, logloss and quantilesloss.")
    ("quantiles_tau", po::value<double>()->default_value(0.0), "Parameter \\tau associated with Quantiles loss. Unless mentioned this parameter would default to a value of 0.0");

  r.global=sd;
  par->global=sd;
  
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
    vars.quiet = true;

  if (vm.count("quadratic")) 
    {
      sd->pairs = vm["quadratic"].as< vector<string> >();
      if (!vars.quiet)
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

  vector<string> regs;
  if (vm.count("initial_regressor"))
    regs = vm["initial_regressor"].as< vector<string> >();

  parse_regressor(regs, r);
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
  if (!vars.quiet)
    {
      cerr << "Num weight bits = " << r.global->num_bits << endl;
      cerr << "learning rate = " << vars.eta << endl;
      cerr << "initial_t = " << vars.t << endl;
      cerr << "power_t = " << vars.power_t << endl;
      cerr << "decay_learning_rate = " << eta_decay_rate << endl;
    }

  if (eta_decay_rate != default_decay && vm.count("passes") <= 1)
    cerr << "Warning: decay_learning_rate has no effect when there is only one pass" << endl;

  par->reader = read_features;
  parse_cache(vm, sd->num_bits, vm["data"].as<string>(), par, vars.quiet, r.global, passes);

  if (vm.count("predictions")) {
    if (!vars.quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      vars.predictions = 1;//stdout
    else 
      {
	const char* fstr = (vm["predictions"].as< string >().c_str());
	//	vars.predictions = open(fstr, (O_WRONLY | O_CREAT));
	vars.predictions = fileno(fopen(fstr,"w"));
	if (vars.predictions < 0)
	  cerr << "Error opening the predictions file: " << fstr << endl;
      }
  }
  
  if (vm.count("raw_predictions")) {
    if (!vars.quiet)
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      vars.raw_predictions = 1;//stdout
    else
      vars.raw_predictions = fileno(fopen(vm["raw_predictions"].as< string >().c_str(), "w"));
  }

  if (par->input.file == -1)
    if (vm.count("daemon"))
      {
	vars.daemon = socket(PF_INET, SOCK_STREAM, 0);
	if (vars.daemon < 0) {
	    cerr << "can't open socket!" << endl;
	    exit(1);
	}
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(39523);
	
	if (bind(vars.daemon,(sockaddr*)&address, sizeof(address)) < 0)
	  {
	    cerr << "failure to bind!" << endl;
	    exit(1);
	  }
	listen(vars.daemon,1);
	
	sockaddr_in client_address;
	socklen_t size = sizeof(client_address);
	par->input.file = accept(vars.daemon,(sockaddr*)&client_address,&size);
	if (par->input.file < 0)
	  {
	    cerr << "bad client socket!" << endl;
	    exit (1);
	  }
	cerr << "reading data from port 39523" << endl;
	vars.predictions = par->input.file;
	par->reader = read_cached_features;
	reserve(par->output.space,0);
      }
    else if (vm.count("data"))
      { 
	string temp = vm["data"].as< string >();
	if (temp.length() != 0)
	  {
	    if (!vars.quiet)
	      cerr << "Reading from " << temp << endl;
	    par->input.file = open(temp.c_str(), O_RDONLY);
	    if (par->input.file == -1)
	      {
		cerr << "can't open " << temp << ", bailing!" << endl;
		exit(0);
	      }
	  }
      }
    else // Default to stdin
      {
	if (!vars.quiet)
	  cerr << "Reading from stdin" << endl;
	par->input.file = fileno(stdin);
      }
  if (vm.count("audit"))
    par->global->audit = true;
  else 
    par->global->audit = false;

  parse_send_args(vm, sd->pairs, sd->thread_bits);

  if (vm.count("final_regressor")) {
    final_regressor_name = vm["final_regressor"].as<string>();
    if (!vars.quiet)
      cerr << "final_regressor = " << vm["final_regressor"].as<string>() << endl;
  }
  else
    final_regressor_name = "";

  if (vm.count("comment")) 
    {
      comment = vm["comment"].as<string>();
    }
  
  if (vm.count("testonly"))
    {
      if (!vars.quiet)
	cerr << "only testing" << endl;
      vars.training = false;
    }
  else 
    if (!vars.quiet)
      cerr << "learning_rate set to " << vars.eta << endl;

  if (vm.count("summer"))
    {
      if (!vars.quiet)
	cerr << "summer = " << vm["summer"].as< string >() << endl;
      hostent* he = gethostbyname(vm["summer"].as< string >().c_str());
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
    }
  return vm;
}

