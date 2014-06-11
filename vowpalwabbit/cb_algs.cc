/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>

#include "reductions.h"
#include "cost_sensitive.h"
#include "cb.h"
#include "cb_algs.h"
#include "simple_label.h"

using namespace LEARNER;

#define CB_TYPE_DR 0
#define CB_TYPE_DM 1
#define CB_TYPE_IPS 2

using namespace CB;

namespace CB_ALGS
{
  struct cb {
    size_t cb_type;
    COST_SENSITIVE::label cb_cs_ld; 
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
      if (cl->cost == FLT_MAX)
        return false;

    return true;
  }

  bool is_test_label(CB::label* ld)
  {
    if (ld->costs.size() == 0)
      return true;
    for (size_t i=0; i<ld->costs.size(); i++)
      if (FLT_MAX != ld->costs[i].cost && ld->costs[i].probability > 0.)
        return false;
    return true;
  }
  
  inline bool observed_cost(cb_class* cl)
  {
    //cost observed for this action if it has non zero probability and cost != FLT_MAX
    return (cl != NULL && cl->cost != FLT_MAX && cl->probability > .0);
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

  void gen_cs_example_ips(vw& all, cb& c, example& ec, COST_SENSITIVE::label& cs_ld)
  {//this implements the inverse propensity score method, where cost are importance weighted by the probability of the chosen action
    CB::label* ld = (CB::label*)ec.ld;
   
    //generate cost-sensitive example
    cs_ld.costs.erase();
    if( ld->costs.size() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for(uint32_t i = 1; i <= all.sd->k; i++)
      {
        COST_SENSITIVE::wclass wc;
        wc.wap_value = 0.;
        wc.x = 0.;
        wc.class_index = i;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        if( c.known_cost != NULL && i == c.known_cost->action )
        {
          wc.x = c.known_cost->cost / c.known_cost->probability; //use importance weighted cost for observed action, 0 otherwise 
          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->cost)*(c.known_cost->cost) - c.avg_loss_regressors );
          c.last_pred_reg = 0;
          c.last_correct_cost = c.known_cost->cost;
        }

        cs_ld.costs.push_back(wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        COST_SENSITIVE::wclass wc;
        wc.wap_value = 0.;
        wc.x = 0.;
        wc.class_index = cl->action;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        if( c.known_cost != NULL && cl->action == c.known_cost->action )
        {
          wc.x = c.known_cost->cost / c.known_cost->probability; //use importance weighted cost for observed action, 0 otherwise 

          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->cost)*(c.known_cost->cost) - c.avg_loss_regressors );
          c.last_pred_reg = 0;
          c.last_correct_cost = c.known_cost->cost;
        }

        cs_ld.costs.push_back( wc );
      }
    }

  }

  template <bool is_learn>
  void gen_cs_example_dm(vw& all, cb& c, example& ec, COST_SENSITIVE::label& cs_ld)
  {
    //this implements the direct estimation method, where costs are directly specified by the learned regressor.
    CB::label* ld = (CB::label*)ec.ld;

    float min = FLT_MAX;
    uint32_t argmin = 1;
    //generate cost sensitive example
    cs_ld.costs.erase();  
    if( ld->costs.size() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions  
      for(uint32_t i = 1; i <= all.sd->k; i++)
      {
        COST_SENSITIVE::wclass wc;
        wc.wap_value = 0.;
      
        //get cost prediction for this action
        wc.x = get_cost_pred<is_learn>(all, c.known_cost, ec, i, 0);
	if (wc.x < min)
	  {
	    min = wc.x;
	    argmin = i;
	  }

        wc.class_index = i;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        if( c.known_cost != NULL && c.known_cost->action == i ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->cost;
        }

        cs_ld.costs.push_back( wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        COST_SENSITIVE::wclass wc;
        wc.wap_value = 0.;
      
        //get cost prediction for this action
        wc.x = get_cost_pred<is_learn>(all, c.known_cost, ec, cl->action, 0);
	if (wc.x < min || (wc.x == min && cl->action < argmin))
	  {
	    min = wc.x;
	    argmin = cl->action;
	  }

        wc.class_index = cl->action;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;

        if( c.known_cost != NULL && c.known_cost->action == cl->action ) {
          c.nb_ex_regressors++;
          c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors );
          c.last_pred_reg = wc.x;
          c.last_correct_cost = c.known_cost->cost;
        }

        cs_ld.costs.push_back( wc );
      }
    }
    
    ld->prediction = argmin;
  }

  template <bool is_learn>
  void gen_cs_label(vw& all, cb& c, example& ec, COST_SENSITIVE::label& cs_ld, uint32_t label)
  {
    COST_SENSITIVE::wclass wc;
    wc.wap_value = 0.;
    
    //get cost prediction for this label
    wc.x = get_cost_pred<is_learn>(all, c.known_cost, ec, label, all.sd->k);
    wc.class_index = label;
    wc.partial_prediction = 0.;
    wc.wap_value = 0.;
    
    //add correction if we observed cost for this action and regressor is wrong
    if( c.known_cost != NULL && c.known_cost->action == label ) {
      c.nb_ex_regressors++;
      c.avg_loss_regressors += (1.0f/c.nb_ex_regressors)*( (c.known_cost->cost - wc.x)*(c.known_cost->cost - wc.x) - c.avg_loss_regressors );
      c.last_pred_reg = wc.x;
      c.last_correct_cost = c.known_cost->cost;
      wc.x += (c.known_cost->cost - wc.x) / c.known_cost->probability;
    }
    cs_ld.costs.push_back( wc );
  }

  template <bool is_learn>
  void gen_cs_example_dr(vw& all, cb& c, example& ec, COST_SENSITIVE::label& cs_ld)
  {//this implements the doubly robust method
    CB::label* ld = (CB::label*)ec.ld;
    
    //generate cost sensitive example
    cs_ld.costs.erase();
    if( ld->costs.size() == 1) //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for(uint32_t i = 1; i <= all.sd->k; i++)
	gen_cs_label<is_learn>(all, c, ec, cs_ld, i);
    else  //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
	gen_cs_label<is_learn>(all, c, ec, cs_ld, cl->action);
  }

  void cb_test_to_cs_test_label(vw& all, example& ec, COST_SENSITIVE::label& cs_ld)
  {
    CB::label* ld = (CB::label*)ec.ld;

    cs_ld.costs.erase();
    if(ld->costs.size() > 0)
    {
      //if this is a test example and we specified actions, this means we are only allowed to perform these actions, so copy all actions with their specified costs
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++)
      {
        COST_SENSITIVE::wclass wc;
        wc.wap_value = 0.;

        wc.x = cl->cost;
        wc.class_index = cl->action;
        wc.partial_prediction = 0.;
        wc.wap_value = 0.;
        
        cs_ld.costs.push_back(wc);
      }
    }
    else
      {
	for (uint32_t i = 0; i < all.sd->k; i++)
	  {
	    COST_SENSITIVE::wclass wc;
	    wc.wap_value = 0.;
	    
	    wc.x = FLT_MAX;
	    wc.class_index = i+1;
	    wc.partial_prediction = 0.;
	    wc.wap_value = 0.;
	    
	    cs_ld.costs.push_back(wc);
	  }
      }
  }

  template <bool is_learn>
  void predict_or_learn(cb& c, learner& base, example& ec) {
    vw* all = c.all;
    CB::label* ld = (CB::label*)ec.ld;

     //check if this is a test example where we just want a prediction
    if( is_test_label(ld) )
    {
      //if so just query base cost-sensitive learner
      cb_test_to_cs_test_label(*all,ec,c.cb_cs_ld);

      ec.ld = &c.cb_cs_ld;
      base.predict(ec);
      ld->prediction = c.cb_cs_ld.prediction;

      ec.ld = ld;
      for (size_t i=0; i<ld->costs.size(); i++)
        ld->costs[i].partial_prediction = c.cb_cs_ld.costs[i].partial_prediction;
      return;
    }

    //now this is a training example
    c.known_cost = get_observed_cost(ld);
    c.min_cost = min (c.min_cost, c.known_cost->cost);
    c.max_cost = max (c.max_cost, c.known_cost->cost);
    
    //generate a cost-sensitive example to update classifiers
    switch(c.cb_type)
    {
      case CB_TYPE_IPS:
        gen_cs_example_ips(*all,c,ec,c.cb_cs_ld);
        break;
      case CB_TYPE_DM:
        gen_cs_example_dm<is_learn>(*all,c,ec,c.cb_cs_ld);
        break;
      case CB_TYPE_DR:
        gen_cs_example_dr<is_learn>(*all,c,ec,c.cb_cs_ld);
        break;
      default:
        std::cerr << "Unknown cb_type specified for contextual bandit learning: " << c.cb_type << ". Exiting." << endl;
        throw exception();
    }

    if (c.cb_type != CB_TYPE_DM)
      {
	ec.ld = &c.cb_cs_ld;

	if (is_learn)
	  base.learn(ec);
	else
	  base.predict(ec);

	ld->prediction = c.cb_cs_ld.prediction;
        for (size_t i=0; i<ld->costs.size(); i++)
          ld->costs[i].partial_prediction = c.cb_cs_ld.costs[i].partial_prediction;
	ec.ld = ld;
      }
  }

  void init_driver(cb&)
  {
    fprintf(stderr, "*estimate* *estimate*                                                avglossreg last pred  last correct\n");
  }

  void print_update(vw& all, cb& c, bool is_test, example& ec)
  {
    if (all.sd->weighted_examples >= all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        char label_buf[32];
        if (is_test)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf," known");

	CB::label& ld = *(CB::label*)ec.ld;
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
              (long unsigned int)ld.prediction,
              (long unsigned int)ec.num_features,
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
                (long unsigned int)ld.prediction,
                (long unsigned int)ec.num_features,
                c.avg_loss_regressors,
                c.last_pred_reg,
                c.last_correct_cost);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
	fflush(stderr);
        VW::update_dump_interval(all);
      }
  }

  void output_example(vw& all, cb& c, example& ec)
  {
    CB::label* ld = (CB::label*)ec.ld;

    float loss = 0.;
    if (!is_test_label(ld))
      {//need to compute exact loss
        size_t pred = (size_t)ld->prediction;

        float chosen_loss = FLT_MAX;
        if( know_all_cost_example(ld) ) {
          for (cb_class *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
            if (cl->action == pred)
              chosen_loss = cl->cost;
          }
        }
        else {
          //we do not know exact cost of each action, so evaluate on generated cost-sensitive example currently stored in cb_cs_ld
          for (COST_SENSITIVE::wclass *cl = c.cb_cs_ld.costs.begin; cl != c.cb_cs_ld.costs.end; cl ++) {
            if (cl->class_index == pred)
	      {
		chosen_loss = cl->x;
		if (c.known_cost->action == pred && c.cb_type == CB_TYPE_DM) 
		  chosen_loss += (c.known_cost->cost - chosen_loss) / c.known_cost->probability;
	      }
          }
        }
        if (chosen_loss == FLT_MAX)
          cerr << "warning: cb predicted an invalid class" << endl;

        loss = chosen_loss;
      }

    if(ec.test_only)
    {
      all.sd->weighted_holdout_examples += 1.;//test weight seen
      all.sd->weighted_holdout_examples_since_last_dump += 1.;
      all.sd->weighted_holdout_examples_since_last_pass += 1.;
      all.sd->holdout_sum_loss += loss;
      all.sd->holdout_sum_loss_since_last_dump += loss;
      all.sd->holdout_sum_loss_since_last_pass += loss;//since last pass
    }
    else
    {
      all.sd->sum_loss += loss;
      all.sd->sum_loss_since_last_dump += loss;
      all.sd->weighted_examples += 1.;
      all.sd->total_features += ec.num_features;
      all.sd->example_number++;
    }

    for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
      {
        int f = all.final_prediction_sink[i];
        all.print(f, (float)ld->prediction, 0, ec.tag);
      }
  


    print_update(all, c, is_test_label((CB::label*)ec.ld), ec);
  }

  void finish(cb& c)
  {
    c.cb_cs_ld.costs.delete_v();
  }

  void finish_example(vw& all, cb& c, example& ec)
  {
    output_example(all, c, ec);
    VW::finish_example(all, &ec);
  }

  learner* setup(vw& all, po::variables_map& vm)
  {
    cb* c = (cb*)calloc_or_die(1, sizeof(cb));
    c->all = &all;
    c->min_cost = 0.;
    c->max_cost = 1.;

    uint32_t nb_actions = (uint32_t)vm["cb"].as<size_t>();
    //append cb with nb_actions to file_options so it is saved to regressor later

    po::options_description cb_opts("CB options");
    cb_opts.add_options()
      ("cb_type", po::value<string>(), "contextual bandit method to use in {ips,dm,dr}");

    vm = add_options(all, cb_opts);

    std::stringstream ss;
    ss << " --cb " << nb_actions;
    all.file_options.append(ss.str());

    all.sd->k = nb_actions;

    size_t problem_multiplier = 2;//default for DR
    if (vm.count("cb_type"))
    {
      std::string type_string;

      type_string = vm["cb_type"].as<std::string>();
      
      all.file_options.append(" --cb_type ");
      all.file_options.append(type_string);

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
      all.file_options.append(" --cb_type dr");
    }

    all.p->lp = CB::cb_label; 

    learner* l = new learner(c, all.l, problem_multiplier);
    l->set_learn<cb, predict_or_learn<true> >();
    l->set_predict<cb, predict_or_learn<false> >();
    l->set_finish_example<cb,finish_example>(); 
    l->set_init_driver<cb,init_driver>();
    l->set_finish<cb,finish>();
    // preserve the increment of the base learner since we are
    // _adding_ to the number of problems rather than multiplying.
    l->increment = all.l->increment; 

    return l;
  }
}
