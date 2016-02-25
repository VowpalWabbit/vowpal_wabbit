/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include <string>
#include "correctedMath.h"
#include "gd.h"
#include "vw.h"

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

struct etas
{ float pred;
  float ub;
  ftrl& b;
  etas(ftrl& ftrlb) : b(ftrlb)
  { pred = 0;
  	ub = 0;
  }
};

inline float sign(float w) { if (w < 0.) return -1.; else  return 1.;}

inline void pred_confidence(etas& d, const float fx, float& fw)
{ float* w = &fw;
  d.pred += w[W_XT] * fx;
  float sqrtf_ng2 = sqrtf(w[W_G2]);
  float eta = ( (d.b.data.ftrl_beta+sqrtf_ng2)/d.b.data.ftrl_alpha +d.b.data.l2_lambda);
  if(fx < 0)
    d.ub += (1/eta)*fx*(-1);
  else
    d.ub += (1/eta)*fx;
}

void print_result(int f, float pred, float ub)
{ if (f >= 0)
  { char temp[30];
    std::stringstream ss;
	sprintf(temp, "%f", pred);
	ss << temp;
	ss << ' ';
	sprintf(temp, "%f", ub);
	ss << temp;
	ss << '\n';
	ssize_t len = ss.str().size();
	ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
	if (t != len)
	  cerr << "write error: " << strerror(errno) << endl;
  }
}

void output_example(vw& all, example& ec)
{
  label_data& ld = ec.l.simple;

  all.sd->update(ec.test_only, ec.loss, ec.weight, ec.num_features);
  if (ld.label != FLT_MAX && !ec.test_only)
    all.sd->weighted_labels += ld.label * ec.weight;
  all.sd->weighted_unlabeled_examples += ld.label == FLT_MAX ? ec.weight : 0;

  for (int sink : all.final_prediction_sink)
	  print_result(sink, ec.partial_prediction, ec.confidence);

  print_update(all, ec);
}

void finish_example(vw& all, ftrl& b, example& ec)
{ output_example(all, ec);
  VW::finish_example(all, &ec);
}

void predict(ftrl& b, base_learner&, example& ec)
{ ec.partial_prediction = GD::inline_predict(*b.all, ec);
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}

void predict_with_confidence(ftrl& b, base_learner&, example& ec)
{ etas eta(b);
  GD::foreach_feature<etas, pred_confidence>(*(b.all), ec, eta);
  ec.confidence = eta.ub;
  ec.partial_prediction = eta.pred;
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}

void multipredict(ftrl& b, base_learner&, example& ec, size_t count, size_t step, polyprediction* pred, bool finalize_predictions)
{ vw& all = *b.all;
  for (size_t c=0; c<count; c++)
    pred[c].scalar = ec.l.simple.initial;
  GD::multipredict_info mp = { count, step, pred, &all.reg, (float)all.sd->gravity };
  GD::foreach_feature<GD::multipredict_info, uint64_t, GD::vec_add_multipredict>(all, ec, mp);
  if (all.sd->contraction != 1.)
    for (size_t c=0; c<count; c++)
      pred[c].scalar *= (float)all.sd->contraction;
  if (finalize_predictions)
    for (size_t c=0; c<count; c++)
      pred[c].scalar = GD::finalize_prediction(all.sd, pred[c].scalar);
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
  {
	float step = 1/(d.l2_lambda + (d.ftrl_beta + sqrt_wW_G2)/d.ftrl_alpha);
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

void learn_proximal(ftrl& a, base_learner& base, example& ec)
{ assert(ec.in_use);

  // predict with confidence
  predict_with_confidence(a, base, ec);

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
      missing_option(all, false, "pistol", "FTRL: Parameter-free Stochastic Learning")){
    return nullptr;
	}

  new_options(all, "FTRL options")
  ("ftrl_alpha", po::value<float>(), "Learning rate for FTRL optimization")
  ("ftrl_beta", po::value<float>(), "FTRL beta parameter")
  ("ftrl_confidence", "FTRL Confidnece Estimate");
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
    learn_ptr=learn_proximal;
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

  all.reg.stride_shift = 2; // NOTE: for more parameter storage

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

  learner<ftrl>& l = init_learner(&b, learn_ptr, 1 << all.reg.stride_shift);

  if (vm.count("ftrl_confidence")){
	  l.set_predict(predict_with_confidence);
	  l.set_finish_example(finish_example);
  }else{
	  l.set_predict(predict);
  }

  l.set_multipredict(multipredict);
  l.set_save_load(save_load);
  l.set_end_pass(end_pass);
  return make_base(l);
}
