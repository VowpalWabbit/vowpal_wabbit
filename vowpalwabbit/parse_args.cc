/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdio.h>
#include <float.h>

#include "cache.h"
#include "io_buf.h"
#include "parse_regressor.h"
#include "parser.h"
#include "parse_args.h"
#include "sender.h"
#include "network.h"
#include "global_data.h"
#include "nn.h"
#include "oaa.h"
#include "ect.h"
#include "csoaa.h"
#include "wap.h"
#include "cb.h"
#include "searn.h"
#include "bfgs.h"
#include "lda_core.h"
#include "noop.h"
#include "gd_mf.h"
#include "vw.h"
#include "rand48.h"
#include "parse_args.h"
#include "binary.h"
#include "autolink.h"

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

bool valid_ns(char c)
{
    if (c=='|'||c==':')
        return false;
    return true;
}

vw* parse_args(int argc, char *argv[])
{
  po::options_description desc("VW options");
  
  vw* all = new vw();

  size_t random_seed = 0;
  all->program_name = argv[0];
  // Declare the supported options.
  desc.add_options()
    ("help,h","Look here: http://hunch.net/~vw/ and click on Tutorial.")
    ("active_learning", "active learning mode")
    ("active_simulation", "active learning simulation mode")
    ("active_mellowness", po::value<float>(&(all->active_c0)), "active learning mellowness parameter c_0. Default 8")
    ("binary", "report loss as binary classification on -1,1")
    ("autolink", po::value<size_t>(), "create link function with polynomial d")
    ("sgd", "use regular stochastic gradient descent update.")
    ("adaptive", "use adaptive, individual learning rates.")
    ("invariant", "use safe/importance aware updates.")
    ("normalized", "use per feature normalized updates")
    ("exact_adaptive_norm", "use current default invariant normalized adaptive update rule")
    ("audit,a", "print weights of features")
    ("bit_precision,b", po::value<size_t>(),
     "number of bits in the feature table")
    ("bfgs", "use bfgs optimization")
    ("cache,c", "Use a cache.  The default is <data>.cache")
    ("cache_file", po::value< vector<string> >(), "The location(s) of cache_file.")
    ("compressed", "use gzip format whenever possible. If a cache file is being created, this option creates a compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection.")
    ("no_stdin", "do not default to reading from stdin")
    ("conjugate_gradient", "use conjugate gradient based optimization")
    ("csoaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> costs")
    ("wap", po::value<size_t>(), "Use weighted all-pairs multiclass learning with <k> costs")
    ("csoaa_ldf", po::value<string>(), "Use one-against-all multiclass learning with label dependent features.  Specify singleline or multiline.")
    ("wap_ldf", po::value<string>(), "Use weighted all-pairs multiclass learning with label dependent features.  Specify singleline or multiline.")
    ("cb", po::value<size_t>(), "Use contextual bandit learning with <k> costs")
    ("l1", po::value<float>(&(all->l1_lambda)), "l_1 lambda")
    ("l2", po::value<float>(&(all->l2_lambda)), "l_2 lambda")
    ("data,d", po::value< string >(), "Example Set")
    ("daemon", "persistent daemon mode on port 26542")
    ("num_children", po::value<size_t>(&(all->num_children)), "number of children for persistent daemon mode")
    ("pid_file", po::value< string >(), "Write pid file in persistent daemon mode")
    ("decay_learning_rate",    po::value<float>(&(all->eta_decay_rate)),
     "Set Decay factor for learning_rate between passes")
    ("input_feature_regularizer", po::value< string >(&(all->per_feature_regularizer_input)), "Per feature regularization input file")
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("readable_model", po::value< string >(), "Output human-readable final regressor")
    ("hash", po::value< string > (), "how to hash the features. Available options: strings, all")
    ("hessian_on", "use second derivative in line search")
    ("version","Version information")
    ("ignore", po::value< vector<unsigned char> >(), "ignore namespaces beginning with character <arg>")
    ("keep", po::value< vector<unsigned char> >(), "keep namespaces beginning with character <arg>")
    ("kill_cache,k", "do not reuse existing cache: create a new one always")
    ("initial_weight", po::value<float>(&(all->initial_weight)), "Set all weights to an initial value of 1.")
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_pass_length", po::value<size_t>(&(all->pass_length)), "initial number of examples per pass")
    ("initial_t", po::value<double>(&((all->sd->t))), "initial t value")
    ("lda", po::value<size_t>(&(all->lda)), "Run lda with <int> topics")
    ("span_server", po::value<string>(&(all->span_server)), "Location of server for setting up spanning tree")
    ("min_prediction", po::value<float>(&(all->sd->min_label)), "Smallest prediction to output")
    ("max_prediction", po::value<float>(&(all->sd->max_label)), "Largest prediction to output")
    ("mem", po::value<int>(&(all->m)), "memory in bfgs")
    ("nn", po::value<size_t>(), "Use sigmoidal feedforward network with <k> hidden units")
    ("noconstant", "Don't add a constant feature")
    ("noop","do no learning")
    ("oaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> labels")
    ("ect", po::value<size_t>(), "Use error correcting tournament with <k> labels")
    ("output_feature_regularizer_binary", po::value< string >(&(all->per_feature_regularizer_output)), "Per feature regularization output file")
    ("output_feature_regularizer_text", po::value< string >(&(all->per_feature_regularizer_text)), "Per feature regularization output file, in text")
    ("port", po::value<size_t>(),"port to listen on")
    ("power_t", po::value<float>(&(all->power_t)), "t power value")
    ("learning_rate,l", po::value<float>(&(all->eta)), "Set Learning Rate")
    ("passes", po::value<size_t>(&(all->numpasses)),"Number of Training Passes")
    ("termination", po::value<float>(&(all->rel_threshold)),"Termination threshold")
    ("predictions,p", po::value< string >(), "File to output predictions to")
    ("quadratic,q", po::value< vector<string> > (),
     "Create and use quadratic features")
    ("q:", po::value< string >(), ": corresponds to a wildcard for all printable characters")
    ("cubic", po::value< vector<string> > (),
     "Create and use cubic features")
    ("quiet", "Don't output diagnostics")
    ("rank", po::value<uint32_t>(&(all->rank)), "rank for matrix factorization.")
    ("random_weights", po::value<bool>(&(all->random_weights)), "make initial weights random")
    ("random_seed", po::value<size_t>(&random_seed), "seed random number generator")
    ("raw_predictions,r", po::value< string >(),
     "File to output unnormalized predictions to")
    ("ring_size", po::value<size_t>(&(all->p->ring_size)), "size of example ring")
	("examples", po::value<size_t>(&(all->max_examples)), "number of examples to parse")
    ("save_per_pass", "Save the model after every pass over data")
    ("save_resume", "save extra state so learning can be resumed later with new data")
    ("sendto", po::value< vector<string> >(), "send examples to <host>")
    ("searn", po::value<size_t>(), "use searn, argument=maximum action id")
    ("searnimp", po::value<size_t>(), "use searn, argument=maximum action id or 0 for LDF")
    ("testonly,t", "Ignore label information and just test")
    ("loss_function", po::value<string>()->default_value("squared"), "Specify the loss function to be used, uses squared by default. Currently available ones are squared, classic, hinge, logistic and quantile.")
    ("quantile_tau", po::value<float>()->default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5")

    ("unique_id", po::value<size_t>(&(all->unique_id)),"unique id used for cluster parallel jobs")
    ("total", po::value<size_t>(&(all->total)),"total number of nodes used in cluster parallel job")    
    ("node", po::value<size_t>(&(all->node)),"node number in cluster parallel job")    

    ("sort_features", "turn this on to disregard order in which features have been defined. This will lead to smaller cache sizes")
    ("ngram", po::value< vector<string> >(), "Generate N grams")
    ("skips", po::value< vector<string> >(), "Generate skips in N grams. This in conjunction with the ngram tag can be used to generate generalized n-skip-k-gram.");

  //po::positional_options_description p;
  // Be friendly: if -d was left out, treat positional param as data file
  //p.add("data", -1);

  po::variables_map vm = po::variables_map();
  po::variables_map vm_file = po::variables_map(); //separate variable map for storing flags in regressor file

  po::parsed_options parsed = po::command_line_parser(argc, argv).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(desc).allow_unregistered().run();   // got rid of ".positional(p)" because it doesn't work well with unrecognized options
  vector<string> to_pass_further = po::collect_unrecognized(parsed.options, po::include_positional);
  string last_unrec_arg =
    (to_pass_further.size() > 0)
    ? string(to_pass_further[to_pass_further.size()-1])  // we want to write this down in case it's a data argument ala the positional option we got rid of
    : "";

  po::store(parsed, vm);
  po::notify(vm);

  all->data_filename = "";

  all->searn = false;
  all->searnstr = NULL;

  all->sd->weighted_unlabeled_examples = all->sd->t;
  all->initial_t = (float)all->sd->t;

  if(all->initial_t > 0)
  {
    all->normalized_sum_norm_x = all->initial_t;//for the normalized update: if initial_t is bigger than 1 we interpret this as if we had seen (all->initial_t) previous fake datapoints all with norm 1
  }

  if (vm.count("help") || argc == 1) {
    /* upon direct query for help -- spit it out to stdout */
    cout << "\n" << desc << "\n";
    exit(0);
  }

  if (vm.count("quiet"))
    all->quiet = true;
  else
    all->quiet = false;

  msrand48(random_seed);

  if (vm.count("active_simulation"))
      all->active_simulation = true;

  if (vm.count("active_learning") && !all->active_simulation)
    all->active = true;

  if (vm.count("no_stdin"))
    all->stdin_off = true;

  if (vm.count("testonly") || all->eta == 0.)
    {
      if (!all->quiet)
	cerr << "only testing" << endl;
      all->training = false;
      if (all->lda > 0)
        all->eta = 0;
    }
  else
    all->training = true;

  if ( (vm.count("total") || vm.count("node") || vm.count("unique_id")) && !(vm.count("total") && vm.count("node") && vm.count("unique_id")) )
    {
      cout << "you must specificy unique_id, total, and node if you specify any" << endl;
      throw exception();
    }

  all->reg.stride = 4; //use stride of 4 for default invariant normalized adaptive updates
  //if we are doing matrix factorization, or user specified anything in sgd,adaptive,invariant,normalized, we turn off default update rules and use whatever user specified
  if( all->rank > 0 || !all->training || ( ( vm.count("sgd") || vm.count("adaptive") || vm.count("invariant") || vm.count("normalized") ) && !vm.count("exact_adaptive_norm")) )
  {
    all->adaptive = all->training && (vm.count("adaptive") && all->rank == 0);
    all->invariant_updates = all->training && vm.count("invariant");
    all->normalized_updates = all->training && (vm.count("normalized") && all->rank == 0);

    all->reg.stride = 1;

    if( all->adaptive ) all->reg.stride *= 2;
    else all->normalized_idx = 1; //store per feature norm at 1 index offset from weight value instead of 2

    if( all->normalized_updates ) all->reg.stride *= 2;

    if(!vm.count("learning_rate") && !vm.count("l") && !(all->adaptive && all->normalized_updates))
      all->eta = 10; //default learning rate to 10 for non default update rule

    //if not using normalized or adaptive, default initial_t to 1 instead of 0
    if(!all->adaptive && !all->normalized_updates && !vm.count("initial_t")) {
      all->sd->t = 1.f;
      all->sd->weighted_unlabeled_examples = 1.f;
      all->initial_t = 1.f;
    }
  }

  if (vm.count("bfgs") || vm.count("conjugate_gradient")) 
    BFGS::setup(*all, to_pass_further, vm, vm_file);

  if (vm.count("version") || argc == 1) {
    /* upon direct query for version -- spit it out to stdout */
    cout << version.to_string() << "\n";
    exit(0);
  }


  if(vm.count("ngram")){
    if(vm.count("sort_features"))
      {
	cerr << "ngram is incompatible with sort_features.  " << endl;
	throw exception();
      }

    all->ngram_strings = vm["ngram"].as< vector<string> >();
    compile_gram(all->ngram_strings, all->ngram, (char*)"grams", all->quiet);
  }

  if(vm.count("skips"))
    {
      if(!vm.count("ngram"))
	{
	  cout << "You can not skip unless ngram is > 1" << endl;
	  throw exception();
	}
      
      all->skip_strings = vm["skips"].as<vector<string> >();
      compile_gram(all->skip_strings, all->skips, (char*)"skips", all->quiet);
    }
  if (vm.count("bit_precision"))
    {
      all->default_bits = false;
      all->num_bits = (uint32_t)vm["bit_precision"].as< size_t>();
      if (all->num_bits > min(32, sizeof(size_t)*8 - 3))
	{
	  cout << "Only " << min(32, sizeof(size_t)*8 - 3) << " or fewer bits allowed.  If this is a serious limit, speak up." << endl;
	  throw exception();
	}
    }
  
  if (vm.count("daemon") || vm.count("pid_file") || (vm.count("port") && !all->active) ) {
    all->daemon = true;

    // allow each child to process up to 1e5 connections
    all->numpasses = (size_t) 1e5;
  }

  if (vm.count("compressed"))
      set_compressed(all->p);

  if (vm.count("data")) {
    all->data_filename = vm["data"].as<string>();
    if (ends_with(all->data_filename, ".gz"))
      set_compressed(all->p);
  } else {
    all->data_filename = "";
  }

  if(vm.count("sort_features"))
    all->p->sort_features = true;

  

  if (vm.count("quadratic"))
    {
      all->pairs = vm["quadratic"].as< vector<string> >();
      vector<string> newpairs;
      //string tmp;       
      char printable_start = '!';
      char printable_end = '~';
      int valid_ns_size = printable_end - printable_start - 1; //will skip two characters

      if(!all->quiet)
        cerr<<"creating quadratic features for pairs: ";   
    
      for (vector<string>::iterator i = all->pairs.begin(); i != all->pairs.end();i++){
        if(!all->quiet){
          cerr << *i << " ";
          if (i->length() > 2)
            cerr << endl << "warning, ignoring characters after the 2nd.\n";
          if (i->length() < 2) {
            cerr << endl << "error, quadratic features must involve two sets.\n";
            throw exception();
          }
        }
        //-q x:
        if((*i)[0]!=':'&&(*i)[1]==':'){
          newpairs.reserve(newpairs.size() + valid_ns_size);
          for (char j=printable_start; j<=printable_end; j++){
            if(valid_ns(j))
              newpairs.push_back(string(1,(*i)[0])+j);
          }
        }
        //-q :x
        else if((*i)[0]==':'&&(*i)[1]!=':'){
          newpairs.reserve(newpairs.size() + valid_ns_size);
          for (char j=printable_start; j<=printable_end; j++){
            if(valid_ns(j))
              newpairs.push_back(string(&j)+(*i)[1]);
          }
        }
        //-q ::
        else if((*i)[0]==':'&&(*i)[1]==':'){
          newpairs.reserve(newpairs.size() + valid_ns_size*valid_ns_size);
          for (char j=printable_start; j<=printable_end; j++){
            if(valid_ns(j)){
              for (char k=printable_start; k<=printable_end; k++){
                if(valid_ns(k))
                  newpairs.push_back(string(&j)+k);
              }
            }
          }
        }
        else{
          newpairs.push_back(string(*i));
        }    
      }
      newpairs.swap(all->pairs);
      if(!all->quiet)
        cerr<<endl;
    }

  if (vm.count("cubic"))
    {
      all->triples = vm["cubic"].as< vector<string> >();
      if (!all->quiet)
	{
	  cerr << "creating cubic features for triples: ";
	  for (vector<string>::iterator i = all->triples.begin(); i != all->triples.end();i++) {
	    cerr << *i << " ";
	    if (i->length() > 3)
	      cerr << endl << "warning, ignoring characters after the 3rd.\n";
	    if (i->length() < 3) {
	      cerr << endl << "error, cubic features must involve three sets.\n";
	      throw exception();
	    }
	  }
	  cerr << endl;
	}
    }

  io_buf io_temp;
  parse_regressor_args(*all, vm, io_temp);

  //parse flags from regressor file
  all->options_from_file_argv = VW::get_argv_from_string(all->options_from_file,all->options_from_file_argc);

  po::parsed_options parsed_file = po::command_line_parser(all->options_from_file_argc, all->options_from_file_argv).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(desc).allow_unregistered().run();

  po::store(parsed_file, vm_file);
  po::notify(vm_file);

  for (size_t i = 0; i < 256; i++)
    all->ignore[i] = false;
  all->ignore_some = false;

  if (vm.count("ignore"))
    {
      all->ignore_some = true;

      vector<unsigned char> ignore = vm["ignore"].as< vector<unsigned char> >();
      for (vector<unsigned char>::iterator i = ignore.begin(); i != ignore.end();i++)
	{
	  all->ignore[*i] = true;
	}
      if (!all->quiet)
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
        all->ignore[i] = true;

      all->ignore_some = true;

      vector<unsigned char> keep = vm["keep"].as< vector<unsigned char> >();
      for (vector<unsigned char>::iterator i = keep.begin(); i != keep.end();i++)
	{
	  all->ignore[*i] = false;
	}
      if (!all->quiet)
	{
	  cerr << "using namespaces beginning with: ";
	  for (vector<unsigned char>::iterator i = keep.begin(); i != keep.end();i++)
	    cerr << *i << " ";

	  cerr << endl;
	}
    }

  // matrix factorization enabled
  if (all->rank > 0) {
    // store linear + 2*rank weights per index, round up to power of two
    float temp = ceilf(logf((float)(all->rank*2+1)) / logf (2.f));
    all->reg.stride = 1 << (int) temp;
    all->random_weights = true;

    if ( vm.count("adaptive") )
      {
	cerr << "adaptive is not implemented for matrix factorization" << endl;
        throw exception();
      }
    if ( vm.count("normalized") )
      {
	cerr << "normalized is not implemented for matrix factorization" << endl;
        throw exception();
      }
    if ( vm.count("exact_adaptive_norm") )
      {
	cerr << "normalized adaptive updates is not implemented for matrix factorization" << endl;
        throw exception();
      }
    if (vm.count("bfgs") || vm.count("conjugate_gradient"))
      {
	cerr << "bfgs is not implemented for matrix factorization" << endl;
	throw exception();
      }	

    //default initial_t to 1 instead of 0
    if(!vm.count("initial_t")) {
      all->sd->t = 1.f;
      all->sd->weighted_unlabeled_examples = 1.f;
      all->initial_t = 1.f;
    }
  }

  if (vm.count("noconstant"))
    all->add_constant = false;

  //if (vm.count("nonormalize"))
  //  all->nonormalize = true;

  if (vm.count("lda")) 
    all->l = LDA::setup(*all, to_pass_further, vm);

  if (!vm.count("lda") && !all->adaptive && !all->normalized_updates) 
    all->eta *= powf((float)(all->sd->t), all->power_t);
  
  if (vm.count("readable_model"))
    all->text_regressor_name = vm["readable_model"].as<string>();
  
  if (vm.count("save_per_pass"))
    all->save_per_pass = true;

  if (vm.count("save_resume"))
    all->save_resume = true;

  if (vm.count("min_prediction"))
    all->sd->min_label = vm["min_prediction"].as<float>();
  if (vm.count("max_prediction"))
    all->sd->max_label = vm["max_prediction"].as<float>();
  if (vm.count("min_prediction") || vm.count("max_prediction") || vm.count("testonly"))
    all->set_minmax = noop_mm;

  string loss_function;
  if(vm.count("loss_function"))
    loss_function = vm["loss_function"].as<string>();
  else
    loss_function = "squaredloss";
  float loss_parameter = 0.0;
  if(vm.count("quantile_tau"))
    loss_parameter = vm["quantile_tau"].as<float>();

  all->is_noop = false;
  if (vm.count("noop")) 
    all->l = NOOP::setup(*all);
  
  if (all->rank != 0) 
    all->l = GDMF::setup(*all);

  all->loss = getLossFunction(all, loss_function, (float)loss_parameter);

  if (pow((double)all->eta_decay_rate, (double)all->numpasses) < 0.0001 )
    cerr << "Warning: the learning rate for the last pass is multiplied by: " << pow((double)all->eta_decay_rate, (double)all->numpasses)
	 << " adjust --decay_learning_rate larger to avoid this." << endl;

  if (!all->quiet)
    {
      cerr << "Num weight bits = " << all->num_bits << endl;
      cerr << "learning rate = " << all->eta << endl;
      cerr << "initial_t = " << all->sd->t << endl;
      cerr << "power_t = " << all->power_t << endl;
      if (all->numpasses > 1)
	cerr << "decay_learning_rate = " << all->eta_decay_rate << endl;
      if (all->rank > 0)
	cerr << "rank = " << all->rank << endl;
    }

  if (vm.count("predictions")) {
    if (!all->quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      {
	all->final_prediction_sink.push_back((size_t) 1);//stdout
      }
    else
      {
	const char* fstr = (vm["predictions"].as< string >().c_str());
	int f;
#ifdef _WIN32
	_sopen_s(&f, fstr, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
	f = open(fstr, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
	if (f < 0)
	  cerr << "Error opening the predictions file: " << fstr << endl;
	all->final_prediction_sink.push_back((size_t) f);
      }
  }

  if (vm.count("raw_predictions")) {
    if (!all->quiet)
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      all->raw_prediction = 1;//stdout
    else
	{
	  const char* t = vm["raw_predictions"].as< string >().c_str();
	  int f;
#ifdef _WIN32
	  _sopen_s(&f, t, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
	  f = open(t, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
	  all->raw_prediction = f;
	}
  }

  if (vm.count("audit"))
    all->audit = true;

  if (vm.count("sendto"))
    all->l = SENDER::setup(*all, vm, all->pairs);

  // load rest of regressor
  all->l.save_load(io_temp, true, false);
  io_temp.close_file();

  if (all->l1_lambda < 0.) {
    cerr << "l1_lambda should be nonnegative: resetting from " << all->l1_lambda << " to 0" << endl;
    all->l1_lambda = 0.;
  }
  if (all->l2_lambda < 0.) {
    cerr << "l2_lambda should be nonnegative: resetting from " << all->l2_lambda << " to 0" << endl;
    all->l2_lambda = 0.;
  }
  all->reg_mode += (all->l1_lambda > 0.) ? 1 : 0;
  all->reg_mode += (all->l2_lambda > 0.) ? 2 : 0;
  if (!all->quiet)
    {
      if (all->reg_mode %2)
	cerr << "using l1 regularization = " << all->l1_lambda << endl;
      if (all->reg_mode > 1)
	cerr << "using l2 regularization = " << all->l2_lambda << endl;
    }

  bool got_mc = false;
  bool got_cs = false;
  bool got_cb = false;

  if(vm.count("nn") || vm_file.count("nn") ) 
    all->l = NN::setup(*all, to_pass_further, vm, vm_file);

  if(vm.count("autolink") || vm_file.count("autolinnk") ) 
    all->l = ALINK::setup(*all, to_pass_further, vm, vm_file);
  
  if (vm.count("binary") || vm_file.count("binary"))
    all->l = BINARY::setup(*all, to_pass_further, vm, vm_file);

  if(vm.count("oaa") || vm_file.count("oaa") ) {
    if (got_mc) { cerr << "error: cannot specify multiple MC learners" << endl; throw exception(); }

    all->l = OAA::setup(*all, to_pass_further, vm, vm_file);
    got_mc = true;
  }
  
  if (vm.count("ect") || vm_file.count("ect") ) {
    if (got_mc) { cerr << "error: cannot specify multiple MC learners" << endl; throw exception(); }

    all->l = ECT::setup(*all, to_pass_further, vm, vm_file);
    got_mc = true;
  }

  if(vm.count("csoaa") || vm_file.count("csoaa") ) {
    if (got_cs) { cerr << "error: cannot specify multiple CS learners" << endl; throw exception(); }
    
    all->l = CSOAA::setup(*all, to_pass_further, vm, vm_file);
    got_cs = true;
  }

  if(vm.count("wap") || vm_file.count("wap") ) {
    if (got_cs) { cerr << "error: cannot specify multiple CS learners" << endl; throw exception(); }
    
    all->l = WAP::setup(*all, to_pass_further, vm, vm_file);
    got_cs = true;
  }

  if(vm.count("csoaa_ldf") || vm_file.count("csoaa_ldf")) {
    if (got_cs) { cerr << "error: cannot specify multiple CS learners" << endl; throw exception(); }

    all->l = CSOAA_AND_WAP_LDF::setup(*all, to_pass_further, vm, vm_file);
    got_cs = true;
  }

  if(vm.count("wap_ldf") || vm_file.count("wap_ldf") ) {
    if (got_cs) { cerr << "error: cannot specify multiple CS learners" << endl; throw exception(); }

    all->l = CSOAA_AND_WAP_LDF::setup(*all, to_pass_further, vm, vm_file);
    got_cs = true;
  }

  if( vm.count("cb") || vm_file.count("cb") )
  {
    if(!got_cs) {
      if( vm_file.count("cb") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm_file["cb"]));
      else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["cb"]));

      all->l = CSOAA::setup(*all, to_pass_further, vm, vm_file);  // default to CSOAA unless wap is specified
      got_cs = true;
    }

    all->l = CB::setup(*all, to_pass_further, vm, vm_file);
    got_cb = true;
  }

  if (vm.count("searn") || vm_file.count("searn") ) { 
    if (vm.count("searnimp") || vm_file.count("searnimp")) {
      cerr << "fail: cannot have both --searn and --searnimp" << endl;
      throw exception();
    }
    if (!got_cs && !got_cb) {
      if( vm_file.count("searn") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm_file["searn"]));
      else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["searn"]));
      
      all->l = CSOAA::setup(*all, to_pass_further, vm, vm_file);  // default to CSOAA unless others have been specified
      got_cs = true;
    }
    all->l = Searn::setup(*all, to_pass_further, vm, vm_file);
  }

  if (vm.count("searnimp") || vm_file.count("searnimp") ) { 
    if (!got_cs && !got_cb) {
      if( vm_file.count("searnimp") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm_file["searnimp"]));
      else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["searnimp"]));
      
      all->l = CSOAA::setup(*all, to_pass_further, vm, vm_file);  // default to CSOAA unless others have been specified
      got_cs = true;
    }
    all->searnstr = (ImperativeSearn::searn*)calloc(1, sizeof(ImperativeSearn::searn));
    all->l = ImperativeSearn::setup(*all, to_pass_further, vm, vm_file);
  }

  if (got_cb && got_mc) {
    cerr << "error: doesn't make sense to do both MC learning and CB learning" << endl;
    throw exception();
  }

  if (to_pass_further.size() > 0) {
    bool is_actually_okay = false;

    // special case to try to emulate the missing -d
    if ((to_pass_further.size() == 1) &&
        (to_pass_further[to_pass_further.size()-1] == last_unrec_arg)) {

      int f = io_buf().open_file(last_unrec_arg.c_str(), all->stdin_off, io_buf::READ);
      if (f != -1) {
#ifdef _WIN32
		 _close(f);
#else
		  close(f);
#endif
        //cerr << "warning: final argument '" << last_unrec_arg << "' assumed to be input file; in the future, please use -d" << endl;
        all->data_filename = last_unrec_arg;
        if (ends_with(last_unrec_arg, ".gz"))
          set_compressed(all->p);
        is_actually_okay = true;
      }
    }

    if (!is_actually_okay) {
      cerr << "unrecognized options:";
      for (size_t i=0; i<to_pass_further.size(); i++)
        cerr << " " << to_pass_further[i];
      cerr << endl;
      throw exception();
    }
  }

  parse_source_args(*all, vm, all->quiet,all->numpasses);

  // force stride * weights_per_problem to be divisible by 2 to avoid 32-bit overflow
  uint32_t i = 0;
  while (all->reg.stride * all->weights_per_problem  > (uint32_t)(1 << i))
    i++;
  all->weights_per_problem = (1 << i) / all->reg.stride;

  return all;
}

namespace VW {
  void cmd_string_replace_value( string& cmd, string flag_to_replace, string new_value )
  {
    flag_to_replace.append(" "); //add a space to make sure we obtain the right flag in case 2 flags start with the same set of characters
    size_t pos = cmd.find(flag_to_replace);
    if( pos == string::npos ) {
      //flag currently not present in command string, so just append it to command string
      cmd.append(" ");
      cmd.append(flag_to_replace);
      cmd.append(new_value);
    }
    else {
      //flag is present, need to replace old value with new value
      
      //compute position after flag_to_replace
      pos += flag_to_replace.size();

      //now pos is position where value starts
      //find position of next space
      size_t pos_after_value = cmd.find(" ",pos);
      if(pos_after_value == string::npos) {
        //we reach the end of the string, so replace the all characters after pos by new_value
        cmd.replace(pos,cmd.size()-pos,new_value);
      }
      else {
        //replace characters between pos and pos_after_value by new_value
        cmd.replace(pos,pos_after_value-pos,new_value);
      }
    }
  }

  char** get_argv_from_string(string s, int& argc)
  {
    char* c = (char*)calloc(s.length()+3, sizeof(char));
    c[0] = 'b';
    c[1] = ' ';
    strcpy(c+2, s.c_str());
    substring ss = {c, c+s.length()+2};
    v_array<substring> foo;
    foo.end_array = foo.begin = foo.end = NULL;
    tokenize(' ', ss, foo);
    
    char** argv = (char**)calloc(foo.size(), sizeof(char*));
    for (size_t i = 0; i < foo.size(); i++)
      {
	*(foo[i].end) = '\0';
	argv[i] = (char*)calloc(foo[i].end-foo[i].begin+1, sizeof(char));
        sprintf(argv[i],"%s",foo[i].begin);
      }

    argc = (int)foo.size();
    free(c);
    foo.delete_v();
    return argv;
  }
 
  vw* initialize(string s)
  {
    int argc = 0;
    s += " --no_stdin";
    char** argv = get_argv_from_string(s,argc);
    
    vw* all = parse_args(argc, argv);
    
    initialize_examples(*all);

    for(int i = 0; i < argc; i++)
      free(argv[i]);
    free (argv);

    return all;
  }

  void finish(vw& all)
  {
    finalize_regressor(all, all.final_regressor_name);
    all.l.finish();
    if (all.reg.weight_vector != NULL)
      free(all.reg.weight_vector);
    if (all.searnstr != NULL) free(all.searnstr);
    free_parser(all);
    finalize_source(all.p);
    free(all.p->lp);
    all.p->parse_name.erase();
    all.p->parse_name.delete_v();
    free(all.p);
    free(all.sd);
    for (int i = 0; i < all.options_from_file_argc; i++)
      free(all.options_from_file_argv[i]);
    free(all.options_from_file_argv);
    for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
      if (all.final_prediction_sink[i] != 1)
#ifdef _WIN32
	_close(all.final_prediction_sink[i]);
#else
	close(all.final_prediction_sink[i]);
#endif
    all.final_prediction_sink.delete_v();
    delete all.loss;
    delete &all;
  }
}
