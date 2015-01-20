/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include "gd.h"

using namespace std;
using namespace LEARNER;

#define W_XT 0   // current parameter w(XT)
#define W_ZT 1   // accumulated z(t) = z(t-1) + g(t) + sigma*w(t)
#define W_G2 2   // accumulated gradient squre n(t) = n(t-1) + g(t)*g(t)

/********************************************************************/
/* mem & w definition ***********************************************/
/********************************************************************/ 
// w[0] = current weight
// w[1] = accumulated zt
// w[2] = accumulated g2

//nonrentrant
struct ftrl {
  vw* all;
  // set by initializer
  float ftrl_alpha;
  float ftrl_beta;
};
  
void predict(ftrl& b, base_learner& base, example& ec)
{
  ec.partial_prediction = GD::inline_predict(*b.all, ec);
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

void inner_update(update_data& d, float x, float& wref) {
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

void update(ftrl& b, example& ec)
{
  struct update_data data;  
  data.update = b.all->loss->first_derivative(b.all->sd, ec.pred.scalar, ec.l.simple.label)
    *ec.l.simple.weight;
  data.ftrl_alpha = b.ftrl_alpha;
  data.ftrl_beta = b.ftrl_beta;
  data.l1_lambda = b.all->l1_lambda;
  data.l2_lambda = b.all->l2_lambda;
 
  GD::foreach_feature<update_data, inner_update>(*b.all, ec, data);
}

void learn(ftrl& a, base_learner& base, example& ec) {
  assert(ec.in_use);
  // predict w*x
  predict(a, base, ec);
  //updat state
  update(a,ec);
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
    ("ftrl_alpha", po::value<float>()->default_value(0.0), "Learning rate for FTRL-proximal optimization")
    ("ftrl_beta", po::value<float>()->default_value(0.1f), "FTRL beta");
  add_options(all);
  
  ftrl& b = calloc_or_die<ftrl>();
  b.all = &all;
  b.ftrl_beta = all.vm["ftrl_beta"].as<float>();
  b.ftrl_alpha = all.vm["ftrl_alpha"].as<float>();
  
  all.reg.stride_shift = 2; // NOTE: for more parameter storage
  
  if (!all.quiet) {
    cerr << "Enabling FTRL-Proximal based optimization" << endl;
    cerr << "ftrl_alpha = " << b.ftrl_alpha << endl;
    cerr << "ftrl_beta = " << b.ftrl_beta << endl;
  }
  
  learner<ftrl>& l = init_learner(&b, learn, 1 << all.reg.stride_shift);
  l.set_predict(predict);
  l.set_save_load(save_load);
  return make_base(l);
}
