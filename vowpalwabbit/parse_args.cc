/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdio.h>
#include <float.h>
#include <sstream>

#include "cache.h"
#include "io_buf.h"
#include "parse_regressor.h"
#include "parser.h"
#include "parse_args.h"
#include "sender.h"
#include "network.h"
#include "global_data.h"
#include "nn.h"
#include "cbify.h"
#include "oaa.h"
#include "rand48.h"
#include "bs.h"
#include "topk.h"
#include "ect.h"
#include "csoaa.h"
#include "wap.h"
#include "cb.h"
#include "cb_algs.h"
#include "scorer.h"
#include "searn.h"
#include "bfgs.h"
#include "lda_core.h"
#include "noop.h"
#include "print.h"
#include "gd_mf.h"
#include "mf.h"
#include "vw.h"
#include "rand48.h"
#include "parse_args.h"
#include "binary.h"
#include "lrq.h"
#include "autolink.h"
#include "memory.h"
#include "stagewise_poly.h"
#include "stagewise_poly_ssm.h"

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


void parse_affix_argument(vw&all, string str) {
  if (str.length() == 0) return;
  char* cstr = (char*)calloc_or_die(str.length()+1, sizeof(char));
  strcpy(cstr, str.c_str());

  char*p = strtok(cstr, ",");
  while (p != 0) {
    char*q = p;
    uint16_t prefix = 1;
    if (q[0] == '+') { q++; }
    else if (q[0] == '-') { prefix = 0; q++; }
    if ((q[0] < '1') || (q[0] > '7')) {
      cerr << "malformed affix argument (length must be 1..7): " << p << endl;
      throw exception();
    }
    uint16_t len = (uint16_t)(q[0] - '0');
    uint16_t ns = (uint16_t)' ';  // default namespace
    if (q[1] != 0) {
      if (valid_ns(q[1]))
        ns = (uint16_t)q[1];
      else {
        cerr << "malformed affix argument (invalid namespace): " << p << endl;
        throw exception();
      }
      if (q[2] != 0) {
        cerr << "malformed affix argument (too long): " << p << endl;
        throw exception();
      }
    }

    uint16_t afx = (len << 1) | (prefix & 0x1);
    all.affix_features[ns] <<= 4;
    all.affix_features[ns] |=  afx;

    p = strtok(NULL, ",");
  }

  free(cstr);
}

void parse_diagnostics(vw& all, po::variables_map& vm, int argc)
{
  po::options_description diag_opt("Diagnostic options");

  diag_opt.add_options()
    ("version","Version information")
    ("audit,a", "print weights of features")
    ("progress,P", po::value< string >(), "Progress update frequency. int: additive, float: multiplicative")
    ("quiet", "Don't output disgnostics and progress updates")
    ("help,h","Look here: http://hunch.net/~vw/ and click on Tutorial.");
  
  vm = add_options(all, diag_opt);

  if (vm.count("version")) {
    /* upon direct query for version -- spit it out to stdout */
    cout << version.to_string() << "\n";
    exit(0);
  }

  if (vm.count("quiet")) {
    all.quiet = true;
    // --quiet wins over --progress
  } else {
    all.quiet = false;

    if (vm.count("progress")) {
      string progress_str = vm["progress"].as<string>();
      all.progress_arg = (float)::atof(progress_str.c_str());

      // --progress interval is dual: either integer or floating-point
      if (progress_str.find_first_of(".") == string::npos) {
        // No "." in arg: assume integer -> additive
        all.progress_add = true;
        if (all.progress_arg < 1) {
          cerr    << "warning: additive --progress <int>"
                  << " can't be < 1: forcing to 1\n";
          all.progress_arg = 1;

        }
        all.sd->dump_interval = all.progress_arg;

      } else {
        // A "." in arg: assume floating-point -> multiplicative
        all.progress_add = false;

        if (all.progress_arg <= 1.0) {
          cerr    << "warning: multiplicative --progress <float>: "
                  << vm["progress"].as<string>()
                  << " is <= 1.0: adding 1.0\n";
          all.progress_arg += 1.0;

        } else if (all.progress_arg > 9.0) {
          cerr    << "warning: multiplicative --progress <float>"
                  << " is > 9.0: you probably meant to use an integer\n";
        }
        all.sd->dump_interval = 1.0;
      }
    }
  }  

  if (vm.count("audit")){
    all.audit = true;
  }
}

void parse_source(vw& all, po::variables_map& vm)
{
  //begin input source
  if (vm.count("no_stdin"))
    all.stdin_off = true;
  
  if ( (vm.count("total") || vm.count("node") || vm.count("unique_id")) && !(vm.count("total") && vm.count("node") && vm.count("unique_id")) )
    {
      cout << "you must specificy unique_id, total, and node if you specify any" << endl;
      throw exception();
    }
  
  if (vm.count("daemon") || vm.count("pid_file") || (vm.count("port") && !all.active) ) {
    all.daemon = true;
    
    // allow each child to process up to 1e5 connections
    all.numpasses = (size_t) 1e5;
  }

  if (vm.count("compressed"))
      set_compressed(all.p);

  if (vm.count("data")) {
    all.data_filename = vm["data"].as<string>();
    if (ends_with(all.data_filename, ".gz"))
      set_compressed(all.p);
  } else
    all.data_filename = "";

  if(!all.holdout_set_off && (vm.count("output_feature_regularizer_binary") || vm.count("output_feature_regularizer_text")))
    {
      all.holdout_set_off = true;
      cerr<<"Making holdout_set_off=true since output regularizer specified\n";
    }
}

void parse_feature_tweaks(vw& all, po::variables_map& vm)
{
  po::options_description feature_opt("Feature options");
  feature_opt.add_options()
    ("hash", po::value< string > (), "how to hash the features. Available options: strings, all")
    ("ignore", po::value< vector<unsigned char> >(), "ignore namespaces beginning with character <arg>")
    ("keep", po::value< vector<unsigned char> >(), "keep namespaces beginning with character <arg>")
    ("bit_precision,b", po::value<size_t>(), "number of bits in the feature table")
    ("noconstant", "Don't add a constant feature")
    ("constant,C", po::value<float>(&(all.initial_constant)), "Set initial value of constant")
    ("ngram", po::value< vector<string> >(), "Generate N grams. To generate N grams for a single namespace 'foo', arg should be fN.")
    ("skips", po::value< vector<string> >(), "Generate skips in N grams. This in conjunction with the ngram tag can be used to generate generalized n-skip-k-gram. To generate n-skips for a single namespace 'foo', arg should be fn.")
    ("affix", po::value<string>(), "generate prefixes/suffixes of features; argument '+2a,-3b,+1' means generate 2-char prefixes for namespace a, 3-char suffixes for b and 1 char prefixes for default namespace")
    ("spelling", po::value< vector<string> >(), "compute spelling features for a give namespace (use '_' for default namespace)")
    ("quadratic,q", po::value< vector<string> > (), "Create and use quadratic features")
    ("q:", po::value< string >(), ": corresponds to a wildcard for all printable characters")
    ("cubic", po::value< vector<string> > (),
     "Create and use cubic features");

  vm = add_options(all, feature_opt);

  //feature manipulation
  string hash_function("strings");
  if(vm.count("hash")) 
    hash_function = vm["hash"].as<string>();
  all.p->hasher = getHasher(hash_function);
      
  if (vm.count("spelling")) {
    vector<string> spelling_ns = vm["spelling"].as< vector<string> >();
    for (size_t id=0; id<spelling_ns.size(); id++)
      if (spelling_ns[id][0] == '_') all.spelling_features[(unsigned char)' '] = true;
      else all.spelling_features[(size_t)spelling_ns[id][0]] = true;
  }

  if (vm.count("affix")) {
    parse_affix_argument(all, vm["affix"].as<string>());
    stringstream ss;
    ss << " --affix " << vm["affix"].as<string>();
    all.file_options.append(ss.str());
  }

  if(vm.count("ngram")){
    if(vm.count("sort_features"))
      {
	cerr << "ngram is incompatible with sort_features.  " << endl;
	throw exception();
      }

    all.ngram_strings = vm["ngram"].as< vector<string> >();
    compile_gram(all.ngram_strings, all.ngram, (char*)"grams", all.quiet);
  }

  if(vm.count("skips"))
    {
      if(!vm.count("ngram"))
	{
	  cout << "You can not skip unless ngram is > 1" << endl;
	  throw exception();
	}

      all.skip_strings = vm["skips"].as<vector<string> >();
      compile_gram(all.skip_strings, all.skips, (char*)"skips", all.quiet);
    }

  if (vm.count("bit_precision"))
    {
      all.default_bits = false;
      all.num_bits = (uint32_t)vm["bit_precision"].as< size_t>();
      if (all.num_bits > min(32, sizeof(size_t)*8 - 3))
	{
	  cout << "Only " << min(32, sizeof(size_t)*8 - 3) << " or fewer bits allowed.  If this is a serious limit, speak up." << endl;
	  throw exception();
	}
    }

  if (vm.count("quadratic"))
    {
      all.pairs = vm["quadratic"].as< vector<string> >();
      vector<string> newpairs;
      //string tmp;
      char printable_start = '!';
      char printable_end = '~';
      int valid_ns_size = printable_end - printable_start - 1; //will skip two characters

      if(!all.quiet)
        cerr<<"creating quadratic features for pairs: ";

      for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++){
        if(!all.quiet){
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
            if(valid_ns(j)){
	      stringstream ss;
	      ss << j << (*i)[1];
	      newpairs.push_back(ss.str());
	    }
          }
        }
        //-q ::
        else if((*i)[0]==':'&&(*i)[1]==':'){
	  cout << "in pair creation" << endl;
          newpairs.reserve(newpairs.size() + valid_ns_size*valid_ns_size);
	  stringstream ss;
	  ss << ' ' << ' ';
	  newpairs.push_back(ss.str());
          for (char j=printable_start; j<=printable_end; j++){
            if(valid_ns(j)){
              for (char k=printable_start; k<=printable_end; k++){
                if(valid_ns(k)){
		  stringstream ss;
                  ss << j << k;
                  newpairs.push_back(ss.str());
		}
              }
            }
          }
        }
        else{
          newpairs.push_back(string(*i));
        }
      }
      newpairs.swap(all.pairs);
      if(!all.quiet)
        cerr<<endl;
    }

  if (vm.count("cubic"))
    {
      all.triples = vm["cubic"].as< vector<string> >();
      if (!all.quiet)
	{
	  cerr << "creating cubic features for triples: ";
	  for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) {
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

  if (vm.count("noconstant"))
    all.add_constant = false;
}

void parse_example_tweaks(vw& all, po::variables_map& vm)
{
  po::options_description example_opts("Example options");
  
  example_opts.add_options()
    ("testonly,t", "Ignore label information and just test")
    ("holdout_off", "no holdout data in multiple passes")
    ("holdout_period", po::value<uint32_t>(&(all.holdout_period)), "holdout period for test only, default 10")
    ("holdout_after", po::value<uint32_t>(&(all.holdout_after)), "holdout after n training examples, default off (disables holdout_period)")
    ("early_terminate", po::value<size_t>(), "Specify the number of passes tolerated when holdout loss doesn't decrease before early termination, default is 3")
    ("passes", po::value<size_t>(&(all.numpasses)),"Number of Training Passes")
    ("initial_pass_length", po::value<size_t>(&(all.pass_length)), "initial number of examples per pass")
    ("examples", po::value<size_t>(&(all.max_examples)), "number of examples to parse")
    ("min_prediction", po::value<float>(&(all.sd->min_label)), "Smallest prediction to output")
    ("max_prediction", po::value<float>(&(all.sd->max_label)), "Largest prediction to output")
    ("sort_features", "turn this on to disregard order in which features have been defined. This will lead to smaller cache sizes")
    ("loss_function", po::value<string>()->default_value("squared"), "Specify the loss function to be used, uses squared by default. Currently available ones are squared, classic, hinge, logistic and quantile.")
    ("quantile_tau", po::value<float>()->default_value(0.5), "Parameter \\tau associated with Quantile loss. Defaults to 0.5")
    ("l1", po::value<float>(&(all.l1_lambda)), "l_1 lambda")
    ("l2", po::value<float>(&(all.l2_lambda)), "l_2 lambda");

  vm = add_options(all, example_opts);

  if (vm.count("testonly") || all.eta == 0.)
    {
      if (!all.quiet)
	cerr << "only testing" << endl;
      all.training = false;
      if (all.lda > 0)
        all.eta = 0;
    }
  else
    all.training = true;

  if(all.numpasses > 1)
      all.holdout_set_off = false;

  if(vm.count("holdout_off"))
      all.holdout_set_off = true;

  if(vm.count("sort_features"))
    all.p->sort_features = true;
  
  if (vm.count("min_prediction"))
    all.sd->min_label = vm["min_prediction"].as<float>();
  if (vm.count("max_prediction"))
    all.sd->max_label = vm["max_prediction"].as<float>();
  if (vm.count("min_prediction") || vm.count("max_prediction") || vm.count("testonly"))
    all.set_minmax = noop_mm;

  string loss_function;
  if(vm.count("loss_function"))
    loss_function = vm["loss_function"].as<string>();
  else
    loss_function = "squaredloss";
  float loss_parameter = 0.0;
  if(vm.count("quantile_tau"))
    loss_parameter = vm["quantile_tau"].as<float>();

  all.loss = getLossFunction(&all, loss_function, (float)loss_parameter);

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
      if (all.reg_mode %2 && !vm.count("bfgs"))
	cerr << "using l1 regularization = " << all.l1_lambda << endl;
      if (all.reg_mode > 1)
	cerr << "using l2 regularization = " << all.l2_lambda << endl;
    }
}

void parse_output_preds(vw& all, po::variables_map& vm)
{
  if (vm.count("predictions")) {
    if (!all.quiet)
      cerr << "predictions = " <<  vm["predictions"].as< string >() << endl;
    if (strcmp(vm["predictions"].as< string >().c_str(), "stdout") == 0)
      {
	all.final_prediction_sink.push_back((size_t) 1);//stdout
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
	all.final_prediction_sink.push_back((size_t) f);
      }
  }

  if (vm.count("raw_predictions")) {
    if (!all.quiet) {
      cerr << "raw predictions = " <<  vm["raw_predictions"].as< string >() << endl;
      if (vm.count("binary"))
        cerr << "Warning: --raw has no defined value when --binary specified, expect no output" << endl;
    }
    if (strcmp(vm["raw_predictions"].as< string >().c_str(), "stdout") == 0)
      all.raw_prediction = 1;//stdout
    else
	{
	  const char* t = vm["raw_predictions"].as< string >().c_str();
	  int f;
#ifdef _WIN32
	  _sopen_s(&f, t, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
	  f = open(t, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
	  all.raw_prediction = f;
	}
  }
}

void parse_output_model(vw& all, po::variables_map& vm)
{
  po::options_description output_model("Output model");
  
  output_model.add_options()
    ("final_regressor,f", po::value< string >(), "Final regressor")
    ("readable_model", po::value< string >(), "Output human-readable final regressor with numeric features")
    ("invert_hash", po::value< string >(), "Output human-readable final regressor with feature names")
    ("save_per_pass", "Save the model after every pass over data")
    ("output_feature_regularizer_binary", po::value< string >(&(all.per_feature_regularizer_output)), "Per feature regularization output file")
    ("output_feature_regularizer_text", po::value< string >(&(all.per_feature_regularizer_text)), "Per feature regularization output file, in text");
  
  vm = add_options(all, output_model);

  if (vm.count("final_regressor")) {
    all.final_regressor_name = vm["final_regressor"].as<string>();
    if (!all.quiet)
      cerr << "final_regressor = " << vm["final_regressor"].as<string>() << endl;
  }
  else
    all.final_regressor_name = "";

  if (vm.count("readable_model"))
    all.text_regressor_name = vm["readable_model"].as<string>();

  if (vm.count("invert_hash")){
    all.inv_hash_regressor_name = vm["invert_hash"].as<string>();
    all.hash_inv = true;
  }

  if (vm.count("save_per_pass"))
    all.save_per_pass = true;

  if (vm.count("save_resume"))
    all.save_resume = true;
}

void parse_base_algorithm(vw& all, po::variables_map& vm)
{
  //base learning algorithm.
  po::options_description base_opt("base algorithms (these are exclusive)");
  
  base_opt.add_options()
    ("sgd", "use regular stochastic gradient descent update.")
    ("adaptive", "use adaptive, individual learning rates.")
    ("invariant", "use safe/importance aware updates.")
    ("normalized", "use per feature normalized updates")
    ("exact_adaptive_norm", "use current default invariant normalized adaptive update rule")
    ("bfgs", "use bfgs optimization")
    ("lda", po::value<uint32_t>(&(all.lda)), "Run lda with <int> topics")
    ("rank", po::value<uint32_t>(&(all.rank)), "rank for matrix factorization.")
    ("noop","do no learning")
    ("print","print examples")
    ("sendto", po::value< vector<string> >(), "send examples to <host>");

  vm = add_options(all, base_opt);

  if (vm.count("bfgs") || vm.count("conjugate_gradient"))
    all.l = BFGS::setup(all, vm);
  else if (vm.count("lda"))
    all.l = LDA::setup(all, vm);
  else if (vm.count("noop"))
    all.l = NOOP::setup(all);
  else if (vm.count("print"))
    all.l = PRINT::setup(all);
  else if (!vm.count("new_mf") && all.rank > 0)
    all.l = GDMF::setup(all, vm);
  else if (vm.count("sendto"))
    all.l = SENDER::setup(all, vm, all.pairs);
  else
    {
      all.l = GD::setup(all, vm);
      all.scorer = all.l;
    }
}

void load_input_model(vw& all, po::variables_map& vm, io_buf& io_temp)
{
  // Need to see if we have to load feature mask first or second.
  // -i and -mask are from same file, load -i file first so mask can use it
  if (vm.count("feature_mask") && vm.count("initial_regressor")
      && vm["feature_mask"].as<string>() == vm["initial_regressor"].as< vector<string> >()[0]) {
    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();

    // set the mask, which will reuse -i file we just loaded
    parse_mask_regressor_args(all, vm);
  }
  else {
    // load mask first
    parse_mask_regressor_args(all, vm);

    // load rest of regressor
    all.l->save_load(io_temp, true, false);
    io_temp.close_file();
  }
}

void parse_scorer_reductions(vw& all, po::variables_map& vm)
{
  po::options_description score_mod_opt("Score modifying options (can be combined)");

  score_mod_opt.add_options()
    ("nn", po::value<size_t>(), "Use sigmoidal feedforward network with <k> hidden units")
    ("new_mf", "use new, reduction-based matrix factorization")
    ("autolink", po::value<size_t>(), "create link function with polynomial d")
    ("lrq", po::value<vector<string> > (), "use low rank quadratic features")
    ("lrqdropout", "use dropout training for low rank quadratic features")
    ("stage_poly", "use stagewise polynomial feature learning")
    ("stage_poly_ssm", "use stagewise polynomial feature learning with SSM heuristic");

  vm = add_options(all, score_mod_opt);

  if(vm.count("nn"))
    all.l = NN::setup(all, vm);
  
  if (vm.count("new_mf") && all.rank > 0)
    all.l = MF::setup(all, vm);
  
  if(vm.count("autolink"))
    all.l = ALINK::setup(all, vm);
  
  if (vm.count("lrq"))
    all.l = LRQ::setup(all, vm);

  if (vm.count("stage_poly"))
    all.l = StagewisePoly::setup(all, vm);

  if (vm.count("stage_poly_ssm"))
    all.l = StagewisePoly_SSM::setup(all, vm);
  
  all.l = Scorer::setup(all, vm);
}

LEARNER::learner* exclusive_setup(vw& all, po::variables_map& vm, bool& score_consumer, LEARNER::learner* (*setup)(vw&, po::variables_map&))
{
  if (score_consumer) { cerr << "error: cannot specify multiple direct score consumers" << endl; throw exception(); }
  score_consumer = true;
  return setup(all, vm);
}

void parse_score_users(vw& all, po::variables_map& vm, bool& got_cs)
{
  po::options_description multiclass_opt("Score user options (these are exclusive)");
  multiclass_opt.add_options()
    ("top", po::value<size_t>(), "top k recommendation")
    ("binary", "report loss as binary classification on -1,1")
    ("oaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> labels")
    ("ect", po::value<size_t>(), "Use error correcting tournament with <k> labels")
    ("csoaa", po::value<size_t>(), "Use one-against-all multiclass learning with <k> costs")
    ("wap", po::value<size_t>(), "Use weighted all-pairs multiclass learning with <k> costs")
    ("csoaa_ldf", po::value<string>(), "Use one-against-all multiclass learning with label dependent features.  Specify singleline or multiline.")
    ("wap_ldf", po::value<string>(), "Use weighted all-pairs multiclass learning with label dependent features.  Specify singleline or multiline.")
    ;

  vm = add_options(all, multiclass_opt);
  bool score_consumer = false;
  
  if(vm.count("top"))
    all.l = exclusive_setup(all, vm, score_consumer, TOPK::setup);
  
  if (vm.count("binary"))
    all.l = exclusive_setup(all, vm, score_consumer, BINARY::setup);
  
  if (vm.count("oaa")) 
    all.l = exclusive_setup(all, vm, score_consumer, OAA::setup);
  
  if (vm.count("ect")) 
    all.l = exclusive_setup(all, vm, score_consumer, ECT::setup);
  
  if(vm.count("csoaa")) {
    all.l = exclusive_setup(all, vm, score_consumer, CSOAA::setup);
    all.cost_sensitive = all.l;
    got_cs = true;
  }
  
  if(vm.count("wap")) {
    all.l = exclusive_setup(all, vm, score_consumer, WAP::setup);
    all.cost_sensitive = all.l;
    got_cs = true;
  }
  
  if(vm.count("csoaa_ldf") || vm.count("csoaa_ldf")) {
    all.l = exclusive_setup(all, vm, score_consumer, CSOAA_AND_WAP_LDF::setup);
    all.cost_sensitive = all.l;
    got_cs = true;
  }
  
  if(vm.count("wap_ldf") || vm.count("wap_ldf") ) {
    all.l = exclusive_setup(all, vm, score_consumer, CSOAA_AND_WAP_LDF::setup);
    all.cost_sensitive = all.l;
    got_cs = true;
  }
}

void parse_cb(vw& all, po::variables_map& vm, bool& got_cs, bool& got_cb)
{
  if( vm.count("cb") || vm.count("cb") )
    {
      if(!got_cs) {
	if( vm.count("cb") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["cb"]));
	else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["cb"]));
	
	all.l = CSOAA::setup(all, vm);  // default to CSOAA unless wap is specified
	all.cost_sensitive = all.l;
	got_cs = true;
      }
      
      all.l = CB_ALGS::setup(all, vm);
      got_cb = true;
    }

  if (vm.count("cbify") || vm.count("cbify"))
    {
      if(!got_cs) {
	if( vm.count("cbify") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["cbify"]));
	else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["cbify"]));
	
	all.l = CSOAA::setup(all, vm);  // default to CSOAA unless wap is specified
	all.cost_sensitive = all.l;
	got_cs = true;
      }

      if (!got_cb) {
	if( vm.count("cbify") ) vm.insert(pair<string,po::variable_value>(string("cb"),vm["cbify"]));
	else vm.insert(pair<string,po::variable_value>(string("cb"),vm["cbify"]));
	all.l = CB_ALGS::setup(all, vm);
	got_cb = true;
      }

      all.l = CBIFY::setup(all, vm);
    }
}

void parse_search(vw& all, po::variables_map& vm, bool& got_cs, bool& got_cb)
{
  if (vm.count("search")) {
    if (!got_cs && !got_cb) {
      if( vm.count("search") ) vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["search"]));
      else vm.insert(pair<string,po::variable_value>(string("csoaa"),vm["search"]));
      
      all.l = CSOAA::setup(all, vm);  // default to CSOAA unless others have been specified
      all.cost_sensitive = all.l;
      got_cs = true;
    }
    //all.searnstr = (Searn::searn*)calloc_or_die(1, sizeof(Searn::searn));
    all.l = Searn::setup(all, vm);
  }
}

void add_to_args(vw& all, int argc, char* argv[])
{
  for (int i = 1; i < argc; i++)
    all.args.push_back(string(argv[i]));
}

vw* parse_args(int argc, char *argv[])
{
  po::options_description desc("VW options");

  vw* all = new vw();

  add_to_args(*all, argc, argv);

  size_t random_seed = 0;
  all->program_name = argv[0];

  po::options_description in_opt("Input options");

  in_opt.add_options()
    ("data,d", po::value< string >(), "Example Set")
    ("daemon", "persistent daemon mode on port 26542")
    ("port", po::value<size_t>(),"port to listen on; use 0 to pick unused port")
    ("num_children", po::value<size_t>(&(all->num_children)), "number of children for persistent daemon mode")
    ("pid_file", po::value< string >(), "Write pid file in persistent daemon mode")
    ("port_file", po::value< string >(), "Write port used in persistent daemon mode")
    ("cache,c", "Use a cache.  The default is <data>.cache")
    ("cache_file", po::value< vector<string> >(), "The location(s) of cache_file.")
    ("kill_cache,k", "do not reuse existing cache: create a new one always")
    ("compressed", "use gzip format whenever possible. If a cache file is being created, this option creates a compressed cache file. A mixture of raw-text & compressed inputs are supported with autodetection.")
    ("no_stdin", "do not default to reading from stdin")
    ("ring_size", po::value<size_t>(&(all->p->ring_size)), "size of example ring")
    ("save_resume", "save extra state so learning can be resumed later with new data")
    ;

  po::options_description out_opt("Output options");

  out_opt.add_options()
    ("predictions,p", po::value< string >(), "File to output predictions to")
    ("raw_predictions,r", po::value< string >(), "File to output unnormalized predictions to")
    ;

  po::options_description update_opt("Update options");

  update_opt.add_options()
    ("learning_rate,l", po::value<float>(&(all->eta)), "Set Learning Rate")
    ("power_t", po::value<float>(&(all->power_t)), "t power value")
    ("decay_learning_rate",    po::value<float>(&(all->eta_decay_rate)),
     "Set Decay factor for learning_rate between passes")
    ("initial_t", po::value<double>(&((all->sd->t))), "initial t value")
    ("feature_mask", po::value< string >(), "Use existing regressor to determine which parameters may be updated.  If no initial_regressor given, also used for initial weights.")
    ;

  po::options_description weight_opt("Weight options");

  weight_opt.add_options()
    ("initial_regressor,i", po::value< vector<string> >(), "Initial regressor(s)")
    ("initial_weight", po::value<float>(&(all->initial_weight)), "Set all weights to an initial value of 1.")
    ("random_weights", po::value<bool>(&(all->random_weights)), "make initial weights random")
    ("input_feature_regularizer", po::value< string >(&(all->per_feature_regularizer_input)), "Per feature regularization input file")
    ;

  po::options_description active_opt("Active Learning options");
  active_opt.add_options()
    ("active_learning", "active learning mode")
    ("active_simulation", "active learning simulation mode")
    ("active_mellowness", po::value<float>(&(all->active_c0)), "active learning mellowness parameter c_0. Default 8")
    ;

  po::options_description cluster_opt("Parallelization options");
  cluster_opt.add_options()
    ("span_server", po::value<string>(&(all->span_server)), "Location of server for setting up spanning tree")
    ("unique_id", po::value<size_t>(&(all->unique_id)),"unique id used for cluster parallel jobs")
    ("total", po::value<size_t>(&(all->total)),"total number of nodes used in cluster parallel job")
    ("node", po::value<size_t>(&(all->node)),"node number in cluster parallel job")
    ;

  po::options_description other_opt("Other options");
  other_opt.add_options()
    ("bootstrap", po::value<size_t>(), "bootstrap mode with k rounds by online importance resampling")
    ("cb", po::value<size_t>(), "Use contextual bandit learning with <k> costs")
    ("cbify", po::value<size_t>(), "Convert multiclass on <k> classes into a contextual bandit problem and solve")
    ("search", po::value<size_t>(), "use search-based structured prediction, argument=maximum action id or 0 for LDF")    
    ;

  // Declare the supported options.
  desc.add_options()
    ("random_seed", po::value<size_t>(&random_seed), "seed random number generator");

  desc.add(in_opt)
    .add(out_opt)
    .add(update_opt)
    .add(weight_opt)
    .add(active_opt)
    .add(cluster_opt)
    .add(other_opt);

  po::variables_map vm = add_options(*all, desc);

  msrand48(random_seed);

  parse_diagnostics(*all, vm, argc);

  if (vm.count("active_simulation"))
    all->active_simulation = true;
  
  if (vm.count("active_learning") && !all->active_simulation)
    all->active = true;
  
  all->sd->weighted_unlabeled_examples = all->sd->t;
  all->initial_t = (float)all->sd->t;

  if(all->initial_t > 0)//for the normalized update: if initial_t is bigger than 1 we interpret this as if we had seen (all->initial_t) previous fake datapoints all with norm 1
    all->normalized_sum_norm_x = all->initial_t;

  //Input regressor header
  io_buf io_temp;
  parse_regressor_args(*all, vm, io_temp);
  
  int temp_argc = 0;
  char** temp_argv = VW::get_argv_from_string(all->file_options, temp_argc);
  add_to_args(*all, temp_argc, temp_argv);
  
  po::parsed_options pos = po::command_line_parser(all->args).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(all->opts).allow_unregistered().run();

  vm = po::variables_map();

  po::store(pos, vm);
  po::notify(vm);

  parse_feature_tweaks(*all, vm); //feature tweaks

  parse_example_tweaks(*all, vm); //example manipulation

  parse_base_algorithm(*all, vm);

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

  parse_output_model(*all, vm);
  
  parse_output_preds(*all, vm);

  parse_scorer_reductions(*all, vm);

  load_input_model(*all, vm, io_temp);

  

  bool got_cs = false;
  
  parse_score_users(*all, vm, got_cs);

  bool got_cb = false;
  
  parse_cb(*all, vm, got_cs, got_cb);

  parse_search(*all, vm, got_cs, got_cb);
  

  if(vm.count("bootstrap"))
    all->l = BS::setup(*all, vm);

  // Be friendly: if -d was left out, treat positional param as data file
  po::positional_options_description p;  
  p.add("data", -1);

  vm = po::variables_map();
  pos = po::command_line_parser(all->args).
    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
    options(all->opts).positional(p).run();
    vm = po::variables_map();
  po::store(pos, vm);

  parse_source(*all, vm);

  enable_sources(*all, vm, all->quiet,all->numpasses);

  // force wpp to be a power of 2 to avoid 32-bit overflow
  uint32_t i = 0;
  size_t params_per_problem = all->l->increment;
  while (params_per_problem > (uint32_t)(1 << i))
    i++;
  all->wpp = (1 << i) >> all->reg.stride_shift;

  if (vm.count("help")) {
    /* upon direct query for help -- spit it out to stdout */
    cout << "\n" << all->opts << "\n";
    exit(0);
  }
  
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
    char* c = (char*)calloc_or_die(s.length()+3, sizeof(char));
    c[0] = 'b';
    c[1] = ' ';
    strcpy(c+2, s.c_str());
    substring ss = {c, c+s.length()+2};
    v_array<substring> foo;
    foo.end_array = foo.begin = foo.end = NULL;
    tokenize(' ', ss, foo);

    char** argv = (char**)calloc_or_die(foo.size(), sizeof(char*));
    for (size_t i = 0; i < foo.size(); i++)
      {
	*(foo[i].end) = '\0';
	argv[i] = (char*)calloc_or_die(foo[i].end-foo[i].begin+1, sizeof(char));
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
    
    initialize_parser_datastructures(*all);

    for(int i = 0; i < argc; i++)
      free(argv[i]);
    free (argv);

    return all;
  }

  void finish(vw& all)
  {
    finalize_regressor(all, all.final_regressor_name);
    all.l->finish();
    delete all.l;
    if (all.reg.weight_vector != NULL)
      free(all.reg.weight_vector);
    free_parser(all);
    finalize_source(all.p);
    all.p->parse_name.erase();
    all.p->parse_name.delete_v();
    free(all.p);
    free(all.sd);
    for (size_t i = 0; i < all.final_prediction_sink.size(); i++)
      if (all.final_prediction_sink[i] != 1)
	io_buf::close_file_or_socket(all.final_prediction_sink[i]);
    all.final_prediction_sink.delete_v();
    delete all.loss;
    delete &all;
  }
}
