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
#include "csoaa.h"
#include "wap.h"
#include "sequence.h"
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

const float default_decay = 1.;

po::variables_map parse_args(int argc, char *argv[], 
			     boost::program_options::options_description& desc,
			     parser* par)
{
  global.program_name = argv[0];
  global.sd = (shared_data *) malloc(sizeof(shared_data));
  // Declare the supported options.
  desc.add_options()
    ("help,h","Look here: http://hunch.net/~vw/ and click on Tutorial.")
    ("active_learning", "active learning mode")
    ("active_simulation", "active learning simulation mode")
    ("active_mellowness", po::value<float>(&global.active_c0)->default_value(8.f), "active learning mellowness parameter c_0. Default 8")
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
    ("l1", po::value<float>(&global.l1_lambda)->default_value(0.0), "l_1 lambda")
    ("l2", po::value<float>(&global.l2_lambda)->default_value(0.0), "l_2 lambda")
    ("data,d", po::value< string >()->default_value(""), "Example Set")
    ("daemon", "persistent daemon mode on port 26542")
    ("num_children", po::value<size_t>(&global.num_children)->default_value(10), "number of children for persistent daemon mode")
    ("pid_file", po::value< string >(), "Write pid file in persistent daemon mode")
    ("decay_learning_rate",    po::value<float>(&global.eta_decay_rate)->default_value(default_decay),
     "Set Decay factor for learning_rate between passes")
    ("input_feature_regularizer", po::value< string >(&global.per_feature_regularizer_input), "Per feature regularization input file")
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("readable_model", po::value< string >(), "Output human-readable final regressor")
    ("hash", po::value< string > (), "how to hash the features. Available options: strings, all")
    ("hessian_on", "use second derivative in line search")
    ("version","Version information")
    ("ignore", po::value< vector<unsigned char> >(), "ignore namespaces beginning with character <arg>")
    ("initial_weight", po::value<float>(&global.initial_weight)->default_value(0.), "Set all weights to an initial value of 1.")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_pass_length", po::value<size_t>(&global.pass_length)->default_value((size_t)-1), "initial number of examples per pass")
    ("initial_t", po::value<double>(&(global.sd->t))->default_value(1.), "initial t value")
    ("lda", po::value<size_t>(&global.lda), "Run lda with <int> topics")
    ("lda_alpha", po::value<float>(&global.lda_alpha)->default_value(0.1), "Prior on sparsity of per-document topic weights")
    ("lda_rho", po::value<float>(&global.lda_rho)->default_value(0.1), "Prior on sparsity of topic distributions")
    ("lda_D", po::value<float>(&global.lda_D)->default_value(10000.), "Number of documents")
    ("minibatch", po::value<size_t>(&global.minibatch)->default_value(1), "Minibatch size, for LDA")
    ("span_server", po::value<string>(&global.span_server)->default_value(""), "Location of server for setting up spanning tree")
    ("min_prediction", po::value<double>(&global.sd->min_label), "Smallest prediction to output")
    ("max_prediction", po::value<double>(&global.sd->max_label), "Largest prediction to output")
    ("mem", po::value<int>(&global.m)->default_value(15), "memory in bfgs")
    ("noconstant", "Don't add a constant feature")
    ("noop","do no learning")
    ("oaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> labels")
    ("output_feature_regularizer_binary", po::value< string >(&global.per_feature_regularizer_output), "Per feature regularization output file")
    ("output_feature_regularizer_text", po::value< string >(&global.per_feature_regularizer_text), "Per feature regularization output file, in text")
    ("port", po::value<size_t>(),"port to listen on")
    ("power_t", po::value<float>(&global.power_t)->default_value(0.5), "t power value")
    ("learning_rate,l", po::value<float>(&global.eta)->default_value(10),
     "Set Learning Rate")
    ("passes", po::value<size_t>(&global.numpasses)->default_value(1),
     "Number of Training Passes")
    ("termination", po::value<float>(&global.rel_threshold)->default_value(0.001),
     "Termination threshold")
    ("predictions,p", po::value< string >(), "File to output predictions to")
    ("quadratic,q", po::value< vector<string> > (),
     "Create and use quadratic features")
    ("quiet", "Don't output diagnostics")
    ("rank", po::value<size_t>(&global.rank)->default_value(0), "rank for matrix factorization.")
    ("random_weights", po::value<bool>(&global.random_weights), "make initial weights random")
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
    ("testonly,t", "Ignore label information and just test")
    ("loss_function", po::value<string>()->default_value("squared"), "Specify the loss function to be used, uses squared by default. Currently available ones are squared, classic, hinge, logistic and quantile.")
    ("quantile_tau", po::value<double>()->default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5")

    ("unique_id", po::value<size_t>(&global.unique_id)->default_value(0),"unique id used for cluster parallel jobs")
    ("total", po::value<size_t>(&global.total)->default_value(1),"total number of nodes used in cluster parallel job")    
    ("node", po::value<size_t>(&global.node)->default_value(0),"node number in cluster parallel job")    

    ("sort_features", "turn this on to disregard order in which features have been defined. This will lead to smaller cache sizes")
    ("ngram", po::value<size_t>(), "Generate N grams")
    ("skips", po::value<size_t>(), "Generate skips in N grams. This in conjunction with the ngram tag can be used to generate generalized n-skip-k-gram.");

  global.sd->queries = 0;
  global.sd->example_number = 0;
  global.sd->weighted_examples = 0.;
  global.sd->old_weighted_examples = 0.;
  global.sd->weighted_labels = 0.;
  global.sd->total_features = 0;
  global.sd->sum_loss = 0.0;
  global.sd->sum_loss_since_last_dump = 0.0;
  global.sd->dump_interval = exp(1.);
  global.sd->gravity = 0.;
  global.sd->contraction = 1.;
  global.sd->min_label = 0.;
  global.sd->max_label = 1.;
  global.lp = (label_parser*)malloc(sizeof(label_parser));
  *(global.lp) = simple_label;
  global.reg_mode = 0;
  global.local_example_number = 0;
  global.bfgs = false;
  global.hessian_on = false;
  global.sequence = false;
  global.stride = 1;
  global.num_bits = 18;
  global.default_bits = true;
  global.daemon = false;

  global.driver = drive_gd;
  global.k = 0;

  global.final_prediction_sink.begin = global.final_prediction_sink.end=global.final_prediction_sink.end_array = NULL;
  global.raw_prediction = -1;
  global.print = print_result;
  global.lda = 0;
  global.random_weights = false;
  global.per_feature_regularizer_input = "";
  global.per_feature_regularizer_output = "";
  global.per_feature_regularizer_text = "";
  global.ring_size = 1 << 8;
  global.nonormalize = false;
  global.binary_label = false;

  global.adaptive = false;
  global.add_constant = true;
  global.exact_adaptive_norm = false;
  global.audit = false;
  global.active = false;
  global.active_simulation =false;
  global.reg.weight_vectors = NULL;
  global.reg.regularizers = NULL;

  global.save_per_pass = false;

  po::positional_options_description p;
  // Be friendly: if -d was left out, treat positional param as data file
  p.add("data", -1);

  po::variables_map vm;

  po::store(po::command_line_parser(argc, argv).
	    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
	    options(desc).positional(p).run(), vm);
  po::notify(vm);

  global.sd->weighted_unlabeled_examples = global.sd->t;
  global.initial_t = global.sd->t;

  if (vm.count("help") || argc == 1) {
    /* upon direct query for help -- spit it out to stdout */
    cout << "\n" << desc << "\n";
    exit(0);
  }

  if (vm.count("quiet"))
    global.quiet = true;
  else
    global.quiet = false;

  if (vm.count("active_simulation"))
      global.active_simulation = true;

  if (vm.count("active_learning") && !global.active_simulation)
    global.active = true;

  if (vm.count("adaptive") || vm.count("exact_adaptive_norm")) {
      global.adaptive = true;
      if (vm.count("exact_adaptive_norm"))
	{
	  global.exact_adaptive_norm = true;
	  if (vm.count("nonormalize"))
	    cout << "Options don't make sense.  You can't use an exact norm and not normalize." << endl;
	}
      global.stride = 2;
  }
  
  void (*base_learner)(example*) = learn_gd;
  void (*base_finish)() = finish_gd;
  
  if (vm.count("bfgs") || vm.count("conjugate_gradient")) {
    global.driver = BFGS::drive_bfgs;
    base_learner = BFGS::learn;
    base_finish = BFGS::finish;
    BFGS::initializer();
    
    global.bfgs = true;
    global.stride = 4;
    if (vm.count("hessian_on") || global.m==0) {
      global.hessian_on = true;
    }
    if (!global.quiet) {
      if (global.m>0)
	cerr << "enabling BFGS based optimization ";
      else
	cerr << "enabling conjugate gradient optimization via BFGS ";
      if (global.hessian_on)
	cerr << "with curvature calculation" << endl;
      else
	cerr << "**without** curvature calculation" << endl;
    }
    if (global.numpasses < 2)
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
    global.ngram = vm["ngram"].as<size_t>();
    if(!vm.count("skip_gram")) cerr << "You have chosen to generate " << global.ngram << "-grams" << endl;
    if(vm.count("sort_features"))
      {
	cerr << "ngram is incompatible with sort_features.  " << endl;
	exit(1);
      }
  }
  if(vm.count("skips"))
    {
    global.skips = vm["skips"].as<size_t>();
    if(!vm.count("ngram"))
      {
	cout << "You can not skip unless ngram is > 1" << endl;
	exit(1);
      }
    cerr << "You have chosen to generate " << global.skips << "-skip-" << global.ngram << "-grams" << endl;
    if(global.skips > 4)
      {
      cout << "*********************************" << endl;
      cout << "Generating these features might take quite some time" << endl;
      cout << "*********************************" << endl;
      }
    }
  if (vm.count("bit_precision"))
    {
      global.default_bits = false;
      global.num_bits = vm["bit_precision"].as< size_t>();
      if (global.num_bits > 29)
	{
	  cout << "Only 29 or fewer bits allowed.  If this is a serious limit, speak up." << endl;
	  exit(1);
	}
    }
  
  if (vm.count("daemon") || vm.count("pid_file")) {
    global.daemon = true;

    // allow each child to process up to 1e5 connections
    global.numpasses = (size_t) 1e5;
  }

  string data_filename = vm["data"].as<string>();
  if (vm.count("compressed") || ends_with(data_filename, ".gz"))
    set_compressed(par);

  if(vm.count("sort_features"))
    par->sort_features = true;

  if (global.num_bits > 30) {
    cerr << "The system limits at 30 bits of precision!\n" << endl;
    exit(1);
  }

  if (vm.count("quadratic"))
    {
      global.pairs = vm["quadratic"].as< vector<string> >();
      if (!global.quiet)
	{
	  cerr << "creating quadratic features for pairs: ";
	  for (vector<string>::iterator i = global.pairs.begin(); i != global.pairs.end();i++) {
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
    global.ignore[i] = false;
  global.ignore_some = false;

  if (vm.count("ignore"))
    {
      vector<unsigned char> ignore = vm["ignore"].as< vector<unsigned char> >();
      for (vector<unsigned char>::iterator i = ignore.begin(); i != ignore.end();i++)
	{
	  global.ignore[*i] = true;
	  global.ignore_some = true;
	}
      if (!global.quiet)
	{
	  cerr << "ignoring namespaces beginning with: ";
	  for (vector<unsigned char>::iterator i = ignore.begin(); i != ignore.end();i++)
	    cerr << *i << " ";

	  cerr << endl;
	}
    }

  // matrix factorization enabled
  if (global.rank > 0) {
    // store linear + 2*rank weights per index, round up to power of two
    float temp = ceilf(logf((float)(global.rank*2+1)) / logf (2.f));
    global.stride = 1 << (int) temp;
    global.random_weights = true;
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
    global.add_constant = false;

  if (vm.count("nonormalize"))
    global.nonormalize = true;

  if (vm.count("lda"))
    {
      global.driver = drive_lda;
      par->sort_features = true;
      float temp = ceilf(logf((float)(global.lda*2+1)) / logf (2.f));
      global.stride = 1 << (int) temp;
      global.random_weights = true;
      global.add_constant = false;
    }

  if (vm.count("lda") && global.eta > 1.)
    {
      cerr << "your learning rate is too high, setting it to 1" << endl;
      global.eta = min(global.eta,1.f);
    }
  if (!vm.count("lda")) 
    global.eta *= pow(global.sd->t, (double)global.power_t);

  if (vm.count("minibatch")) {
    size_t minibatch2 = next_pow2(global.minibatch);
    global.ring_size = global.ring_size > minibatch2 ? global.ring_size : minibatch2;
  }

  if (vm.count("sequence_max_length")) {
    size_t maxlen = vm["sequence_max_length"].as<size_t>();
    global.ring_size = (global.ring_size > maxlen) ? global.ring_size : maxlen;
  }

  parse_regressor_args(vm, global.reg, global.final_regressor_name, global.quiet);
  parse_source_args(vm,par,global.quiet,global.numpasses);
  if (vm.count("readable_model"))
    global.text_regressor_name = vm["readable_model"].as<string>();
  
  if (vm.count("active_c0"))
    global.active_c0 = vm["active_c0"].as<float>();
  
  if (vm.count("save_per_pass"))
    global.save_per_pass = true;

  if (vm.count("min_prediction"))
    global.sd->min_label = vm["min_prediction"].as<double>();
  if (vm.count("max_prediction"))
    global.sd->max_label = vm["max_prediction"].as<double>();
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
    global.driver = drive_noop;
  
  if (global.rank != 0) {
    global.driver = drive_gd_mf;
    loss_function = "classic";
    cerr << "Forcing classic squared loss for matrix factorization" << endl;
  }

  global.loss = getLossFunction(loss_function, loss_parameter);

  if (global.eta_decay_rate != default_decay && global.numpasses == 1)
    cerr << "Warning: decay_learning_rate has no effect when there is only one pass" << endl;

  if (pow((double)global.eta_decay_rate, (double)global.numpasses) < 0.0001 )
    cerr << "Warning: the learning rate for the last pass is multiplied by: " << pow((double)global.eta_decay_rate, (double)global.numpasses)
	 << " adjust --decay_learning_rate larger to avoid this." << endl;

  if (!global.quiet)
    {
      cerr << "Num weight bits = " << global.num_bits << endl;
      cerr << "learning rate = " << global.eta << endl;
      cerr << "initial_t = " << global.sd->t << endl;
      cerr << "power_t = " << global.power_t << endl;
      if (global.numpasses > 1)
	cerr << "decay_learning_rate = " << global.eta_decay_rate << endl;
      if (global.rank > 0)
	cerr << "rank = " << global.rank << endl;
    }

  if (vm.count("predictions")) {
    if (!global.quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      {
	push(global.final_prediction_sink, (size_t) 1);//stdout
      }
    else
      {
	const char* fstr = (vm["predictions"].as< string >().c_str());
	int f = fileno(fopen(fstr,"w"));
	if (f < 0)
	  cerr << "Error opening the predictions file: " << fstr << endl;
	push(global.final_prediction_sink, (size_t) f);
      }
  }

  if (vm.count("raw_predictions")) {
    if (!global.quiet)
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      global.raw_prediction = 1;//stdout
    else
      global.raw_prediction = fileno(fopen(vm["raw_predictions"].as< string >().c_str(), "w"));
  }

  if (vm.count("audit"))
    global.audit = true;

  if (vm.count("sendto"))
    {
      global.driver = drive_send;
      parse_send_args(vm, global.pairs);
    }

  if (vm.count("testonly"))
    {
      if (!global.quiet)
	cerr << "only testing" << endl;
      global.training = false;
      if (global.lda > 0)
        global.eta = 0;
    }
  else
    global.training = true;

  if (global.l1_lambda < 0.) {
    cerr << "l1_lambda should be nonnegative: resetting from " << global.l1_lambda << " to 0" << endl;
    global.l1_lambda = 0.;
  }
  if (global.l2_lambda < 0.) {
    cerr << "l2_lambda should be nonnegative: resetting from " << global.l2_lambda << " to 0" << endl;
    global.l2_lambda = 0.;
  }
  global.reg_mode += (global.l1_lambda > 0.) ? 1 : 0;
  global.reg_mode += (global.l2_lambda > 0.) ? 2 : 0;
  if (!global.quiet)
    {
      if (global.reg_mode %2)
	cerr << "using l1 regularization" << endl;
      if (global.reg_mode > 1)
	cerr << "using l2 regularization" << endl;
    }

  void (*mc_learner)(example*) = NULL;
  void (*mc_finish)() = NULL;

  if(vm.count("oaa"))
    {
      OAA::parse_flags(vm["oaa"].as<size_t>(), base_learner, base_finish);
      mc_learner = OAA::learn;
      mc_finish = OAA::finish;
    }

  void (*cs_learner)(example*) = CSOAA::learn;
  void (*cs_finish)() = CSOAA::finish;

  if(vm.count("wap"))
    {
      WAP::parse_flags(vm["wap"].as<size_t>(), base_learner, base_finish);
      cs_learner = WAP::learn;
      cs_finish = WAP::finish;
    }

  if(vm.count("csoaa_ldf")) {
    CSOAA_LDF::parse_flags(0, base_learner, base_finish);
    cs_learner = CSOAA_LDF::learn;
    cs_finish  = CSOAA_LDF::finish;
  }

  if(vm.count("wap_ldf")) {
    WAP_LDF::parse_flags(0, base_learner, base_finish);
    cs_learner = WAP_LDF::learn;
    cs_finish  = WAP_LDF::finish;
  }


  if(vm.count("csoaa"))
    CSOAA::parse_flags(vm["csoaa"].as<size_t>(), base_learner, base_finish);

  if (vm.count("sequence")) {
    if (vm.count("wap")) 
      ;
    else
      CSOAA::parse_flags(vm["sequence"].as<size_t>(), base_learner, base_finish);  // default to CSOAA unless wap is specified

    parse_sequence_args(vm, cs_learner, cs_finish);
    global.driver = drive_sequence;
    global.sequence = true;
  }

  return vm;
}

