/*
  Copyright (c) by respective owners including Yahoo!, Microsoft, and
  individual contributors. All rights reserved.  Released under a BSD
  license as described in the file LICENSE.
*/
#pragma once
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif

#include "parse_regressor.h"
#include "constant.h"
#include "interactions.h"


namespace GD{
  LEARNER::base_learner* setup(vw& all);

  struct gd;

  float finalize_prediction(shared_data* sd, float ret);
  void print_audit_features(vw&, example& ec);
  void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
  void save_load_online_state(vw& all, io_buf& model_file, bool read, bool text, GD::gd *g = NULL);


  // iterate through one namespace (or its part), callback function T(some_data_R, feature_value_x, feature_weight)
  template <class R, void (*T)(R&, const float, float&)>
  inline void foreach_feature(weight* weight_vector, size_t weight_mask, feature* begin, feature* end, R& dat, uint32_t offset=0, float mult=1.)
  {
    for (feature* f = begin; f!= end; ++f)
      T(dat, mult*f->x, weight_vector[(f->weight_index + offset) & weight_mask]);
  }

  // iterate through one namespace (or its part), callback function T(some_data_R, feature_value_x, feature_index)
  template <class R, void (*T)(R&, float, uint32_t)>
   void foreach_feature(weight* /*weight_vector*/, size_t /*weight_mask*/, feature* begin, feature* end, R&dat, uint32_t offset=0, float mult=1.)
   {
     for (feature* f = begin; f!= end; ++f)
       T(dat, mult*f->x, f->weight_index + offset);
   }
 
   // iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x, S)
   // where S is EITHER float& feature_weight OR uint32_t feature_index
   template <class R, class S, void (*T)(R&, float, S)>
   inline void foreach_feature(vw& all, example& ec, R& dat)
   {
       uint32_t offset = ec.ft_offset;

       for (unsigned char* i = ec.indices.begin; i != ec.indices.end; ++i)
           foreach_feature<R,T>(all.reg.weight_vector, all.reg.weight_mask, ec.atomics[*i].begin, ec.atomics[*i].end, dat, offset);
   }

  // iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x, feature_weight)
  template <class R, void (*T)(R&, float, float&)>
  inline void foreach_feature(vw& all, example& ec, R& dat)
  {
    foreach_feature<R,float&,T>(all, ec, dat);
  }

 inline void vec_add(float& p, const float fx, float& fw) { p += fw * fx; }

  inline float inline_predict(vw& all, example& ec)
  {
    float temp = ec.l.simple.initial;
    foreach_feature<float, vec_add>(all, ec, temp);
    return temp;
  }
}
