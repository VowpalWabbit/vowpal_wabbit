/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include "gd.h"

using namespace std;
using namespace LEARNER;

#define W_XT 0   // current parameter
#define W_ZT 1   // in proximal is "accumulated z(t) = z(t-1) + g(t) + sigma*w(t)", in general is the dual weight vector
#define W_G2 2   // accumulated gradient information
#define W_MX 3   // maximum absolute value

//nonrentrant
struct ftrl {
  vw* all; //features, finalize, l1, l2, 
  float ftrl_alpha;
  float ftrl_beta;
  bool proximal;
  bool pistol;
  bool normalized;
};
  
void predict(ftrl& b, base_learner& base, example& ec)
{
  ec.partial_prediction = GD::inline_predict(*b.all, ec);
  if (b.normalized)
    ec.partial_prediction /= sqrt(ec.total_sum_feat_sq);
  ec.pred.scalar = GD::finalize_prediction(b.all->sd, ec.partial_prediction);
}
  
inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

struct update_data {
  float update;
  float ftrl_alpha;
  float ftrl_beta;
  float l1_lambda;
  float l2_lambda;
};

void inner_update_proximal(update_data& d, float x, float& wref) {
  float* w = &wref;
  float gradient = d.update * x;
  float ng2 = w[W_G2] + gradient * gradient;
  float sigma = (sqrtf(ng2) - sqrtf(w[W_G2]))/ d.ftrl_alpha;
  w[W_ZT] += gradient - sigma * w[W_XT];
  w[W_G2] = ng2;
  float flag = sign(w[W_ZT]);
  float fabs_zt = w[W_ZT] * flag;
  if (fabs_zt <= d.l1_lambda) 
    w[W_XT] = 0.;
  else {
    float step = 1/(d.l2_lambda + (d.ftrl_beta + sqrtf(w[W_G2]))/d.ftrl_alpha);
    w[W_XT] = step * flag * (d.l1_lambda - fabs_zt);
  }
}

void inner_update_pistol_pre(update_data& d, float x, float& wref) {
  float* w = &wref;
  
  float fabs_x = fabs(x);
  if (fabs_x > w[W_MX]) 
    w[W_MX]=fabs_x;
  
  float squared_theta = w[W_ZT] * w[W_ZT];
  float tmp = 1.f / (d.ftrl_alpha * w[W_MX] * (w[W_G2] + w[W_MX]));
  w[W_XT] = sqrt(w[W_G2]) * d.ftrl_beta * w[W_ZT] * exp(squared_theta / 2 * tmp) * tmp;
}

void inner_update_pistol_post(update_data& d, float x, float& wref) {
  float* w = &wref;
  float gradient = d.update * x;
  
  w[W_ZT] += -gradient;
  w[W_G2] += fabs(gradient);
}

void update_before_prediction(ftrl& b, example& ec)
{
  if (b.pistol) {
    struct update_data data;  
    data.ftrl_alpha = b.ftrl_alpha;
    data.ftrl_beta = b.ftrl_beta;

    GD::foreach_feature<update_data, inner_update_pistol_pre>(*b.all, ec, data);
  }
}

void update_after_prediction(ftrl& b, example& ec)
{
  struct update_data data;  
  data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label)
    *ec.l.simple.weight;
  data.ftrl_alpha = b.ftrl_alpha;
  data.ftrl_beta = b.ftrl_beta;
  data.l1_lambda = b.all->l1_lambda;
  data.l2_lambda = b.all->l2_lambda;
  
  if (b.proximal)
    GD::foreach_feature<update_data, inner_update_proximal>(*b.all, ec, data);
  else if (b.pistol)
    GD::foreach_feature<update_data, inner_update_pistol_post>(*b.all, ec, data);  
}

void learn(ftrl& a, base_learner& base, example& ec) {
  assert(ec.in_use);
  
  // update state based on the example
  update_before_prediction(a,ec);
  
  // predict w*x
  predict(a, base, ec);
  
  //update state based on the prediction
  update_after_prediction(a,ec);
}

void save_load(ftrl& b, io_buf& model_file, bool read, bool text) 
{
  vw* all = b.all;
  if (read)
    initialize_regressor(*all);
  
  if (model_file.files.size() > 0) {
    bool resume = all->save_resume;
    char buff[512];
    uint32_t text_len = sprintf(buff, ":%d\n", resume);
    bin_text_read_write_fixed(model_file,(char *)&resume, sizeof (resume), "", read, buff, text_len, text);
    
    if (resume) 
      GD::save_load_online_state(*all, model_file, read, text);
    else
      GD::save_load_regressor(*all, model_file, read, text);
  }
}

base_learner* ftrl_setup(vw& all) 
{
  if (missing_option(all, false, "ftrl", "Follow the Regularized Leader")) 
    return NULL;
  
  new_options(all, "FTRL options")
    ("ftrl_algo", po::value<string>()->default_value("pistol"), "Specify the kind of FTRL used. Currently available ones are pistol, proximal.")
    ("ftrl_normalized", "normalize the samples by their L2 norm")
    ("ftrl_alpha", po::value<float>(), "Learning rate for FTRL optimization")
    ("ftrl_beta", po::value<float>(), "FTRL beta parameter");
  add_options(all);
  
  ftrl& b = calloc_or_die<ftrl>();
  b.all = &all;
  po::variables_map& vm = all.vm;
  string ftrl_algo = vm["ftrl_algo"].as<string>();
  
  b.normalized = false;
  if (vm.count("ftrl_normalized"))
    b.normalized=true;

  b.proximal = false;
  b.pistol = false;
  if (ftrl_algo.compare("proximal") == 0) {
    b.proximal = true;
    if (vm.count("ftrl_alpha"))
      b.ftrl_alpha = vm["ftrl_alpha"].as<float>();
    else
      b.ftrl_alpha = 0.005f;
    if (vm.count("ftrl_beta"))
      b.ftrl_beta = vm["ftrl_beta"].as<float>();
    else
      b.ftrl_beta = 0.1f;
  }
  else if (ftrl_algo.compare("pistol") == 0) {
    b.pistol = true;
    if (vm.count("ftrl_alpha"))
      b.ftrl_alpha = vm["ftrl_alpha"].as<float>();
    else
      b.ftrl_alpha = 1.0f;
    if (vm.count("ftrl_beta"))
      b.ftrl_beta = vm["ftrl_beta"].as<float>();
    else
      b.ftrl_beta = 0.5f;
  }
  
  all.reg.stride_shift = 2; // NOTE: for more parameter storage
  
  if (!all.quiet) {
    cerr << "Enabling FTRL based optimization" << endl;
    cerr << "Algorithm used: "<< ftrl_algo << endl;
    cerr << "ftrl_alpha = " << b.ftrl_alpha << endl;
    cerr << "ftrl_beta = " << b.ftrl_beta << endl;
  }
  
  learner<ftrl>& l = init_learner(&b, learn, 1 << all.reg.stride_shift);
  l.set_predict(predict);
  l.set_save_load(save_load);
  return make_base(l);
}
