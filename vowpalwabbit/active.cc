#include "reductions.h"
#include "multiclass.h"
#include "simple_label.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"

using namespace LEARNER;

namespace ACTIVE {
  
  struct active{
    float active_c0;
    vw* all;
  };
    
    float get_active_coin_bias(float k, float avg_loss, float g, float c0)
    {
      float b,sb,rs,sl;
      b=(float)(c0*(log(k+1.)+0.0001)/(k+0.0001));
      sb=sqrt(b);
      avg_loss = min(1.f, max(0.f, avg_loss)); //loss should be in [0,1]

      sl=sqrt(avg_loss)+sqrt(avg_loss+g);
      if (g<=sb*sl+b)
	return 1;
      rs = (sl+sqrt(sl*sl+4*g))/(2*g);
      return b*rs*rs;
    }
  
  float query_decision(active& a, example& ec, float k)
    {
      float bias, avg_loss, weighted_queries;
      if (k<=1.)
	bias=1.;
      else{
	weighted_queries = (float)(a.all->initial_t + a.all->sd->weighted_examples - a.all->sd->weighted_unlabeled_examples);
	avg_loss = (float)(a.all->sd->sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001)));
	bias = get_active_coin_bias(k, avg_loss, ec.revert_weight/k, a.active_c0);
      }
      if(frand48() < bias)
	return 1.f / bias;
      else
	return -1.;
    }

  template <bool is_learn>
  void predict_or_learn_simulation(active& a, learner& base, example& ec) {
    base.predict(ec);
    
    if (is_learn)
      {
	vw& all = *a.all;

	float k = ec.example_t - ec.l.simple.weight;
	ec.revert_weight = all.loss->getRevertingWeight(all.sd, ec.pred.scalar, all.eta/powf(k,all.power_t));
	float importance = query_decision(a, ec, k);

	if(importance > 0){
	  all.sd->queries += 1;
	  ec.l.simple.weight *= importance;
	  base.learn(ec);
	}
	else
	  ec.l.simple.label = FLT_MAX;
      }
  }
  
  template <bool is_learn>
  void predict_or_learn_active(active& a, learner& base, example& ec) {
    if (is_learn)
      base.learn(ec);
    else
      base.predict(ec);

    if (ec.l.simple.label == FLT_MAX) {
      vw& all = *a.all;
      float t = (float)(ec.example_t - all.sd->weighted_holdout_examples);
      
      ec.revert_weight = all.loss->getRevertingWeight(all.sd, ec.pred.scalar, 
						      all.eta/powf(t,all.power_t));
    }
  }

  void active_print_result(int f, float res, float weight, v_array<char> tag)
  {
    if (f >= 0)
      {
	std::stringstream ss;
	char temp[30];
	sprintf(temp, "%f", res);
	ss << temp;
	if(!print_tag(ss, tag))
          ss << ' ';
	if(weight >= 0)
	  {
	    sprintf(temp, " %f", weight);
	    ss << temp;
	  }
	ss << '\n';
	ssize_t len = ss.str().size();
	ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
	if (t != len)
	cerr << "write error" << endl;
      }
  }
  
  void output_and_account_example(vw& all, active& a, example& ec)
  {
    label_data& ld = ec.l.simple;
    
    if(ec.test_only)
      {
	all.sd->weighted_holdout_examples += ld.weight;//test weight seen
	all.sd->weighted_holdout_examples_since_last_dump += ld.weight;
	all.sd->weighted_holdout_examples_since_last_pass += ld.weight;
	all.sd->holdout_sum_loss += ec.loss;
	all.sd->holdout_sum_loss_since_last_dump += ec.loss;
	all.sd->holdout_sum_loss_since_last_pass += ec.loss;//since last pass
      }
    else
      {
	if (ld.label != FLT_MAX)
	  all.sd->weighted_labels += ld.label * ld.weight;
	all.sd->weighted_examples += ld.weight;
	all.sd->sum_loss += ec.loss;
	all.sd->sum_loss_since_last_dump += ec.loss;
	all.sd->total_features += ec.num_features;
	all.sd->example_number++;
      }
    all.print(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
    
    float ai=-1; 
    if(ld.label == FLT_MAX)
      ai=query_decision(a, ec, (float)all.sd->weighted_unlabeled_examples);
    
    all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ld.weight : 0;
    
    for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
      {
	int f = (int)all.final_prediction_sink[i];
	active_print_result(f, ec.pred.scalar, ai, ec.tag);
      }
    
    print_update(all, ec);
  }

  void return_active_example(vw& all, active& a, example& ec)
  {
    output_and_account_example(all, a, ec);
    VW::finish_example(all,&ec);
  }
  
  learner* setup(vw& all, po::variables_map& vm)
  {//parse and set arguments
    active* data = (active*)calloc_or_die(1, sizeof(active));

    po::options_description active_opts("Active Learning options");
    active_opts.add_options()
      ("simulation", "active learning simulation mode")
      ("mellowness", po::value<float>(&(data->active_c0)), "active learning mellowness parameter c_0. Default 8")
      ;

    vm = add_options(all, active_opts);

    data->all=&all;

    //Create new learner
    learner* ret = new learner(data, all.l);
    if (vm.count("simulation"))
      {
	ret->set_learn<active, predict_or_learn_simulation<true> >();
	ret->set_predict<active, predict_or_learn_simulation<false> >();
      }
    else
      {
	all.active = true;
	ret->set_learn<active, predict_or_learn_active<true> >();
	ret->set_predict<active, predict_or_learn_active<false> >();
	ret->set_finish_example<active, return_active_example>();
      }

    return ret;
  }
}
