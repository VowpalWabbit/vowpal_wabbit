/*
   Copyright (c) by respective owners including Yahoo!, Microsoft, and
   individual contributors. All rights reserved.  Released under a BSD (revised)
   license as described in the file LICENSE.
   */
#include <fstream>
#include <float.h>
#ifndef _WIN32
#include <netdb.h>
#endif
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include "parse_example.h"
#include "constant.h"
#include "ftrl_proximal.h"
#include "cache.h"
#include "simple_label.h"
#include "vw.h"
#include "gd.h"
#include "accumulate.h"
#include "memory.h"
#include <exception>

using namespace std;
using namespace LEARNER;


#define W_XT 0   // current parameter w(XT)
#define W_GT 1   // current gradient  g(GT)
#define W_ZT 2   // accumulated z(t) = z(t-1) + g(t) + sigma*w(t)
#define W_G2 3   // accumulated gradient squre n(t) = n(t-1) + g(t)*g(t)

/********************************************************************/
/* mem & w definition ***********************************************/
/********************************************************************/ 
// w[0] = current weight
// w[1] = current first derivative
// w[2] = accumulated zt
// w[3] = accumulated g2

namespace FTRL {

  //nonrentrant
  struct ftrl {

    vw* all;
    // set by initializer

    // evaluation file pointer
    FILE* fo;
    bool progressive_validation;
  };
  
  void reset_state(vw& all) {
    uint32_t length = 1 << all.num_bits;
    size_t stride = all.reg.stride_shift;
    weight* weights = all.reg.weight_vector;
    for(uint32_t i = 0; i < length; i++, weights += stride) {
      weights[W_GT] = 0;
      weights[W_ZT] = 0;
      weights[W_G2] = 0;
    }
  }

  void update_accumulated_state(weight* w, float alpha) {
    double ng2 = w[W_G2] + w[W_GT]*w[W_GT];
    double sigma = (sqrt(ng2) - sqrt(w[W_G2]))/ alpha;
    w[W_ZT] += w[W_GT] - sigma * w[W_XT];
    w[W_G2] = ng2;
  }

  // use in gradient prediction
  void quad_grad_update(weight* weights, feature& page_feature, 
      v_array<feature> &offer_features, size_t mask, float g, float alpha) {
    size_t halfhash = quadratic_constant * page_feature.weight_index;
    float update = g * page_feature.x;
    for (feature* ele = offer_features.begin; ele != offer_features.end; ele++)
    {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[W_GT] = update * ele->x;
      update_accumulated_state(w, alpha);
    }
  }

  void cubic_grad_update(weight* weights, feature& f0, feature& f1,
      v_array<feature> &cross_features, size_t mask, float g, float alpha) {
    size_t halfhash = cubic_constant2 * (cubic_constant * f0.weight_index + f1.weight_index);
    float update = g * f0.x * f1.x;
    for (feature* ele = cross_features.begin; ele != cross_features.end; ele++) {
      weight* w=&weights[(halfhash + ele->weight_index) & mask];
      w[W_GT] = update * ele->x;
      update_accumulated_state(w, alpha);
    }
  }

  float ftrl_predict(vw& all, example& ec) {
    ec.partial_prediction = GD::inline_predict(all, ec);
    return GD::finalize_prediction(all.sd, ec.partial_prediction);
  }

  float predict_and_gradient(vw& all, example& ec) {
    float fp = ftrl_predict(all, ec);
    ec.final_prediction = fp;

    label_data* ld = (label_data*)ec.ld;
    all.set_minmax(all.sd, ld->label);

    float loss_grad = all.loss->first_derivative(all.sd, fp, ld->label) * ld->weight;

    size_t mask = all.reg.weight_mask;
    weight* weights = all.reg.weight_vector;
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) {
      feature *f = ec.atomics[*i].begin;
      for (; f != ec.atomics[*i].end; f++) {
        weight* w = &weights[f->weight_index & mask];
        w[W_GT] = loss_grad * f->x; // += -> =
        update_accumulated_state(w, all.ftrl_alpha);
      }
    }

    // bi-gram feature
    for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end(); i++) {
      if (ec.atomics[(int)(*i)[0]].size() > 0) {
        v_array<feature> temp = ec.atomics[(int)(*i)[0]];
        for (; temp.begin != temp.end; temp.begin++)
          quad_grad_update(weights, *temp.begin, ec.atomics[(int)(*i)[1]], mask, loss_grad, all.ftrl_alpha);
      } 
    }

    // tri-gram feature
    for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) {
      if ((ec.atomics[(int)(*i)[0]].size() == 0)
          || (ec.atomics[(int)(*i)[1]].size() == 0)
          || (ec.atomics[(int)(*i)[2]].size() == 0)) {
        continue; 
      }
      v_array<feature> temp1 = ec.atomics[(int)(*i)[0]];
      for (; temp1.begin != temp1.end; temp1.begin++) {
        v_array<feature> temp2 = ec.atomics[(int)(*i)[1]];
        for (; temp2.begin != temp2.end; temp2.begin++)
          cubic_grad_update(weights, *temp1.begin, *temp2.begin, ec.atomics[(int)(*i)[2]], mask, loss_grad, all.ftrl_alpha);
      }
    }
    return fp;
  }

 inline float sign(float w){ if (w < 0.) return -1.; else  return 1.;}

  void update_weight(vw& all, example& ec) {
    size_t mask = all.reg.weight_mask;
    weight* weights = all.reg.weight_vector;
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) {
      feature *f = ec.atomics[*i].begin;
      for (; f != ec.atomics[*i].end; f++) {
        weight* w = &weights[f->weight_index & mask];
        float flag = sign(w[W_ZT]);
        float fabs_zt = w[W_ZT] * flag;
        if (fabs_zt <= all.l1_lambda) {
          w[W_XT] = 0.;
        } else {
          double step = 1/(all.l2_lambda + (all.ftrl_beta + sqrt(w[W_G2]))/all.ftrl_alpha);
          w[W_XT] = step * flag * (all.l1_lambda - fabs_zt);
        }
      }
    }
  }

  void evaluate_example(vw& all, ftrl& b , example& ec) {
    label_data* ld = (label_data*)ec.ld;
    ec.loss = all.loss->getLoss(all.sd, ec.final_prediction, ld->label) * ld->weight;
    if (b.progressive_validation) {
      float v = 1./(1 + exp(-ec.final_prediction));
      fprintf(b.fo, "%.6f\t%d\n", v, (int)(ld->label * ld->weight));
    }
  }

  //void learn(void* a, void* d, example* ec) {
  void learn(ftrl& a, learner& base, example& ec) {
    vw* all = a.all;
    assert(ec.in_use);
 
    // predict w*x, compute gradient, update accumulate state
    predict_and_gradient(*all, ec);
    // evaluate, statistic
    evaluate_example(*all, a, ec);
    // update weight
    update_weight(*all, ec);
  }

  void save_load_online_state(vw& all, io_buf& model_file, bool read, bool text) {
    char buff[512];

    int text_len = sprintf(buff, "sum_loss %f\n", all.sd->sum_loss);
    bin_text_read_write_fixed(model_file,(char*)&all.sd->sum_loss, sizeof(all.sd->sum_loss), "", read, buff, text_len, text);

    text_len = sprintf(buff, "weighted_examples %f\n", all.sd->weighted_examples);
    bin_text_read_write_fixed(model_file,(char*)&all.sd->weighted_examples, sizeof(all.sd->weighted_examples), "", read, buff, text_len, text);

    text_len = sprintf(buff, "weighted_labels %f\n", all.sd->weighted_labels);
    bin_text_read_write_fixed(model_file,(char*)&all.sd->weighted_labels, sizeof(all.sd->weighted_labels), "", read, buff, text_len, text);

    text_len = sprintf(buff, "example_number %u\n", (uint32_t)all.sd->example_number);
    bin_text_read_write_fixed(model_file,(char*)&all.sd->example_number, sizeof(all.sd->example_number), "", read, buff, text_len, text);

    text_len = sprintf(buff, "total_features %u\n", (uint32_t)all.sd->total_features);
    bin_text_read_write_fixed(model_file,(char*)&all.sd->total_features, sizeof(all.sd->total_features),  "", read, buff, text_len, text);

    uint32_t length = 1 << all.num_bits;
    uint32_t stride = all.reg.stride_shift;
    uint32_t i = 0;
    size_t brw = 1;
    do 
    {
      brw = 1;
      weight* v;
      if (read) { // read binary
        brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
        if (brw > 0) {
          assert (i< length);		
          v = &(all.reg.weight_vector[stride*i]);
          brw += bin_read_fixed(model_file, (char*)v, 4*sizeof(*v), "");	
        }
      }
      else { // write binary or text
        // save w[W_XT], w[W_ZT], w[W_G2] if any of them is not zero
        v = &(all.reg.weight_vector[stride*i]);
        if (v[W_XT] !=0. || v[W_ZT] !=0. || v[W_G2] !=0.) {
          text_len = sprintf(buff, "%d", i);
          brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i),
              buff, text_len, text);

          text_len = sprintf(buff, ":%f %f %f %f\n", *v, *(v+1), *(v+2), *(v+3));
          brw += bin_text_write_fixed(model_file, (char *)v, 4*sizeof (*v),
              buff, text_len, text);
        }  // end if
      
      } // end else
      
      if (!read) { i++; }
    } while ((!read && i < length) || (read && brw >0));  
  }

  //void save_load(void* in, void* d, io_buf& model_file, bool read, bool text) {
  void save_load(ftrl& b, io_buf& model_file, bool read, bool text) {
    vw* all = b.all;
    if (read) {
      initialize_regressor(*all);
    } 

    if (model_file.files.size() > 0) {
      bool resume = all->save_resume;
      char buff[512];
      uint32_t text_len = sprintf(buff, ":%d\n", resume);
      bin_text_read_write_fixed(model_file,(char *)&resume, sizeof (resume), "", read, buff, text_len, text);

      if (resume) {
        save_load_online_state(*all, model_file, read, text);
      } else {
        GD::save_load_regressor(*all, model_file, read, text);
      }
    }

  }
  
  // placeholder
  void predict(ftrl& b, learner& base, example& ec)
  {
    vw* all = b.all;
    ((label_data*) ec.ld)->prediction = ftrl_predict(*all,ec);
  }

  //void finish(void* a, void* d) {
  void finish(ftrl& b) {
  }

  
  learner* setup(vw& all, po::variables_map& vm) {
    ftrl* b = (ftrl*)calloc_or_die(1, sizeof(ftrl));
    b->all = &all;

    po::options_description ftrl_opts("FTRL options");
    vm = add_options(all, ftrl_opts);
    
    all.ftrl = true;
    all.reg.stride_shift = 2; // NOTE: for more parameter storage
    
    b->progressive_validation = false;
    if (vm.count("progressive_validation")) {
      std::string filename = vm["progressive_validation"].as<string>();
      b->fo = fopen(filename.c_str(), "w");
      assert(b->fo != NULL);
      b->progressive_validation = true;
    }

    learner* l = new learner(b, 1 << all.reg.stride_shift);
    l->set_learn<ftrl, learn>();
    l->set_predict<ftrl, predict>();
    l->set_save_load<ftrl,save_load>();
    l->set_finish<ftrl,finish>();

    return l;
  }


} // end namespace
