#include <errno.h>
#include "reductions.h"
#include "rand48.h"
#include "float.h"
#include "vw.h"
#include "active.h"
#include "vw_exception.h"

using namespace LEARNER;
using namespace std;
float get_active_coin_bias(float k, float avg_loss, float g, float c0)
{ float b,sb,rs,sl;
  b=(float)(c0*(log(k+1.)+0.0001)/(k+0.0001));
  sb=sqrt(b);
  avg_loss = min(1.f, max(0.f, avg_loss)); //loss should be in [0,1]

  sl=sqrt(avg_loss)+sqrt(avg_loss+g);
  if (g<=sb*sl+b)
    return 1;
  rs = (sl+sqrt(sl*sl+4*g))/(2*g);
  return b*rs*rs;
}

float query_decision(active& a, float ec_revert_weight, float k)
{ float bias, avg_loss, weighted_queries;
  if (k<=1.)
    bias=1.;
  else
  { weighted_queries = (float)a.all->sd->weighted_labeled_examples;
    avg_loss = (float)(a.all->sd->sum_loss/k + sqrt((1.+0.5*log(k))/(weighted_queries+0.0001)));
    bias = get_active_coin_bias(k, avg_loss, ec_revert_weight/k, a.active_c0);
  }
  if(merand48(a.all->random_state) < bias)
    return 1.f / bias;
  else
    return -1.;
}

template <bool is_learn>
void predict_or_learn_simulation(active& a, base_learner& base, example& ec)
{ base.predict(ec);

  if (is_learn)
  { vw& all = *a.all;

    float k = (float)all.sd->t;
    float threshold = 0.f;

    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
    float importance = query_decision(a, ec.confidence, k);

    if(importance > 0)
    { all.sd->queries += 1;
      ec.weight *= importance;
      base.learn(ec);
    }
    else
    { ec.l.simple.label = FLT_MAX;
      ec.weight = 0.f;
    }
  }
}

template <bool is_learn>
void predict_or_learn_active(active& a, base_learner& base, example& ec)
{ if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  if (ec.l.simple.label == FLT_MAX)
  { float threshold = (a.all->sd->max_label + a.all->sd->min_label) * 0.5f;
    ec.confidence = fabsf(ec.pred.scalar - threshold) / base.sensitivity(ec);
  }
}

void active_print_result(int f, float res, float weight, v_array<char> tag)
{ if (f >= 0)
  { std::stringstream ss;
    char temp[30];
    sprintf(temp, "%f", res);
    ss << temp;
    if(!print_tag(ss, tag))
      ss << ' ';
    if(weight >= 0)
    { sprintf(temp, " %f", weight);
      ss << temp;
    }
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
      cerr << "write error: " << strerror(errno) << endl;
  }
}

void output_and_account_example(vw& all, active& a, example& ec)
{ label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ld.label != FLT_MAX, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ld.label * ec.weight;
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  float ai=-1;
  if(ld.label == FLT_MAX)
    ai=query_decision(a, ec.confidence, (float)all.sd->weighted_unlabeled_examples);

  all.print(all.raw_prediction, ec.partial_prediction, -1, ec.tag);
  for (size_t i = 0; i<all.final_prediction_sink.size(); i++)
  { int f = (int)all.final_prediction_sink[i];
    active_print_result(f, ec.pred.scalar, ai, ec.tag);
  }

  print_update(all, ec);
}

void return_active_example(vw& all, active& a, example& ec)
{ output_and_account_example(all, a, ec);
  VW::finish_example(all,&ec);
}

base_learner* active_setup(vw& all)
{ //parse and set arguments
  if(missing_option(all, false, "active", "enable active learning")) return nullptr;
  new_options(all, "Active Learning options")
  ("simulation", "active learning simulation mode")
  ("mellowness", po::value<float>(), "active learning mellowness parameter c_0. Default 8");
  add_options(all);

  active& data = calloc_or_throw<active>();
  data.active_c0 = 8;
  data.all=&all;

  if (all.vm.count("mellowness"))
    data.active_c0 = all.vm["mellowness"].as<float>();

  if (count(all.args.begin(), all.args.end(), "--lda") != 0)
  { free(&data);
    THROW("error: you can't combine lda and active learning");
  }

  base_learner* base = setup_base(all);

  //Create new learner
  learner<active>* l;
  if (all.vm.count("simulation"))
    l = &init_learner(&data, base, predict_or_learn_simulation<true>,
                      predict_or_learn_simulation<false>);
  else
  { all.active = true;
    l = &init_learner(&data, base, predict_or_learn_active<true>,
                      predict_or_learn_active<false>);
    l->set_finish_example(return_active_example);
  }

  return make_base(*l);
}
