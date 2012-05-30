/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <stdio.h>
#include <float.h>

#include "cache.h"
#include "io.h"
#include "parse_regressor.h"
#include "parser.h"
#include "parse_args.h"
#include "sender.h"
#include "network.h"
#include "global_data.h"
#include "oaa.h"
#include "ect.h"
#include "csoaa.h"
#include "wap.h"
#include "sequence.h"
#include "searn.h"
#include "bfgs.h"
#include "lda_core.h"
#include "noop.h"
#include "gd_mf.h"

using namespace std;
//
// Does string end with a certain substring?
//
bool ends_with(string const &fullString, string const &ending)
{
    if (fullString.length() > ending.length()) {
        return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    } else {
        return false;
    }
}

size_t next_pow2(size_t x) {
  int i = 0;
  x = x > 0 ? x - 1 : 0;
  while (x > 0) {
    x >>= 1;
    i++;
  }
  return 1 << i;
}

vw parse_args(int argc, char *argv[])
{
  po::options_description desc("VW options");
  
  vw all;
  long int random_seed = 0;
  all.program_name = argv[0];
  // Declare the supported options.
  desc.add_options()
    ("help,h","Look here: http://hunch.net/~vw/ and click on Tutorial.")
    ("active_learning", "active learning mode")
    ("active_simulation", "active learning simulation mode")
    ("active_mellowness", po::value<float>(&all.active_c0), "active learning mellowness parameter c_0. Default 8")
    ("adaptive", "use adaptive, individual learning rates.")
    ("exact_adaptive_norm", "use a more expensive exact norm for adaptive learning rates.")
    ("audit,a", "print weights of features")
    ("bit_precision,b", po::value<size_t>(),
     "number of bits in the feature table")
    ("bfgs", "use bfgs optimization")
    ("cache,c", "Use a cache.  The default is <data>.cache")
    ("cache_file", po::value< vector<string> >(), "The location(s) of cache_file.")
    ("compressed", "use gzip format whenever possible. If a cache file is being created, this option creates a compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection.")
    ("conjugate_gradient", "use conjugate gradient based optimization")
    ("csoaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> costs")
    ("wap", po::value<size_t>(), "Use weighted all-pairs multiclass learning with <k> costs")
    ("csoaa_ldf", "Use one-against-all multiclass learning with label dependent features")
    ("wap_ldf", "Use weighted all-pairs multiclass learning with label dependent features")
    ("nonormalize", "Do not normalize online updates")
    ("l1", po::value<float>(&all.l1_lambda), "l_1 lambda")
    ("l2", po::value<float>(&all.l2_lambda), "l_2 lambda")
    ("data,d", po::value< string >(), "Example Set")
    ("daemon", "persistent daemon mode on port 26542")
    ("num_children", po::value<size_t>(&all.num_children), "number of children for persistent daemon mode")
    ("pid_file", po::value< string >(), "Write pid file in persistent daemon mode")
    ("decay_learning_rate",    po::value<float>(&all.eta_decay_rate),
     "Set Decay factor for learning_rate between passes")
    ("input_feature_regularizer", po::value< string >(&all.per_feature_regularizer_input), "Per feature regularization input file")
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("readable_model", po::value< string >(), "Output human-readable final regressor")
    ("hash", po::value< string > (), "how to hash the features. Available options: strings, all")
    ("hessian_on", "use second derivative in line search")
    ("version","Version information")
    ("ignore", po::value< vector<unsigned char> >(), "ignore namespaces beginning with character <arg>")
    ("keep", po::value< vector<unsigned char> >(), "keep namespaces beginning with character <arg>")
    ("initial_weight", po::value<float>(&all.initial_weight), "Set all weights to an initial value of 1.")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_pass_length", po::value<size_t>(&all.pass_length), "initial number of examples per pass")
    ("initial_t", po::value<double>(&(all.sd->t)), "initial t value")
    ("lda", po::value<size_t>(&all.lda), "Run lda with <int> topics")
    ("lda_alpha", po::value<float>(&all.lda_alpha), "Prior on sparsity of per-document topic weights")
    ("lda_rho", po::value<float>(&all.lda_rho), "Prior on sparsity of topic distributions")
    ("lda_D", po::value<float>(&all.lda_D), "Number of documents")
    ("minibatch", po::value<size_t>(&all.minibatch), "Minibatch size, for LDA")
    ("span_server", po::value<string>(&all.span_server), "Location of server for setting up spanning tree")
    ("min_prediction", po::value<double>(&all.sd->min_label), "Smallest prediction to output")
    ("max_prediction", po::value<double>(&all.sd->max_label), "Largest prediction to output")
    ("mem", po::value<int>(&all.m), "memory in bfgs")
    ("noconstant", "Don't add a constant feature")
    ("noop","do no learning")
    ("oaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> labels")
    //("ect", po::value<size_t>(), "Use error correcting tournament with <k> labels")
    //("errors", po::value<size_t>()->default_value(0), "Errors allowed in an error correcting tournament")
    ("output_feature_regularizer_binary", po::value< string >(&all.per_feature_regularizer_output), "Per feature regularization output file")
    ("output_feature_regularizer_text", po::value< string >(&all.per_feature_regularizer_text), "Per feature regularization output file, in text")
    ("port", po::value<size_t>(),"port to listen on")
    ("power_t", po::value<float>(&all.power_t), "t power value")
    ("learning_rate,l", po::value<float>(&all.eta), "Set Learning Rate")
    ("passes", po::value<size_t>(&all.numpasses),"Number of Training Passes")
    ("termination", po::value<float>(&all.rel_threshold),"Termination threshold")
    ("predictions,p", po::value< string >(), "File to output predictions to")
    ("quadratic,q", po::value< vector<string> > (),
     "Create and use quadratic features")
    ("quiet", "Don't output diagnostics")
    ("rank", po::value<size_t>(&all.rank), "rank for matrix factorization.")
    ("random_weights", po::value<bool>(&all.random_weights), "make initial weights random")
    ("random_seed", po::value<long int>(&random_seed), "seed random number generator")
    ("raw_predictions,r", po::value< string >(),
     "File to output unnormalized predictions to")
    ("save_per_pass", "Save the model after every pass over data")
    ("sendto", po::value< vector<string> >(), "send examples to <host>")
    ("sequence", po::value<size_t>(), "Do sequence prediction with <k> labels per element")
    ("sequence_history", po::value<size_t>(), "Prediction history length for sequences")
    ("sequence_bigrams", "enable bigrams on prediction history")
    ("sequence_features", po::value<size_t>(), "create history predictions x features")
    ("sequence_bigram_features", "enable history bigrams for sequence_features")
    ("sequence_rollout", po::value<size_t>(), "maximum rollout length")
    ("sequence_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
    ("sequence_beta", po::value<float>(), "interpolation rate for policies")
    ("sequence_gamma", po::value<float>(), "discount rate for policies")
    ("sequence_max_length", po::value<size_t>(), "maximum length of sequences (default 256)")
    ("sequence_transition_file", po::value<string>(), "read valid transitions from file (default all valid)")
    ("sequence_allow_current_policy", "allow sequence labeling to use the current policy")
    ("sequence_beam", po::value<size_t>(), "set the beam size for sequence prediction (default: 1 == greedy)")
    ("searn", po::value<string>(), "use searn, argument=task (eg., sequence)")
    ("searn_max_action", po::value<size_t>(), "largest action id that will be encountered in searn")

    ("searn_rollout", po::value<size_t>(), "maximum rollout length")
    ("searn_passes_per_policy", po::value<size_t>(), "maximum number of datapasses per policy")
    ("searn_beta", po::value<float>(), "interpolation rate for policies")
    ("searn_gamma", po::value<float>(), "discount rate for policies")
    ("searn_recombine", "allow searn labeling to use the current policy")
    ("searn_allow_current_policy", "allow searn labeling to use the current policy")

    ("testonly,t", "Ignore label information and just test")
    ("loss_function", po::value<string>()->default_value("squared"), "Specify the loss function to be used, uses squared by default. Currently available ones are squared, classic, hinge, logistic and quantile.")
    ("quantile_tau", po::value<double>()->default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5")

    ("unique_id", po::value<size_t>(&all.unique_id),"unique id used for cluster parallel jobs")
    ("total", po::value<size_t>(&all.total),"total number of nodes used in cluster parallel job")    
    ("node", po::value<size_t>(&all.node),"node number in cluster parallel job")    

    ("sort_features", "turn this on to disregard order in which features have been defined. This will lead to smaller cache sizes")
    ("ngram", po::value<size_t>(), "Generate N grams")
    ("skips", po::value<size_t>(), "Generate skips in N grams. This in conjunction with the ngram tag can be used to generate generalized n-skip-k-gram.");

  po::positional_options_description p;
  // Be friendly: if -d was left out, treat positional param as data file
  p.add("data", -1);

  po::variables_map vm;

  po::store(po::command_line_parser(argc, argv).
	    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
	    options(desc).positional(p).run(), vm);
  po::notify(vm);

  all.sd->weighted_unlabeled_examples = all.sd->t;
  all.initial_t = all.sd->t;

  if (vm.count("help") || argc == 1) {
    /* upon direct query for help -- spit it out to stdout */
    cout << "\n" << desc << "\n";
    exit(0);
  }

  if (vm.count("quiet"))
    all.quiet = true;
  else
    all.quiet = false;

  srand48(random_seed);

  if (vm.count("active_simulation"))
      all.active_simulation = true;

  if (vm.count("active_learning") && !all.active_simulation)
    all.active = true;

  if (vm.count("adaptive") || vm.count("exact_adaptive_norm")) {
      all.adaptive = true;
      if (vm.count("exact_adaptive_norm"))
	{
	  all.exact_adaptive_norm = true;
	  if (vm.count("nonormalize"))
	    cout << "Options don't make sense.  You can't use an exact norm and not normalize." << endl;
	}
      all.stride = 2;
  }
  
  void (*base_learner)(vw& all, example*) = learn_gd;
  void (*base_finish)(vw& all) = finish_gd;
  
  if (vm.count("bfgs") || vm.count("conjugate_gradient")) {
    all.driver = BFGS::drive_bfgs;
    base_learner = BFGS::learn;
    base_finish = BFGS::finish;
    all.bfgs = true;
    all.stride = 4;
    
    if (vm.count("hessian_on") || all.m==0) {
      all.hessian_on = true;
    }
    if (!all.quiet) {
      if (all.m>0)
	cerr << "enabling BFGS based optimization ";
      else
	cerr << "enabling conjugate gradient optimization via BFGS ";
      if (all.hessian_on)
	cerr << "with curvature calculation" << endl;
      else
	cerr << "**without** curvature calculation" << endl;
    }
    if (all.numpasses < 2)
      {
	cout << "you must make at least 2 passes to use BFGS" << endl;
	exit(1);
      }
  }


  if (vm.count("version") || argc == 1) {
    /* upon direct query for version -- spit it out to stdout */
    cout << version << "\n";
    exit(0);
  }


  if(vm.count("ngram")){
    all.ngram = vm["ngram"].as<size_t>();
    if(!vm.count("skip_gram")) cerr << "You have chosen to generate " << all.ngram << "-grams" << endl;
    if(vm.count("sort_features"))
      {
	cerr << "ngram is incompatible with sort_features.  " << endl;
	exit(1);
      }
  }
  if(vm.count("skips"))
    {
    all.skips = vm["skips"].as<size_t>();
    if(!vm.count("ngram"))
      {
	cout << "You can not skip unless ngram is > 1" << endl;
	exit(1);
      }
    cerr << "You have chosen to generate " << all.skips << "-skip-" << all.ngram << "-grams" << endl;
    if(all.skips > 4)
      {
      cout << "*********************************" << endl;
      cout << "Generating these features might take quite some time" << endl;
      cout << "*********************************" << endl;
      }
    }
  if (vm.count("bit_precision"))
    {
      all.default_bits = false;
      all.num_bits = vm["bit_precision"].as< size_t>();
      if (all.num_bits > sizeof(size_t)*8 - 3)
	{
	  cout << "Only " << sizeof(size_t) - 3 << " or fewer bits allowed.  If this is a serious limit, speak up." << endl;
	  exit(1);
	}
    }
  
  if (vm.count("daemon") || vm.count("pid_file") || vm.count("port")) {
    all.daemon = true;

    // allow each child to process up to 1e5 connections
    all.numpasses = (size_t) 1e5;
  }

  string data_filename = vm["data"].as<string>();
  if (vm.count("compressed") || ends_with(data_filename, ".gz"))
    set_compressed(all.p);

  if(vm.count("sort_features"))
    all.p->sort_features = true;

  if (vm.count("quadratic"))
    {
      all.pairs = vm["quadratic"].as< vector<string> >();
      if (!all.quiet)
	{
	  cerr << "creating quadratic features for pairs: ";
	  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) {
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

  for (size_t i = 0; i < 256; i++)
    all.ignore[i] = false;
  all.ignore_some = false;

  if (vm.count("ignore"))
    {
      all.ignore_some = true;

      vector<unsigned char> ignore = vm["ignore"].as< vector<unsigned char> >();
      for (vector<unsigned char>::iterator i = ignore.begin(); i != ignore.end();i++)
	{
	  all.ignore[*i] = true;
	}
      if (!all.quiet)
	{
	  cerr << "ignoring namespaces beginning with: ";
	  for (vector<unsigned char>::iterator i = ignore.begin(); i != ignore.end();i++)
	    cerr << *i << " ";

	  cerr << endl;
	}
    }

  if (vm.count("keep"))
    {
      for (size_t i = 0; i < 256; i++)
        all.ignore[i] = true;

      all.ignore_some = true;

      vector<unsigned char> keep = vm["keep"].as< vector<unsigned char> >();
      for (vector<unsigned char>::iterator i = keep.begin(); i != keep.end();i++)
	{
	  all.ignore[*i] = false;
	}
      if (!all.quiet)
	{
	  cerr << "using namespaces beginning with: ";
	  for (vector<unsigned char>::iterator i = keep.begin(); i != keep.end();i++)
	    cerr << *i << " ";

	  cerr << endl;
	}
    }

  // matrix factorization enabled
  if (all.rank > 0) {
    // store linear + 2*rank weights per index, round up to power of two
    float temp = ceilf(logf((float)(all.rank*2+1)) / logf (2.f));
    all.stride = 1 << (int) temp;
    all.random_weights = true;
    if (vm.count("adaptive") || vm.count("exact_adaptive_norm"))
      {
	cerr << "adaptive is not implemented for matrix factorization" << endl;
	exit (1);
      }
    if (vm.count("bfgs") || vm.count("conjugate_gradient"))
      {
	cerr << "bfgs is not implemented for matrix factorization" << endl;
	exit (1);
      }	
  }

  if (vm.count("noconstant"))
    all.add_constant = false;

  if (vm.count("nonormalize"))
    all.nonormalize = true;

  if (vm.count("lda"))
    {
      all.driver = drive_lda;
      all.p->sort_features = true;
      float temp = ceilf(logf((float)(all.lda*2+1)) / logf (2.f));
      all.stride = 1 << (int) temp;
      all.random_weights = true;
      all.add_constant = false;
    }

  if (vm.count("lda") && all.eta > 1.)
    {
      cerr << "your learning rate is too high, setting it to 1" << endl;
      all.eta = min(all.eta,1.f);
    }
  if (!vm.count("lda")) 
    all.eta *= pow(all.sd->t, (double)all.power_t);

  if (vm.count("minibatch")) {
    size_t minibatch2 = next_pow2(all.minibatch);
    all.p->ring_size = all.p->ring_size > minibatch2 ? all.p->ring_size : minibatch2;
  }

  if (vm.count("sequence_max_length")) {
    size_t maxlen = vm["sequence_max_length"].as<size_t>();
    all.p->ring_size = (all.p->ring_size > maxlen) ? all.p->ring_size : maxlen;
  }

  parse_regressor_args(all, vm, all.final_regressor_name, all.quiet);
  parse_source_args(all, vm, all.quiet,all.numpasses);
  if (vm.count("readable_model"))
    all.text_regressor_name = vm["readable_model"].as<string>();
  
  if (vm.count("save_per_pass"))
    all.save_per_pass = true;

  if (vm.count("min_prediction"))
    all.sd->min_label = vm["min_prediction"].as<double>();
  if (vm.count("max_prediction"))
    all.sd->max_label = vm["max_prediction"].as<double>();
  if (vm.count("min_prediction") || vm.count("max_prediction") || vm.count("testonly"))
    set_minmax = noop_mm;

  string loss_function;
  if(vm.count("loss_function"))
    loss_function = vm["loss_function"].as<string>();
  else
    loss_function = "squaredloss";
  double loss_parameter = 0.0;
  if(vm.count("quantile_tau"))
    loss_parameter = vm["quantile_tau"].as<double>();

  if (vm.count("noop"))
    all.driver = drive_noop;
  
  if (all.rank != 0) {
    all.driver = drive_gd_mf;
    loss_function = "classic";
    cerr << "Forcing classic squared loss for matrix factorization" << endl;
  }

  all.loss = getLossFunction(all.sd, loss_function, loss_parameter);

  if (pow((double)all.eta_decay_rate, (double)all.numpasses) < 0.0001 )
    cerr << "Warning: the learning rate for the last pass is multiplied by: " << pow((double)all.eta_decay_rate, (double)all.numpasses)
	 << " adjust --decay_learning_rate larger to avoid this." << endl;

  if (!all.quiet)
    {
      cerr << "Num weight bits = " << all.num_bits << endl;
      cerr << "learning rate = " << all.eta << endl;
      cerr << "initial_t = " << all.sd->t << endl;
      cerr << "power_t = " << all.power_t << endl;
      if (all.numpasses > 1)
	cerr << "decay_learning_rate = " << all.eta_decay_rate << endl;
      if (all.rank > 0)
	cerr << "rank = " << all.rank << endl;
    }

  if (vm.count("predictions")) {
    if (!all.quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      {
	push(all.final_prediction_sink, (size_t) 1);//stdout
      }
    else
      {
	const char* fstr = (vm["predictions"].as< string >().c_str());
	int f = fileno(fopen(fstr,"w"));
	if (f < 0)
	  cerr << "Error opening the predictions file: " << fstr << endl;
	push(all.final_prediction_sink, (size_t) f);
      }
  }

  if (vm.count("raw_predictions")) {
    if (!all.quiet)
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      all.raw_prediction = 1;//stdout
    else
      all.raw_prediction = fileno(fopen(vm["raw_predictions"].as< string >().c_str(), "w"));
  }

  if (vm.count("audit"))
    all.audit = true;

  if (vm.count("sendto"))
    {
      all.driver = drive_send;
      parse_send_args(vm, all.pairs);
    }

  if (vm.count("testonly"))
    {
      if (!all.quiet)
	cerr << "only testing" << endl;
      all.training = false;
      if (all.lda > 0)
        all.eta = 0;
    }
  else
    all.training = true;

  if (all.l1_lambda < 0.) {
    cerr << "l1_lambda should be nonnegative: resetting from " << all.l1_lambda << " to 0" << endl;
    all.l1_lambda = 0.;
  }
  if (all.l2_lambda < 0.) {
    cerr << "l2_lambda should be nonnegative: resetting from " << all.l2_lambda << " to 0" << endl;
    all.l2_lambda = 0.;
  }
  all.reg_mode += (all.l1_lambda > 0.) ? 1 : 0;
  all.reg_mode += (all.l2_lambda > 0.) ? 2 : 0;
  if (!all.quiet)
    {
      if (all.reg_mode %2)
	cerr << "using l1 regularization" << endl;
      if (all.reg_mode > 1)
	cerr << "using l2 regularization" << endl;
    }

  if (all.bfgs) {
    BFGS::initializer(all);
  }

  void (*mc_learner)(vw&, example*) = NULL;
  void (*mc_finish)(vw&) = NULL;

  if(vm.count("oaa"))
    {
      OAA::parse_flags(all, vm["oaa"].as<size_t>(), base_learner, base_finish);
      mc_learner = OAA::learn;
      mc_finish = OAA::finish;
    }
  else if (vm.count("ect"))
    {
      ECT::parse_flags(all, vm["ect"].as<size_t>(), vm["errors"].as<size_t>(), base_learner, base_finish);
      mc_learner = ECT::learn;
      mc_finish = ECT::finish;
    }

  void (*cs_learner)(vw&,example*) = CSOAA::learn;
  void (*cs_finish)(vw&) = CSOAA::finish;

  if(vm.count("wap"))
    {
      WAP::parse_flags(all, vm["wap"].as<size_t>(), base_learner, base_finish);
      cs_learner = WAP::learn;
      cs_finish = WAP::finish;
    }

  if(vm.count("csoaa_ldf")) {
    if (all.add_constant) {
      cerr << "warning: turning off constant for label dependent features; use --noconstant" << endl;
      all.add_constant = false;
    }
    CSOAA_LDF::parse_flags(all, 0, base_learner, base_finish);
    cs_learner = CSOAA_LDF::learn;
    cs_finish  = CSOAA_LDF::finish;
  }

  if(vm.count("wap_ldf")) {
    if (all.add_constant) {
      cerr << "warning: turning off constant for label dependent features; use --noconstant" << endl;
      all.add_constant = false;
    }
    WAP_LDF::parse_flags(all, 0, base_learner, base_finish);
    cs_learner = WAP_LDF::learn;
    cs_finish  = WAP_LDF::finish;
  }


  if(vm.count("csoaa"))
    CSOAA::parse_flags(all, vm["csoaa"].as<size_t>(), base_learner, base_finish);

  if (vm.count("sequence")) {
    if (vm.count("wap")) 
      ;
    else
      CSOAA::parse_flags(all, vm["sequence"].as<size_t>(), base_learner, base_finish);  // default to CSOAA unless wap is specified

    parse_sequence_args(all, vm, cs_learner, cs_finish);
    all.driver = drive_sequence;
    all.sequence = true;

    if (vm.count("searn")) {
      cerr << "error: you cannot use searn and sequence simultaneously" << endl;
      exit(-1);
    }
  }
  if (vm.count("searn")) {
    size_t max_action = 1;
    if (vm.count("searn_max_action"))
      max_action = vm["searn_max_action"].as<size_t>();
    else {
      cerr << "error: you must specify --searn_max_action" << endl;
      exit(-1);
    }

    if (vm.count("wap"))
      ;
    else
      CSOAA::parse_flags(all, max_action, base_learner, base_finish);  // default to CSOAA unless wap is specified

    Searn::parse_args(all, vm, cs_learner, cs_finish);
    all.driver = Searn::drive;
    all.searn = true;
  }

  return all;
}

