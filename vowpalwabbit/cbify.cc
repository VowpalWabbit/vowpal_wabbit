#include "oaa.h"
#include "vw.h"
#include "cb.h"
#include "rand48.h"

using namespace LEARNER;

namespace CBIFY {

  struct cbify {
    size_t k;
    
    size_t tau;

    float epsilon;
    
    CB::label cb_label;
  };
  
  uint32_t do_uniform(cbify& data)
  {  //Draw an action
    return (uint32_t)ceil(frand48() * data.k);
  }
  
  float loss(uint32_t label, float final_prediction)
  {
    if (label != final_prediction)
      return 1.;
    else
      return 0.;
  }

  template <bool is_learn>
  void predict_or_learn_first(cbify& data, learner& base, example& ec)
  {//Explore tau times, then act according to optimal.
    OAA::mc_label* ld = (OAA::mc_label*)ec.ld;
    //Use CB to find current prediction for remaining rounds.
    if (data.tau && is_learn > 0)
      {
	ec.final_prediction = do_uniform(data);
	ec.loss = loss(ld->label, ec.final_prediction);
	data.tau--;
	uint32_t action = (uint32_t)ec.final_prediction;
	CB::cb_class l = {ec.loss, action, 1.f / data.k};
	data.cb_label.costs.erase();
	data.cb_label.costs.push_back(l);
	ec.ld = &(data.cb_label);
	base.learn(ec);
	ec.final_prediction = (float)action;
	ec.loss = l.cost;
      }
    else
      {
	data.cb_label.costs.erase();
	ec.ld = &(data.cb_label);
	base.predict(ec);
	ec.loss = loss(ld->label, ec.final_prediction);
      }
    ec.ld = ld;
  }
  
  template <bool is_learn>
  void predict_or_learn_greedy(cbify& data, learner& base, example& ec)
  {//Explore uniform random an epsilon fraction of the time.
    OAA::mc_label* ld = (OAA::mc_label*)ec.ld;
    ec.ld = &(data.cb_label);
    data.cb_label.costs.erase();
    
    base.predict(ec);
    uint32_t action = (uint32_t)ec.final_prediction;

    float base_prob = data.epsilon / data.k;
    if (frand48() < 1. - data.epsilon)
      {
	CB::cb_class l = {loss(ld->label, ec.final_prediction), 
			  action, 1.f - data.epsilon + base_prob};
	data.cb_label.costs.push_back(l);
      }
    else
      {
	action = do_uniform(data);
	CB::cb_class l = {loss(ld->label, action), 
			  action, base_prob};
	if (action == ec.final_prediction)
	  l.probability = 1.f - data.epsilon + base_prob;
	data.cb_label.costs.push_back(l);
      }

    if (is_learn)
      base.learn(ec);
    
    ec.final_prediction = (float)action;
    ec.loss = loss(ld->label, ec.final_prediction);
    ec.ld = ld;
  }

    void learn_bagging(void* d, learner& base, example* ec)
    {//Randomize over predictions from a base set of predictors
      //      cbify* data = (cbify*)d;
      
      //Use CB to find current predictions.
    }

    void learn_cover(void* d, learner& base, example* ec)
    {//Randomize over predictions from a base set of predictors
      //cbify* data = (cbify*)d;
      
      //Use cost sensitive oracle to cover actions to form distribution.
    }

  void init_driver(cbify&) {}

  void finish_example(vw& all, cbify&, example& ec)
  {
    OAA::output_example(all, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {//parse and set arguments
    cbify* data = (cbify*)calloc(1, sizeof(cbify));

    data->epsilon = 0.05f;
    data->tau = 1000;
    po::options_description desc("CBIFY options");
    desc.add_options()
      ("first", po::value<size_t>(), "tau-first exploration")
      ("greedy",po::value<float>() ,"epsilon-greedy exploration");
    
    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);
    
    po::parsed_options parsed_file = po::command_line_parser(all.options_from_file_argc,all.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);
    
    if( vm_file.count("cbify") ) {
      data->k = (uint32_t)vm_file["cbify"].as<size_t>();
      if( vm.count("cbify") && (uint32_t)vm["cbify"].as<size_t>() != data->k )
        std::cerr << "warning: you specified a different number of actions through --cbify than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
    }
    else {
      data->k = (uint32_t)vm["cbify"].as<size_t>();
      
      //appends nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --cbify " << data->k;
      all.options_from_file.append(ss.str());
    }

    all.p->lp = OAA::mc_label_parser;
    learner* l = new learner(data, all.l, 1);
    if (vm.count("first") )
      {
	data->tau = (uint32_t)vm["first"].as<size_t>();
	l->set_learn<cbify, predict_or_learn_first<true> >();
	l->set_predict<cbify, predict_or_learn_first<false> >();
      }
    else
      {
	if ( vm.count("greedy") ) 
	  data->epsilon = vm["greedy"].as<float>();
	l->set_learn<cbify, predict_or_learn_greedy<true> >();
	l->set_predict<cbify, predict_or_learn_greedy<false> >();
      }
    l->set_finish_example<cbify,finish_example>();
    l->set_init_driver<cbify,init_driver>();
    
    return l;
  }
}
