#include <float.h>

#include "cb.h"
#include "simple_label.h"
#include "example.h"
#include "csoaa.h"
#include "oaa.h"
#include "parse_example.h"
#include "parse_primitives.h"

size_t hashstring (substring s, unsigned long h);

namespace CB
{
  size_t increment = 0;
  size_t cb_type = 0;
  CSOAA::label cb_cs_ld; 
  float avg_loss_regressors = 0.;
  size_t nb_ex_regressors = 0;
  float last_pred_reg = 0.;
  float last_correct_cost = 0.;
  bool first_print_call = true;

  bool know_all_cost_example(CB::label* ld)
  {
    if (ld->costs.index() <= 1) //this means we specified an example where all actions are possible but only specified the cost for the observed action
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
    if (ld->costs.index() == 0)
      return true;
    for (size_t i=0; i<ld->costs.index(); i++)
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
        push(ld->costs, temp);
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
    *(size_t *)c = ld->costs.index();
    c += sizeof(size_t);
    for (size_t i = 0; i< ld->costs.index(); i++)
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
    buf_write(cache, c, sizeof(size_t)+sizeof(cb_class)*ld->costs.index());
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
    ld->costs.erase();
    free(ld->costs.begin);
  }

  v_array<substring> name;

  void parse_label(shared_data* sd, void* v, v_array<substring>& words)
  {
    CB::label* ld = (CB::label*)v;

    for (size_t i = 0; i < words.index(); i++)
      {
        cb_class f;
	tokenize(':', words[i], name);

        if( name.index() < 1 || name.index() > 3 )
        {
          cerr << "malformed cost specification!" << endl;
	  cerr << "terminating." << endl;
          exit(1);
        }

        f.weight_index = hashstring(name[0], 0);
        if (f.weight_index < 1 || f.weight_index > sd->k)
        {
          cerr << "invalid action: " << f.weight_index << endl;
          cerr << "terminating." << endl;
          exit(1);
        }

        f.x = FLT_MAX;
        if(name.index() > 1)
          f.x = float_of_substring(name[1]);

        if ( isnan(f.x))
        {
	  cerr << "error NaN cost for action: ";
	  cerr.write(name[0].begin, name[0].end - name[0].begin);
	  cerr << " terminating." << endl;
	  exit(1);
        }
      
        f.prob_action = .0;
        if(name.index() > 2)
          f.prob_action = float_of_substring(name[2]);

        if ( isnan(f.prob_action))
        {
	  cerr << "error NaN probability for action: ";
	  cerr.write(name[0].begin, name[0].end - name[0].begin);
	  cerr << " terminating." << endl;
	  exit(1);
        }
        
        if( f.prob_action > 1.0 )
        {
          cerr << "invalid probability > 1 specified for an action, resetting to 1." << endl;
          f.prob_action = 1.0;size_t hashstring (substring s, unsigned long h);
        }
        if( f.prob_action < 0.0 )
        {
          cerr << "invalid probability < 0 specified for an action, resetting to 0." << endl;
          f.prob_action = .0;
        }

        push(ld->costs, f);
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

  void (*base_learner)(void*, example*) = NULL; //base learning algorithm (gd,bfgs,etc...) for training regressors of cb
  void (*base_learner_cs)(void*, example*) = NULL; //base learner for cost-sensitive data
  void (*base_finish)(void*) = NULL;
  void (*base_finish_cs)(void*) = NULL;

  void gen_cs_example_ips(void* a, example* ec, CSOAA::label& cs_ld)
  {
    //this implements the inverse propensity score method, where cost are importance weighted by the probability of the chosen action
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;
   
    cb_class* cl_obs = get_observed_cost(ld);
    
    //generate cost-sensitive example
    cs_ld.costs.erase();
    if( ld->costs.index() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for( size_t i = 1; i <= all->sd->k; i++)
      {
        CSOAA::wclass wc;
        wc.x = 0.;
        wc.weight_index = i;
        wc.partial_prediction = 0.;
        if( cl_obs != NULL && i == cl_obs->weight_index )
        {
          wc.x = cl_obs->x / cl_obs->prob_action; //use importance weighted cost for observed action, 0 otherwise 

          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x)*(cl_obs->x) - avg_loss_regressors );
          last_pred_reg = 0;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;
        wc.x = 0.;
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        if( cl_obs != NULL && cl->weight_index == cl_obs->weight_index )
        {
          wc.x = cl_obs->x / cl_obs->prob_action; //use importance weighted cost for observed action, 0 otherwise 

          //ips can be thought as the doubly robust method with a fixed regressor that predicts 0 costs for everything
          //update the loss of this regressor 
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x)*(cl_obs->x) - avg_loss_regressors );
          last_pred_reg = 0;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }

  }

  float get_cost_pred(void* a, example* ec, size_t index)
  {
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;

    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.label = FLT_MAX;
    simple_temp.weight = 0.;

    ec->ld = &simple_temp;
    ec->partial_prediction = 0.;

    size_t desired_increment = increment * (2*index-1);
    
    update_example_indicies(all->audit, ec, desired_increment);
    base_learner(all, ec);
    update_example_indicies(all->audit, ec, -desired_increment);

    ec->ld = ld;

    float cost = ec->partial_prediction;
    ec->partial_prediction = 0.;

    return cost;
  }

  //this function below was a test to see if we save time by carefully organizing the feature offset/regression calls, but seems to be same time as gen_cs_example_dm
  void gen_cs_example_dm2(void* a, example* ec, CSOAA::label& cs_ld)
  {
    //this implements the direct estimation method, where costs are directly specified by the learned regressor.
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;

    cb_class* cl_obs = get_observed_cost(ld);

    size_t desired_increment = 0;
    size_t current_increment = 0;
 
    label_data simple_temp;
    simple_temp.initial = 0.;
    simple_temp.label = FLT_MAX;
    simple_temp.weight = 0.;

    ec->ld = &simple_temp;

    //generate cost sensitive example
    cs_ld.costs.erase();  
    if( ld->costs.index() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions  
      for( size_t i = 1; i <= all->sd->k; i++)
      {
        CSOAA::wclass wc;

        ec->partial_prediction = 0.;
        desired_increment = increment * (2*i-1);
        update_example_indicies(all->audit, ec, desired_increment-current_increment);
        current_increment = desired_increment;
        base_learner(all, ec); 
      
        //get cost prediction for this action
        wc.x = ec->partial_prediction;
        wc.weight_index = i;
        wc.partial_prediction = 0.;

        if( cl_obs != NULL && cl_obs->weight_index == i ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;

        ec->partial_prediction = 0.;
        desired_increment = increment * (2*cl->weight_index-1);
        update_example_indicies(all->audit, ec, desired_increment-current_increment);
        current_increment = desired_increment;
        base_learner(all, ec);
      
        //get cost prediction for this action
        wc.x = ec->partial_prediction;
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;

        if( cl_obs != NULL && cl_obs->weight_index == cl->weight_index ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }

    ec->ld = ld;
    ec->partial_prediction = 0.;
    update_example_indicies(all->audit, ec, -current_increment);
  }

  void gen_cs_example_dm(void* a, example* ec, CSOAA::label& cs_ld)
  {
    //this implements the direct estimation method, where costs are directly specified by the learned regressor.
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;

    cb_class* cl_obs = get_observed_cost(ld);

    //generate cost sensitive example
    cs_ld.costs.erase();  
    if( ld->costs.index() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions  
      for( size_t i = 1; i <= all->sd->k; i++)
      {
        CSOAA::wclass wc;
      
        //get cost prediction for this action
        wc.x = get_cost_pred(a,ec,i);
        wc.weight_index = i;
        wc.partial_prediction = 0.;

        if( cl_obs != NULL && cl_obs->weight_index == i ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;
      
        //get cost prediction for this action
        wc.x = get_cost_pred(a,ec,cl->weight_index);
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;

        if( cl_obs != NULL && cl_obs->weight_index == cl->weight_index ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
        }

        push( cs_ld.costs, wc );
      }
    }
  }

  void gen_cs_example_dr(void* a, example* ec, CSOAA::label& cs_ld)
  {
    //this implements the doubly robust method
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;

    cb_class* cl_obs = get_observed_cost(ld);

    //generate cost sensitive example
    cs_ld.costs.erase();
    if( ld->costs.index() == 1) { //this is a typical example where we can perform all actions
      //in this case generate cost-sensitive example with all actions
      for( size_t i = 1; i <= all->sd->k; i++)
      {
        CSOAA::wclass wc;

        //get cost prediction for this label
        wc.x = get_cost_pred(a,ec,i);
        wc.weight_index = i;
        wc.partial_prediction = 0.;

        //add correction if we observed cost for this action and regressor is wrong
        if( cl_obs != NULL && cl_obs->weight_index == i ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
          wc.x += (cl_obs->x - wc.x) / cl_obs->prob_action;
        }

        push( cs_ld.costs, wc );
      }
    }
    else { //this is an example where we can only perform a subset of the actions
      //in this case generate cost-sensitive example with only allowed actions
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++ )
      {
        CSOAA::wclass wc;

        //get cost prediction for this label
        wc.x = get_cost_pred(a,ec,cl->weight_index);
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;

        //add correction if we observed cost for this action and regressor is wrong
        if( cl_obs != NULL && cl_obs->weight_index == cl->weight_index ) {
          nb_ex_regressors++;
          avg_loss_regressors += (1.0/nb_ex_regressors)*( (cl_obs->x - wc.x)*(cl_obs->x - wc.x) - avg_loss_regressors );
          last_pred_reg = wc.x;
          last_correct_cost = cl_obs->x;
          wc.x += (cl_obs->x - wc.x) / cl_obs->prob_action;
        }

        push( cs_ld.costs, wc );
      }
    }
  }

  void cb_test_to_cs_test_label(void* a, example* ec, CSOAA::label& cs_ld)
  {
    CB::label* ld = (CB::label*)ec->ld;

    cs_ld.costs.erase();
    if(ld->costs.index() > 0)
    {
      //if this is a test example and we specified actions, this means we are only allowed to perform these actions, so copy all actions with their specified costs
      for( cb_class* cl = ld->costs.begin; cl != ld->costs.end; cl++)
      {
        CSOAA::wclass wc;

        wc.x = cl->x;
        wc.weight_index = cl->weight_index;
        wc.partial_prediction = 0.;
        
        push(cs_ld.costs,wc);
      }
    }
  }

  void learn(void* a, example* ec) {
    vw* all = (vw*)a;
    CB::label* ld = (CB::label*)ec->ld;
    float prediction = 1;

    //check if this is a test example where we just want a prediction
    if( CB::is_test_label(ld) )
    {
       //if so just query base cost-sensitive learner
       cb_test_to_cs_test_label(a,ec,cb_cs_ld);

       ec->ld = &cb_cs_ld;
       base_learner_cs(all,ec);
       ec->ld = ld;
       return;
    }

    //now this is a training example
    
    //generate a cost-sensitive example to update classifiers
    switch(cb_type)
    {
      case CB_TYPE_IPS:
        gen_cs_example_ips(a,ec,cb_cs_ld);
        break;
      case CB_TYPE_DM:
        gen_cs_example_dm(a,ec,cb_cs_ld);
        break;
      case CB_TYPE_DR:
        gen_cs_example_dr(a,ec,cb_cs_ld);
        break;
      default:
        std::cerr << "Unknown cb_type specified for contextual bandit learning: " << cb_type << ". Exiting." << endl;
        exit(1);
    }

    //update classifiers with cost-sensitive exemple
    ec->ld = &cb_cs_ld;
    ec->partial_prediction = 0.;
    base_learner_cs(all,ec);
    ec->ld = ld;

    //store current class prediction
    prediction = ec->final_prediction;

    //update our regressors if we are training regressors
    if( cb_type == CB_TYPE_DM || cb_type == CB_TYPE_DR )
    {
      cb_class* cl_obs = get_observed_cost(ld);

      if( cl_obs != NULL )
      {
        size_t i = cl_obs->weight_index;
	
        label_data simple_temp;
	simple_temp.initial = 0.;
	simple_temp.label = cl_obs->x;
	simple_temp.weight = 1.;
        //std::cerr << "Updating regressor for class " << i << " to predict cost " << simple_temp.label << endl;

	ec->ld = &simple_temp;

        size_t desired_increment = increment * (2*i-1);
        update_example_indicies(all->audit, ec, desired_increment);
        ec->partial_prediction = 0.;
        base_learner(all, ec);
        update_example_indicies(all->audit, ec, -desired_increment);

        ec->ld = ld;
      }
    }

    ec->final_prediction = prediction;
  }

  void print_update(vw& all, bool is_test, example *ec)
  {
    if( first_print_call )
    {
      fprintf(stderr, "*estimate* *estimate*                                                avglossreg last pred  last correct\n");
      first_print_call = false;
    }

    if (all.sd->weighted_examples > all.sd->dump_interval && !all.quiet && !all.bfgs)
      {
        char label_buf[32];
        if (is_test)
          strcpy(label_buf," unknown");
        else
          sprintf(label_buf," known");

        fprintf(stderr, "%-10.6f %-10.6f %8ld %8.1f   %s %8lu %8lu   %-10.6f %-10.6f %-10.6f\n",
                all.sd->sum_loss/all.sd->weighted_examples,
                all.sd->sum_loss_since_last_dump / (all.sd->weighted_examples - all.sd->old_weighted_examples),
                (long int)all.sd->example_number,
                all.sd->weighted_examples,
                label_buf,
                *(OAA::prediction_t*)&ec->final_prediction,
                (long unsigned int)ec->num_features,
                avg_loss_regressors,
                last_pred_reg,
                last_correct_cost);
     
        all.sd->sum_loss_since_last_dump = 0.0;
        all.sd->old_weighted_examples = all.sd->weighted_examples;
        all.sd->dump_interval *= 2;
      }
  }

  void output_example(vw& all, example* ec)
  {
    CB::label* ld = (CB::label*)ec->ld;
    all.sd->weighted_examples += 1.;
    all.sd->total_features += ec->num_features;
    float loss = 0.;
    if (!CB::is_test_label(ld))
      {//need to compute exact loss
        size_t pred = *(OAA::prediction_t*)&ec->final_prediction;
        //size_t min_pred = pred;

        float chosen_loss = FLT_MAX;
        float min = FLT_MAX;
        if( know_all_cost_example(ld) ) {
          for (cb_class *cl = ld->costs.begin; cl != ld->costs.end; cl ++) {
            if (cl->weight_index == pred)
              chosen_loss = cl->x;
            if (cl->x < min)
            {
              min = cl->x;
              //min_pred = cl->weight_index;
            }
          }
        }
        else {
          //we do not know exact cost of each action, so evaluate on generated cost-sensitive example currently stored in cb_cs_ld
          for (CSOAA::wclass *cl = cb_cs_ld.costs.begin; cl != cb_cs_ld.costs.end; cl ++) {
            if (cl->weight_index == pred)
              chosen_loss = cl->x;
            if (cl->x < min)
            {
              min = cl->x;
              //min_pred = cl->weight_index;
            }
          }
        }
        if (chosen_loss == FLT_MAX)
          cerr << "warning: cb predicted an invalid class" << endl;

        loss = chosen_loss - min;
        //cerr << "pred: " << pred << " min_pred: " << min_pred << " loss: " << chosen_loss << " min_loss: " << min << " regret: " << loss << endl;
      }

    all.sd->sum_loss += loss;
    all.sd->sum_loss_since_last_dump += loss;
  
    for (size_t i = 0; i<all.final_prediction_sink.index(); i++)
      {
        int f = all.final_prediction_sink[i];
        all.print(f, *(OAA::prediction_t*)&ec->final_prediction, 0, ec->tag);
      }
  
    all.sd->example_number++;

    print_update(all, CB::is_test_label((CB::label*)ec->ld), ec);
  }

  void finish(void* a)
  {
    vw* all = (vw*)a;
    cb_cs_ld.costs.erase();
    if (cb_cs_ld.costs.begin != NULL)
      free(cb_cs_ld.costs.begin);
    //cerr << "base finish cs" << endl;
    if(base_finish_cs != NULL)
      base_finish_cs(all);
    //cerr << "base finish" << endl;
    if(base_finish != NULL)
      base_finish(all);
    //cerr << "all finished" << endl;
  }

  void drive_cb(void* in)
  {
    vw*all = (vw*)in;
    example* ec = NULL;
    while ( true )
    {
      if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
      {
        learn(all, ec);
        output_example(*all, ec);
	VW::finish_example(*all, ec);
      }
      else if (parser_done(all->p))
      {
        //finish(all); this is already called by main function
        return;
      }
    }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file, size_t s)
  {
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

      if (type_string.compare("dr") == 0) cb_type = CB_TYPE_DR;
      else if (type_string.compare("dm") == 0) cb_type = CB_TYPE_DM;
      else if (type_string.compare("ips") == 0) cb_type = CB_TYPE_IPS;
      else
      {
        std::cerr << "warning: cb_type must be in {'ips','dm','dr'}; resetting to dr." << std::endl;
        cb_type = CB_TYPE_DR;
      }
    }
    else {
      //by default use doubly robust
      cb_type = CB_TYPE_DR;
      all.options_from_file.append(" --cb_type dr");
    }

    *(all.p->lp) = CB::cb_label_parser; 

    all.sd->k = s;
    all.driver = drive_cb;

    //this parsing is done after the cost-sensitive parsing, so all.learn currently points to the base cs learner
    //and all.base_learn points to gd/bfgs base learner
    base_learner_cs = all.learn;
    all.base_cs_learn = all.learn;
    base_learner = all.base_learn;

    all.learn = learn;
    base_finish_cs = all.finish;
    all.base_cs_finish = all.finish;
    base_finish = all.base_finish;
    all.finish = finish;
    increment = (all.length()/all.sd->k/2) * all.stride;
  }


}
