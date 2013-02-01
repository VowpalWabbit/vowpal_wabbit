/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef GD_H
#define GD_H

#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include <math.h>
#include "example.h"
#include "parse_regressor.h"
#include "parser.h"
#include "sparse_dense.h"
#include "learner.h"

namespace GD{
void print_result(int f, float res, v_array<char> tag);
void print_audit_features(regressor &reg, example* ec, size_t offset);
float finalize_prediction(vw&, float ret);
float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask);
void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);
void print_audit_features(vw&, example* ec);
void train(weight* weights, const v_array<feature> &features, float update);
void train_one_example(regressor& r, example* ex);
void train_offset_example(regressor& r, example* ex, size_t offset);
void compute_update(example* ec);
void offset_train(regressor &reg, example* &ec, float update, size_t offset);
void train_one_example_single_thread(regressor& r, example* ex);
 learner get_learner();
 void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
void output_and_account_example(example* ec);
bool command_example(vw&, example* ec);

template <float (*T)(vw&,float,uint32_t)>
float inline_predict(vw& all, example* &ec)
{
  float prediction = all.p->lp->get_initial(ec->ld);

  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
    prediction += sd_add<T>(all, ec->atomics[*i].begin, ec->atomics[*i].end, ec->ft_offset);

  for (vector<string>::iterator i = all.pairs.begin(); i != all.pairs.end();i++) {
    if (ec->atomics[(int)(*i)[0]].size() > 0) {
      v_array<feature> temp = ec->atomics[(int)(*i)[0]];
      for (; temp.begin != temp.end; temp.begin++)
        prediction += one_pf_quad_predict<T>(all,*temp.begin,ec->atomics[(int)(*i)[1]], ec->ft_offset);
    }
  }

  for (vector<string>::iterator i = all.triples.begin(); i != all.triples.end();i++) {
    if ((ec->atomics[(int)(*i)[0]].size() == 0) || (ec->atomics[(int)(*i)[1]].size() == 0) || (ec->atomics[(int)(*i)[2]].size() == 0)) { continue; }
    v_array<feature> temp1 = ec->atomics[(int)(*i)[0]];
    for (; temp1.begin != temp1.end; temp1.begin++) {
      v_array<feature> temp2 = ec->atomics[(int)(*i)[1]];
      for (; temp2.begin != temp2.end; temp2.begin++) {
        prediction += one_pf_cubic_predict<T>(all,*temp1.begin,*temp2.begin,ec->atomics[(int)(*i)[2]], ec->ft_offset);
      }
    }
  }
  
  return prediction;
}
}

#endif
