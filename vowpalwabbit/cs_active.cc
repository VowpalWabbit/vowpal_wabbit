#include <errno.h>
#include "reductions.h"
#include "v_hashmap.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"
#include "vw_exception.h"
#include "csoaa.h"

using namespace LEARNER;
using namespace std;
using namespace COST_SENSITIVE;

struct cs_active
{ // active learning algorithm parameters
  float c0; // mellowness controlling the width of the set of good functions
  float c1; // multiplier on the threshold for the cost range test
  float cost_max; // max cost
  float cost_min; // min cost

  uint32_t num_classes;
  size_t t;

  vw* all;//statistics, loss
  LEARNER::base_learner* l;
};

// test if the cost range for label i is large
bool is_range_large(cs_active& cs_a, base_learner& base, example& ec, uint32_t i)
{ if (cs_a.t == 1) return true;
  float t = (float)cs_a.t; // ec.example_t;  // current round
  float t_prev = t-1.; // ec.weight; // last round

  float eta = cs_a.c1*(cs_a.cost_max - cs_a.cost_min)/sqrt(t); // threshold on cost range
  float delta = cs_a.c0*log((float)cs_a.num_classes)*log(t_prev)*pow(cs_a.cost_max-cs_a.cost_min,2);  // threshold on empirical loss difference
  
  float cost_pred_u = min(ec.pred.scalar+eta, cs_a.cost_max);
  float cost_pred_l = max(ec.pred.scalar-eta, cs_a.cost_min);

  // Compute the minimum weights required to increase/decrease prediction by eta
  ec.l.simple.label = cs_a.cost_max;
  float sensitivity_u = base.sensitivity(ec, i-1);
  float w_u = (cost_pred_u-ec.pred.scalar)/sensitivity_u;
 
  ec.l.simple.label = cs_a.cost_min;
  float sensitivity_l = base.sensitivity(ec, i-1);
  float w_l = (ec.pred.scalar-cost_pred_l)/sensitivity_l; 

  // Compute upper bound on the empirical loss difference
  // Assume squared loss is used
  float loss_delta_upper_bnd = max(w_u*pow(ec.pred.scalar-cs_a.cost_max,2),w_l*pow(ec.pred.scalar-cs_a.cost_min,2));
  if (isnan(loss_delta_upper_bnd))
     cerr << "Warning: loss_delta_upper_bnd is nan!" << endl;

  bool result = (loss_delta_upper_bnd <= delta) || isnan(loss_delta_upper_bnd);

  //if (i == 1)
  //   cout << "is_range_large | t=" << t << " t_prev=" << t_prev << " eta=" << eta << " delta=" << delta << " pred=" << ec.pred.scalar << " cost_pred_u=" << cost_pred_u << " cost_pred_l=" << cost_pred_l << " sensitivity_u=" << sensitivity_u << " w_u=" << w_u << " sensitivity_l=" << sensitivity_l << " w_l=" << w_l << " loss_delta_upper_bnd=" << loss_delta_upper_bnd << " result=" << result << endl;
  
  return result;
}

template<bool is_learn, bool is_simulation>
inline void inner_loop(cs_active& cs_a, base_learner& base, example& ec, uint32_t i, float cost,
                       uint32_t& prediction, float& score, float& partial_prediction, bool& pred_is_certain)
{ base.predict(ec, i-1);
  if (is_learn)
  { vw& all = *cs_a.all;
     
    if (is_simulation)
    { // in simulation mode, query if cost range for this label is large, and use predicted cost otherwise.
      if(is_range_large(cs_a,base,ec,i))
      { ec.l.simple.label = cost;
        all.sd->queries += 1;
      }
      else 
      { ec.l.simple.label = ec.pred.scalar;
      }
      //cerr << "\t[" << i << ":" << ec.l.simple.label << " (truth=" << cost << ")]" << endl;
    }
    else
    { // in reduction mode, always given a cost
      if (pred_is_certain)
        ec.l.simple.label = ec.pred.scalar;
      else
        ec.l.simple.label = cost;
      //cerr << "\t[" << i << ":" << ec.l.simple.label << " (truth=" << cost << ")]" << (pred_is_certain ? '*' : ' ') << endl;
    }

    //all.set_minmax(all.sd, ec.l.simple.label);

    ec.weight = (cost == FLT_MAX) ? 0.f : 1.f;
    //cs_a.t ++;
    base.learn(ec, i-1);
  }
  else if (!is_simulation) 
  {//reduction mode, need to tell upper layer whether this prediction was made with confidence
    pred_is_certain = (!is_range_large(cs_a,base,ec,i));
  }
  
  partial_prediction = ec.partial_prediction;
  if (ec.partial_prediction < score || (ec.partial_prediction == score && i < prediction))
  { score = ec.partial_prediction;
    prediction = i;
  }
  add_passthrough_feature(ec, i, ec.partial_prediction);
}

template <bool is_learn, bool is_simulation>
void predict_or_learn(cs_active& data, base_learner& base, example& ec)
{ //cerr << "------------- passthrough" << endl;
  COST_SENSITIVE::label ld = ec.l.cs;
  uint32_t prediction = 1;
  float score = FLT_MAX;
  ec.l.simple = { 0., 0., 0. };
  if (ld.costs.size() > 0)
  { for (COST_SENSITIVE::wclass& cl : ld.costs) {
      inner_loop<is_learn,is_simulation>(data, base, ec, cl.class_index, cl.x, prediction, score, cl.partial_prediction, cl.pred_is_certain);
      //cl.pred_is_certain = frand48() * 100 < ec.example_t;
      //cerr << "cl.pred_is_certain=" << cl.pred_is_certain << endl;
    }
    ec.partial_prediction = score;
    if(is_learn)
      data.t++;    
  }
  else
  { float temp;
    bool temp2;
    for (uint32_t i = 1; i <= data.num_classes; i++)
      inner_loop<false,is_simulation>(data, base, ec, i, FLT_MAX, prediction, score, temp, temp2);
  }

  ec.pred.multiclass = prediction;
  ec.l.cs = ld;
}

void finish_example(vw& all, cs_active& cs_a, example& ec)
{ CSOAA::finish_example(all, *(CSOAA::csoaa*)&cs_a, ec);
}

base_learner* cs_active_setup(vw& all)
{ //parse and set arguments
  if(missing_option<size_t, true>(all, "cs_active", "Cost-sensitive active learning with <k> costs"))
    return nullptr;

  new_options(all, "cost-sensitive active Learning options")
  ("simulation", "cost-sensitive active learning simulation mode")
  ("mellowness",po::value<float>(),"mellowness parameter c_0. Default 8.")
  ("range_c", po::value<float>(),"parameter controlling the threshold for per-label cost uncertainty. Default 0.5.")
  ("cost_max",po::value<float>(),"cost upper bound. Default 1.")
  ("cost_min",po::value<float>(),"cost lower bound. Default 0.");
  add_options(all);

  cs_active& data = calloc_or_throw<cs_active>();
  
  data.num_classes = (uint32_t)all.vm["cs_active"].as<size_t>();
  data.c0 = 0.1;
  data.c1 = 0.5;
  data.all = &all;
  data.cost_max = 1.f;
  data.cost_min = 0.f;
  data.t = 1;
 
  
  if(all.vm.count("mellowness"))
  { data.c0 = all.vm["mellowness"].as<float>();
  }

  if(all.vm.count("range_c"))
  { data.c1 = all.vm["range_c"].as<float>();
  }
  
  if(all.vm.count("cost_max"))
  { data.cost_max = all.vm["cost_max"].as<float>();
  }
  
  if(all.vm.count("cost_min"))
  { data.cost_min = all.vm["cost_min"].as<float>();
  }

  string loss_function = all.vm["loss_function"].as<string>();
  if (loss_function.compare("squared") != 0)
  { free(&data);
    THROW("error: you can't use non-squared loss with cs_active");
  }
  
  if (count(all.args.begin(), all.args.end(),"--lda") != 0)
  { free(&data);
    THROW("error: you can't combine lda and active learning");
  }


  if (count(all.args.begin(), all.args.end(),"--active") != 0)
  { free(&data);
    THROW("error: you can't use --cs_active and --active at the same time");
  }
  
  if (count(all.args.begin(), all.args.end(),"--active_cover") != 0)
  { free(&data);
    THROW("error: you can't use --cs_active and --active_cover at the same time");
  }
  
  if (count(all.args.begin(), all.args.end(),"--csoaa") != 0)
  { free(&data);
    THROW("error: you can't use --cs_active and --csoaa at the same time");
  }

  learner<cs_active>* l; 

  if (all.vm.count("simulation"))
     l = &init_learner(&data, setup_base(all), predict_or_learn<true,true>, predict_or_learn<false,true>, data.num_classes);
  else
     l = &init_learner(&data, setup_base(all), predict_or_learn<true,false>, predict_or_learn<false,false>, data.num_classes);

  all.set_minmax(all.sd,data.cost_max);
  all.set_minmax(all.sd,data.cost_min);

  all.p->lp = cs_label; // assigning the label parser
  l->set_finish_example(finish_example);
  base_learner* b = make_base(*l);
  all.cost_sensitive = b;
  return b;
}

