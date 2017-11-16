/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include <string>
#include "correctedMath.h"
#include "gd.h"

using namespace std;
using namespace LEARNER;

#define W_XT 0   // current parameter
#define W_ZT 1   // in proximal is "accumulated z(t) = z(t-1) + g(t) + sigma*w(t)", in general is the dual weight vector
#define W_G2 2   // accumulated gradient information
#define W_MX 3   // maximum absolute value

struct update_data
{ float update;
  float ftrl_alpha;
  float ftrl_beta;
  float l1_lambda;
  float l2_lambda;
  float predict;
};

struct ftrl
{ vw* all; //features, finalize, l1, l2,
  float ftrl_alpha;
  float ftrl_beta;
  struct update_data data;
  size_t no_win_counter;
  size_t early_stop_thres;
};

struct uncertainty
{ float pred;
  float score;
  ftrl& b;
  uncertainty(ftrl& ftrlb) : b(ftrlb)
  { pred = 0;
    score = 0;
  }
};

inline float sign(float w) { if (w < 0.) return -1.; else  return 1.;}

inline void predict_with_confidence(uncertainty& d, const float fx, float& fw)
{ float* w = &fw;
  d.pred += w[W_XT] * fx;
  float sqrtf_ng2 = sqrtf(w[W_G2]);
  float uncertain = ( (d.b.data.ftrl_beta+sqrtf_ng2)/d.b.data.ftrl_alpha +d.b.data.l2_lambda);
  d.score += (1/uncertain)*sign(fx);
}

float sensitivity(ftrl& b, base_learner& base, example& ec)
{ uncertainty uncetain(b);
  GD::foreach_feature<uncertainty, predict_with_confidence>(*(b.all), ec, uncetain);
  return uncetain.score;
}
template<bool audit>
void predict(ftrl& b, base_learner&, example& ec)
{ ec.partial_prediction = GD::inline_predict(*b.all, ec);
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
  if (audit)
    GD::print_audit_features(*(b.all), ec);
}

template<bool audit>
void multipredict(ftrl& b, base_learner&, example& ec, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)
{ vw& all = *b.all;
  for (size_t c=0; c<count; c++)
    pred[c].scalar = ec.l.simple.initial;
  if (b.all->weights.sparse)
    {
      GD::multipredict_info<sparse_parameters> mp = { count, step, pred, all.weights.sparse_weights, (float)all.sd->gravity };
      GD::foreach_feature<GD::multipredict_info<sparse_parameters>, uint64_t, GD::vec_add_multipredict>(all, ec, mp);
    }
  else
    {
      GD::multipredict_info<dense_parameters> mp = { count, step, pred, all.weights.dense_weights, (float)all.sd->gravity };
      GD::foreach_feature<GD::multipredict_info<dense_parameters>, uint64_t, GD::vec_add_multipredict>(all, ec, mp);
    }
  if (all.sd->contraction != 1.)
    for (size_t c=0; c<count; c++)
      pred[c].scalar *= (float)all.sd->contraction;
  if (finalize_predictions)
    for (size_t c=0; c<count; c++)
      pred[c].scalar = GD::finalize_prediction(all.sd, pred[c].scalar);
  if (audit)
  { for (size_t c=0; c<count; c++)
    { ec.pred.scalar = pred[c].scalar;
      GD::print_audit_features(all, ec);
      ec.ft_offset += (uint64_t)step;
    }
    ec.ft_offset -= (uint64_t)(step*count);
  }
}

void inner_update_proximal(update_data& d, float x, float& wref)
{ float* w = &wref;
  float gradient = d.update * x;
  float ng2 = w[W_G2] + gradient * gradient;
  float sqrt_ng2 = sqrtf(ng2);
  float sqrt_wW_G2 = sqrtf(w[W_G2]);
  float sigma = (sqrt_ng2 - sqrt_wW_G2)/ d.ftrl_alpha;
  w[W_ZT] += gradient - sigma * w[W_XT];
  w[W_G2] = ng2;
  sqrt_wW_G2 = sqrt_ng2;
  float flag = sign(w[W_ZT]);
  float fabs_zt = w[W_ZT] * flag;
  if (fabs_zt <= d.l1_lambda)
    w[W_XT] = 0.;
  else
  { float step = 1/(d.l2_lambda + (d.ftrl_beta + sqrt_wW_G2)/d.ftrl_alpha);
    w[W_XT] = step * flag * (d.l1_lambda - fabs_zt);
  }
}

void inner_update_pistol_state_and_predict(update_data& d, float x, float& wref)
{ float* w = &wref;

  float fabs_x = fabs(x);
  if (fabs_x > w[W_MX])
    w[W_MX]=fabs_x;

  float squared_theta = w[W_ZT] * w[W_ZT];
  float tmp = 1.f / (d.ftrl_alpha * w[W_MX] * (w[W_G2] + w[W_MX]));
  w[W_XT] = sqrt(w[W_G2]) * d.ftrl_beta * w[W_ZT] * correctedExp(squared_theta / 2 * tmp) * tmp;

  d.predict +=  w[W_XT]*x;
}

void inner_update_pistol_post(update_data& d, float x, float& wref)
{ float* w = &wref;
  float gradient = d.update * x;

  w[W_ZT] += -gradient;
  w[W_G2] += fabs(gradient);
}

void update_state_and_predict_pistol(ftrl& b, base_learner&, example& ec)
{ b.data.predict = 0;

  GD::foreach_feature<update_data, inner_update_pistol_state_and_predict>(*b.all, ec, b.data);
  ec.partial_prediction = b.data.predict;
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}

void update_after_prediction_proximal(ftrl& b, example& ec)
{ b.data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label)
                  *ec.weight;

  GD::foreach_feature<update_data, inner_update_proximal>(*b.all, ec, b.data);
}

void update_after_prediction_pistol(ftrl& b, example& ec)
{ b.data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label)
                  *ec.weight;

  GD::foreach_feature<update_data, inner_update_pistol_post>(*b.all, ec, b.data);
}

template<bool audit>
void learn_proximal(ftrl& a, base_learner& base, example& ec)
{ assert(ec.in_use);

  // predict with confidence
  predict<audit>(a, base, ec);

  //update state based on the prediction
  update_after_prediction_proximal(a,ec);
}

void learn_pistol(ftrl& a, base_learner& base, example& ec)
{ assert(ec.in_use);

  // update state based on the example and predict
  update_state_and_predict_pistol(a, base, ec);

  //update state based on the prediction
  update_after_prediction_pistol(a,ec);
}

void save_load(ftrl& b, io_buf& model_file, bool read, bool text)
{ vw* all = b.all;
  if (read)
    initialize_regressor(*all);

  if (model_file.files.size() > 0)
  { bool resume = all->save_resume;
    stringstream msg;
    msg << ":"<< resume<< "\n";
    bin_text_read_write_fixed(model_file,(char *)&resume, sizeof (resume), "", read, msg, text);

    if (resume)
      GD::save_load_online_state(*all, model_file, read, text);
    else
      GD::save_load_regressor(*all, model_file, read, text);
  }
}

void end_pass(ftrl& g)
{ vw& all = *g.all;

  if(!all.holdout_set_off)
  { if(summarize_holdout_set(all, g.no_win_counter))
      finalize_regressor(all, all.final_regressor_name);
    if((g.early_stop_thres == g.no_win_counter) &&
        ((all.check_holdout_every_n_passes <= 1) ||
         ((all.current_pass % all.check_holdout_every_n_passes) == 0)))
      set_done(all);
  }
}

base_learner* ftrl_setup(vw& all)
{ if (missing_option(all, false, "ftrl", "FTRL: Follow the Proximal Regularized Leader") &&
      missing_option(all, false, "pistol", "FTRL: Parameter-free Stochastic Learning"))
  { return nullptr;
  }

  new_options(all, "FTRL options")
  ("ftrl_alpha", po::value<float>(), "Learning rate for FTRL optimization")
  ("ftrl_beta", po::value<float>(), "FTRL beta parameter");

  add_options(all);

  po::variables_map& vm = all.vm;

  ftrl& b = calloc_or_throw<ftrl>();
  b.all = &all;
  b.no_win_counter = 0;
  b.early_stop_thres = 3;

  void (*learn_ptr)(ftrl&, base_learner&, example&) = nullptr;

  string algorithm_name;
  if (vm.count("ftrl"))
  { algorithm_name = "Proximal-FTRL";
    if (all.audit)
      learn_ptr=learn_proximal<true>;
    else
      learn_ptr=learn_proximal<false>;
    if (vm.count("ftrl_alpha"))
      b.ftrl_alpha = vm["ftrl_alpha"].as<float>();
    else
      b.ftrl_alpha = 0.005f;
    if (vm.count("ftrl_beta"))
      b.ftrl_beta = vm["ftrl_beta"].as<float>();
    else
      b.ftrl_beta = 0.1f;
  }
  else if (vm.count("pistol"))
  { algorithm_name = "PiSTOL";
    learn_ptr=learn_pistol;
    if (vm.count("ftrl_alpha"))
      b.ftrl_alpha = vm["ftrl_alpha"].as<float>();
    else
      b.ftrl_alpha = 1.0f;
    if (vm.count("ftrl_beta"))
      b.ftrl_beta = vm["ftrl_beta"].as<float>();
    else
      b.ftrl_beta = 0.5f;

  }
  b.data.ftrl_alpha = b.ftrl_alpha;
  b.data.ftrl_beta = b.ftrl_beta;
  b.data.l1_lambda = b.all->l1_lambda;
  b.data.l2_lambda = b.all->l2_lambda;

  all.weights.stride_shift(2); // NOTE: for more parameter storage

  if (!all.quiet)
  { cerr << "Enabling FTRL based optimization" << endl;
    cerr << "Algorithm used: " << algorithm_name << endl;
    cerr << "ftrl_alpha = " << b.ftrl_alpha << endl;
    cerr << "ftrl_beta = " << b.ftrl_beta << endl;
  }

  if(!all.holdout_set_off)
  { all.sd->holdout_best_loss = FLT_MAX;
    if(vm.count("early_terminate"))
      b.early_stop_thres = vm["early_terminate"].as< size_t>();
  }

  learner<ftrl>& l = init_learner(&b, learn_ptr, UINT64_ONE << all.weights.stride_shift());
  if (all.audit || all.hash_inv)
    l.set_predict(predict<true>);
  else
    l.set_predict(predict<false>);
  l.set_sensitivity(sensitivity);
  if (all.audit || all.hash_inv)
    l.set_multipredict(multipredict<true>);
  else
    l.set_multipredict(multipredict<false>);
  l.set_save_load(save_load);
  l.set_end_pass(end_pass);
  return make_base(l);
}
