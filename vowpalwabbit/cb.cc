/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>

#include "cb.h"
#include "simple_label.h"
#include "example.h"
#include "csoaa.h"
#include "oaa.h"
#include "parse_example.h"
#include "parse_primitives.h"
#include "vw.h"

namespace CB
{
  struct cb {
    size_t cb_type;
    CSOAA::label cb_cs_ld; 
    float avg_loss_regressors;
    size_t nb_ex_regressors;
    float last_pred_reg;
    float last_correct_cost;

    float min_cost;
    float max_cost;

    cb_class* known_cost;
    vw* all;
  };

  bool know_all_cost_example(CB::label* ld)
  {
    if (ld->costs.size() <= 1) //this means we specified an example where all actions are possible but only specified the cost for the observed action
      return false;

    //if we specified more than 1 action for this example, i.e. either we have a limited set of possible actions, or all actions are specified
    //than check if all actions have a specified cost
    for (cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++)
      if (cl->x == FLT_MAX)
        return false;

    return true;
  }

  bool is_test_label(CB::label* ld)
  {
    if (ld->costs.size() == 0)
      return true;
    for (size_t i=0; i<ld->costs.size(); i++)
      if (FLT_MAX != ld->costs[i].x && ld->costs[i].prob_action > 0.)
        return false;
    return true;
  }
  
  char* bufread_label(CB::label* ld, char* c, io_buf& cache)
  {
    size_t num = *(size_t *)c;
    ld->costs.erase();
    c += sizeof(size_t);
    size_t total = sizeof(cb_class)*num;
    if (buf_read(cache, c, total) < total) 
      {
        cout << "error in demarshal of cost data" << endl;
        return c;
      }
    for (size_t i = 0; i<num; i++)
      {
        cb_class temp = *(cb_class *)c;
        c += sizeof(cb_class);
        ld->costs.push_back(temp);
      }
  
    return c;
  }

  size_t read_cached_label(shared_data*, void* v, io_buf& cache)
  {
    CB::label* ld = (CB::label*) v;
    ld->costs.erase();
    char *c;
    size_t total = sizeof(size_t);
    if (buf_read(cache, c, total) < total) 
      return 0;
    c = bufread_label(ld,c, cache);
  
    return total;
  }

  float weight(void* v)
  {
    return 1.;
  }

  float initial(void* v)
  {
    return 0.;
  }

  char* bufcache_label(CB::label* ld, char* c)
  {
    *(size_t *)c = ld->costs.size();
    c += sizeof(size_t);
    for (size_t i = 0; i< ld->costs.size(); i++)
      {
        *(cb_class *)c = ld->costs[i];
        c += sizeof(cb_class);
      }
    return c;
  }

  void cache_label(void* v, io_buf& cache)
  {
    char *c;
    CB::label* ld = (CB::label*) v;
    buf_write(cache, c, sizeof(size_t)+sizeof(cb_class)*ld->costs.size());
    bufcache_label(ld,c);
  }

  void default_label(void* v)
  {
    CB::label* ld = (CB::label*) v;
    ld->costs.erase();
  }

  void delete_label(void* v)
  {
    CB::label* ld = (CB::label*)v;
    ld->costs.delete_v();
  }

  void copy_label(void*&dst, void*src)
  {
    CB::label*&ldD = (CB::label*&)dst;
    CB::label* ldS = (CB::label* )src;
    copy_array(ldD->costs, ldS->costs);
  }

  void parse_label(parser* p, shared_data* sd, void* v, v_array<substring>& words)
  {
    CB::label* ld = (CB::label*)v;

    for (size_t i = 0; i < words.size(); i++)
      {
        cb_class f;
	tokenize(':', words[i], p->parse_name);

        if( p->parse_name.size() < 1 || p->parse_name.size() > 3 )
        {
          cerr << "malformed cost specification!" << endl;
	  cerr << "terminating." << endl;
          throw exception();
        }

        f.weight_index = (uint32_t)hashstring(p->parse_name[0], 0);
        if (f.weight_index < 1 || f.weight_index > sd->k)
        {
          cerr << "invalid action: " << f.weight_index << endl;
          cerr << "terminating." << endl;
          throw exception();
        }

        f.x = FLT_MAX;
        if(p->parse_name.size() > 1)
          f.x = float_of_substring(p->parse_name[1]);

        if ( nanpattern(f.x))
        {
	  cerr << "error NaN cost for action: ";
	  cerr.write(p->parse_name[0].begin, p->parse_name[0].end - p->parse_name[0].begin);
	  cerr << " terminating." << endl;
	  throw exception();
        }
      
        f.prob_action = .0;
        if(p->parse_name.size() > 2)
          f.prob_action = float_of_substring(p->parse_name[2]);

        if ( nanpattern(f.prob_action))
        {
	  cerr << "error NaN probability for action: ";
	  cerr.write(p->parse_name[0].begin, p->parse_name[0].end - p->parse_name[0].begin);
	  cerr << " terminating." << endl;
	  throw exception();
        }
        
        if( f.prob_action > 1.0 )
        {
          cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
          f.prob_action = 1.0;
        }
        if( f.prob_action < 0.0 )
        {
          cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
          f.prob_action = .0;
        }

        ld->costs.push_back(f);
      }
  }

  inline bool observed_cost(cb_class* cl)
  {
    //cost observed for this action if it has non zero probability and cost != FLT_MAX
    return (cl != NULL && cl->x != FLT_MAX && cl->prob_action > .0);
  }
  
  cb_class* get_observed_cost(CB::label* ld)
  {
    size_t i = 0;
    for (cb_class *cl = ld->costs.begin; cl != ld->costs.end; cl ++, i++)
    {
      if( observed_cost(cl) ) {
        return cl;
      }
    }
    return NULL;
  }

  void gen_cs_example_ips(vw& all, cb& c, example* ec, CSOAA::label& cs_ld)
  {//this implements the inverse propensity score method, where cost are importance weighted by the probability of the chosen action
    CB::label* ld = (CB::label*)ec->ld;
   
    //generate cost-sensitive example
    cs_ld.costs.erase();
    if( ld->costs.size() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for(uint32_t i = 1; i <= all.sd->k; i++)
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;
        wc.x = 0.;
        wc.weight_index = i;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        if( c.known_cost != NULL && i == c.known_cost->weight_index )
        {
          wc.x = c.known_cost->x / c.known_cost->prob_action; //use importance weighted cost for observed action, 0 otherwise 
          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x)*(c.known_cost->x) - c.avg_loss_regressors );
          c.last_pred_reg = 0;
          c.last_correct_cost = c.known_cost->x;
        }

        cs_ld.costs.push_back(wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;
        wc.x = 0.;
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        if( c.known_cost != NULL && cl->weight_index == c.known_cost->weight_index )
        {
          wc.x = c.known_cost->x / c.known_cost->prob_action; //use importance weighted cost for observed action, 0 otherwise 

          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x)*(c.known_cost->x) - c.avg_loss_regressors );
          c.last_pred_reg = 0;
          c.last_correct_cost = c.known_cost->x;
        }

        cs_ld.costs.push_back( wc );
      }
    }

  }

  void call_scorer(vw& all, cb& c, example* ec, uint32_t index)
  {
    float old_min = all.sd->min_label;
    //all.sd->min_label = c.min_cost;
    float old_max = all.sd->max_label;
    //all.sd->max_label = c.max_cost;
    all.scorer->learn(ec, 2*(index)-1);
    all.sd->min_label = old_min;
    all.sd->max_label = old_max;
   }
  
  float get_cost_pred(vw& all, cb& c, example* ec, uint32_t index)
  {
    CB::label* ld = (CB::label*)ec->ld;

    label_data simple_temp;
    simple_temp.initial = 0.;
    if (c.known_cost != NULL && index == c.known_cost->weight_index)
      {
	simple_temp.label = c.known_cost->x;
	simple_temp.weight = 1.;
      }
    else 
      {
	simple_temp.label = FLT_MAX;
	simple_temp.weight = 0.;
      }
    
    ec->ld = &simple_temp;

    call_scorer(all, c, ec, index);
    ec->ld = ld;

    float cost = ec->final_prediction;

    return cost;
  }

  void gen_cs_example_dm(vw& all, cb& c, example* ec, CSOAA::label& cs_ld)
  {
    //this implements the direct estimation method, where costs are directly specified by the learned regressor.
    CB::label* ld = (CB::label*)ec->ld;

    float min = FLT_MAX;
    size_t argmin = 1;
    //generate cost sensitive example
    cs_ld.costs.erase();  
    if( ld->costs.size() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions  
      for(uint32_t i = 1; i <= all.sd->k; i++)
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;
      
        //get cost prediction for this action
        wc.x = get_cost_pred(all, c, ec, i-1);
	if (wc.x < min)
	  {
	    min = wc.x;
	    argmin = i;
	  }

        wc.weight_index = i;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        if( c.known_cost != NULL && c.known_cost->weight_index == i ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x - wc.x)*(c.known_cost->x - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->x;
        }

        cs_ld.costs.push_back( wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;
      
        //get cost prediction for this action
        wc.x = get_cost_pred(all, c, ec, cl->weight_index - 1);
	if (wc.x < min || (wc.x == min && cl->weight_index < argmin))
	  {
	    min = wc.x;
	    argmin = cl->weight_index;
	  }

        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        if( c.known_cost != NULL && c.known_cost->weight_index == cl->weight_index ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x - wc.x)*(c.known_cost->x - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->x;
        }

        cs_ld.costs.push_back( wc );
      }
    }
    
    ec->final_prediction = (float)argmin;
  }

  void gen_cs_example_dr(vw& all, cb& c, example* ec, CSOAA::label& cs_ld)
  {//this implements the doubly robust method
    CB::label* ld = (CB::label*)ec->ld;
    
    //generate cost sensitive example
    cs_ld.costs.erase();
    if( ld->costs.size() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for(uint32_t i = 1; i <= all.sd->k; i++)
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;

        //get cost prediction for this label
        wc.x = get_cost_pred(all, c,ec, all.sd->k + i - 1);
        wc.weight_index = i;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        //add correction if we observed cost for this action and regressor is wrong
        if( c.known_cost != NULL && c.known_cost->weight_index == i ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x - wc.x)*(c.known_cost->x - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->x;
          wc.x += (c.known_cost->x - wc.x) / c.known_cost->prob_action;
        }

        cs_ld.costs.push_back( wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;

        //get cost prediction for this label
        wc.x = get_cost_pred(all, c, ec, all.sd->k + cl->weight_index - 1);
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        //add correction if we observed cost for this action and regressor is wrong
        if( c.known_cost != NULL && c.known_cost->weight_index == cl->weight_index ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->x - wc.x)*(c.known_cost->x - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->x;
          wc.x += (c.known_cost->x - wc.x) / c.known_cost->prob_action;
        }

        cs_ld.costs.push_back( wc );
      }
    }
  }

  void cb_test_to_cs_test_label(vw& all, example* ec, CSOAA::label& cs_ld)
  {
    CB::label* ld = (CB::label*)ec->ld;

    cs_ld.costs.erase();
    if(ld->costs.size() > 0)
    {
      //if this is a test example and we specified actions, this means we are only allowed to perform these actions, so copy all actions with their specified costs
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++)
      {
        CSOAA::wclass wc;
        wc.wap_value = 0.;

        wc.x = cl->x;
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        
        cs_ld.costs.push_back(wc);
      }
    }
  }

  void learn(void* d, learner& base, example* ec) {
    cb* c = (cb*)d;
    vw* all = c->all;
    CB::label* ld = (CB::label*)ec->ld;

    //check if this is a test example where we just want a prediction
    if( CB::is_test_label(ld) )
    {
       //if so just query base cost-sensitive learner
      cb_test_to_cs_test_label(*all,ec,c->cb_cs_ld);

       ec->ld = &c->cb_cs_ld;
       base.learn(ec);
       ec->ld = ld;
       return;
    }

    //now this is a training example
    c->known_cost = get_observed_cost(ld);
    c->min_cost = min (c->min_cost, c->known_cost->x);
    c->max_cost = max (c->max_cost, c->known_cost->x);
    
    //generate a cost-sensitive example to update classifiers
    switch(c->cb_type)
    {
      case CB_TYPE_IPS:
        gen_cs_example_ips(*all,*c,ec,c->cb_cs_ld);
        break;
      case CB_TYPE_DM:
        gen_cs_example_dm(*all,*c,ec,c->cb_cs_ld);
        break;
      case CB_TYPE_DR:
        gen_cs_example_dr(*all,*c,ec,c->cb_cs_ld);
        break;
      default:
        std::cerr << "Unknown cb_type specified for contextual bandit learning: " << c->cb_type << ". Exiting." << endl;
        throw exception();
    }

    if (c->cb_type != CB_TYPE_DM)
      {
	ec->ld = &c->cb_cs_ld;
	base.learn(ec);
	ec->ld = ld;
      }
  }

  void init_driver(void*)
  {
    fprintf(stderr, "*estimate* *estimate*                                                avglossreg last pred  last correct\n");
  }

  void print_update(vw& all, cb& c, bool is_test, example *ec)
  {
    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        char label_buf[32];
        if (is_test)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf," known");

        if(!all.holdout_set_off && all.current_pass >= 1)
        {
          if(all.sd->holdout_sum_loss == 0. && all.sd->weighted_holdout_examples == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss/all.sd->weighted_holdout_examples);

          if(all.sd->holdout_sum_loss_since_last_dump == 0. && all.sd->weighted_holdout_examples_since_last_dump == 0.)
            fprintf(stderr, " unknown   ");
          else
	    fprintf(stderr, "%-10.6f " , all.sd->holdout_sum_loss_since_last_dump/all.sd->weighted_holdout_examples_since_last_dump);
        
          fprintf(stderr, "%8ld %8.1f   %s %8lu %8lu   %-10.6f %-10.6f %-10.6f h\n",
	      (long int)all.sd->example_number,
	      all.sd->weighted_examples,
	      label_buf,
              (long unsigned int)ec->final_prediction,
              (long unsigned int)ec->num_features,
              c.avg_loss_regressors,
              c.last_pred_reg,
              c.last_correct_cost);

          all.sd->weighted_holdout_examples_since_last_dump = 0;
          all.sd->holdout_sum_loss_since_last_dump = 0.0;
        }
        else
          fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8lu %8lu   %-10.6f %-10.6f %-10.6f\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                (long unsigned int)ec->final_prediction,
                (long unsigned int)ec->num_features,
                c.avg_loss_regressors,
                c.last_pred_reg,
                c.last_correct_cost);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, cb& c, example* ec)
  {
    CB::label* ld = (CB::label*)ec->ld;

    float loss = 0.;
    if (!CB::is_test_label(ld))
      {//need to compute exact loss
        size_t pred = (size_t)ec->final_prediction;

        float chosen_loss = FLT_MAX;
        if( know_all_cost_example(ld) ) {
          for (cb_class *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
            if (cl->weight_index == pred)
              chosen_loss = cl->x;
          }
        }
        else {
          //we do not know exact cost of each action, so evaluate on generated cost-sensitive example currently stored in cb_cs_ld
          for (CSOAA::wclass *cl = c.cb_cs_ld.costs.begin; cl != c.cb_cs_ld.costs.end; cl ++) {
            if (cl->weight_index == pred)
	      {
		chosen_loss = cl->x;
		if (c.known_cost->weight_index == pred && c.cb_type == CB_TYPE_DM) 
		  chosen_loss += (c.known_cost->x - chosen_loss) / c.known_cost->prob_action;
	      }
          }
        }
        if (chosen_loss == FLT_MAX)
          cerr << "warning: cb predicted an invalid class" << endl;

        loss = chosen_loss;
      }

    if(ec->test_only)
    {
      all.sd->weighted_holdout_examples += ec->global_weight;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += ec->global_weight;
      all.sd->weighted_holdout_examples_since_last_pass += ec->global_weight;
      all.sd->holdout_sum_loss += loss;
      all.sd->holdout_sum_loss_since_last_dump += loss;
      all.sd->holdout_sum_loss_since_last_pass += loss;//since last pass
    }
    else
    {
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      all.sd->weighted_examples += 1.;
      all.sd->total_features += ec->num_features;
      all.sd->example_number++;
    }

    for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
      {
        int f = all.final_prediction_sink[i];
        all.print(f, ec->final_prediction, 0, ec->tag);
      }
  


    print_update(all, c, CB::is_test_label((CB::label*)ec->ld), ec);
  }

  void finish(void* d)
  {
    cb* c=(cb*)d;
    c->cb_cs_ld.costs.delete_v();
  }

  void finish_example(vw& all, void* data, example* ec)
  {
    cb* c = (cb*)data;
    output_example(all, *c, ec);
    VW::finish_example(all, ec);
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    cb* c = (cb*)calloc(1, sizeof(cb));
    c->all = &all;
    c->min_cost = 0.;
    c->max_cost = 1.;
    po::options_description desc("CB options");
    desc.add_options()
      ("cb_type", po::value<string>(), "contextual bandit method to use in {ips,dm,dr}");

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

    uint32_t nb_actions = 0;
    if( vm_file.count("cb") ) { //if loaded options from regressor file already
      nb_actions = (uint32_t)vm_file["cb"].as<size_t>();
      if( vm.count("cb") && (uint32_t)vm["cb"].as<size_t>() != nb_actions )
        std::cerr << "warning: you specified a different number of actions through --cb than the one loaded from regressor. Pursuing with loaded value of: " << nb_actions << endl;
    }
    else {
      nb_actions = (uint32_t)vm["cb"].as<size_t>();
      //append cb with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --cb " << nb_actions;
      all.options_from_file.append(ss.str());
    }
    all.sd->k = nb_actions;

    size_t problem_multiplier = 2;//default for DR
    if (vm.count("cb_type") || vm_file.count("cb_type"))
    {
      std::string type_string;

      if(vm_file.count("cb_type")) {
        type_string = vm_file["cb_type"].as<std::string>();
        if( vm.count("cb_type") && type_string.compare(vm["cb_type"].as<string>()) != 0)
          cerr << "You specified a different --cb_type than the one loaded from regressor file. Pursuing with loaded value of: " << type_string << endl;
      }
      else {
        type_string = vm["cb_type"].as<std::string>();

        all.options_from_file.append(" --cb_type ");
        all.options_from_file.append(type_string);
      }

      if (type_string.compare("dr") == 0) 
        c->cb_type = CB_TYPE_DR;
      else if (type_string.compare("dm") == 0)
	{
	  c->cb_type = CB_TYPE_DM;
	  problem_multiplier = 1;
	}
      else if (type_string.compare("ips") == 0)
	{
	  c->cb_type = CB_TYPE_IPS;
	  problem_multiplier = 1;
	}
      else {
        std::cerr << "warning: cb_type must be in {'ips','dm','dr'}; resetting to dr." << std::endl;
        c->cb_type = CB_TYPE_DR;
      }
    }
    else {
      //by default use doubly robust
      c->cb_type = CB_TYPE_DR;
      all.options_from_file.append(" --cb_type dr");
    }

    *(all.p->lp) = CB::cb_label_parser; 

    learner* l = new learner(c, learn, all.l, problem_multiplier);
    l->set_finish_example(finish_example); 
    l->set_init_driver(init_driver);
    l->set_finish(finish);
    // preserve the increment of the base learner since we are
    // _adding_ to the number of problems rather than multiplying.
    l->increment = all.l->increment; 

    return l;
  }
}
