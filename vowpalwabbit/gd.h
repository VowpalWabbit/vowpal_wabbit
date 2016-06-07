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

namespace GD
{
LEARNER::base_learner* setup(vw& all);

struct gd;

float finalize_prediction(shared_data* sd, float ret);
void print_audit_features(vw&, example& ec);
void save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
void save_load_online_state(vw& all, io_buf& model_file, bool read, bool text, GD::gd *g = nullptr);

struct multipredict_info { size_t count; size_t step; polyprediction* pred; regressor* reg; /* & for l1: */ float gravity; };

inline void vec_add_multipredict(multipredict_info& mp, const float fx, uint64_t fi)
{ if ((-1e-10 < fx) && (fx < 1e-10)) return;
  weight*w    = mp.reg->weight_vector;
  uint64_t mask = mp.reg->weight_mask;
  polyprediction* p = mp.pred;

  fi &= mask;
  uint64_t top = fi + (uint64_t)((mp.count-1) * mp.step);
  if (top <= mask)
  { weight* last = w + top;
    w += fi;
    for (; w <= last; w += mp.step, ++p)
      p->scalar += fx **w;
  }
  else    // TODO: this could be faster by unrolling into two loops
    for (size_t c=0; c<mp.count; ++c, fi += (uint64_t)mp.step, ++p)
    { fi &= mask;
      p->scalar += fx * w[fi];
    }
}

// iterate through one namespace (or its part), callback function T(some_data_R, feature_value_x, feature_weight)
template <class R, void (*T)(R&, const float, float&)>
inline void foreach_feature(weight* weight_vector, uint64_t weight_mask, features& fs, R& dat, uint64_t offset=0, float mult=1.)
{
  for (features::iterator& f : fs)
    T(dat, mult*f.value(), weight_vector[(f.index() + offset) & weight_mask]);
}

// iterate through one namespace (or its part), callback function T(some_data_R, feature_value_x, feature_index)
template <class R, void (*T)(R&, float, uint64_t)>
void foreach_feature(weight* /*weight_vector*/, uint64_t /*weight_mask*/, features& fs, R&dat, uint64_t offset=0, float mult=1.)
{
  for (features::iterator& f : fs)
    T(dat, mult*f.value(), f.index() + offset);
}

// iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x, S)
// where S is EITHER float& feature_weight OR uint64_t feature_index
template <class R, class S, void (*T)(R&, float, S)>
inline void foreach_feature(vw& all, example& ec, R& dat)
{ uint64_t offset = ec.ft_offset;

for (features& f : ec)
    foreach_feature<R,T>(all.reg.weight_vector, all.reg.weight_mask, f, dat, offset);

  INTERACTIONS::generate_interactions<R,S,T>(all, ec, dat);
}

// iterate through all namespaces and quadratic&cubic features, callback function T(some_data_R, feature_value_x, feature_weight)
template <class R, void (*T)(R&, float, float&)>
inline void foreach_feature(vw& all, example& ec, R& dat)
{ foreach_feature<R,float&,T>(all, ec, dat);
}

inline void vec_add(float& p, const float fx, float& fw) { p += fw * fx; }

inline float inline_predict(vw& all, example& ec)
{ float temp = ec.l.simple.initial;
  foreach_feature<float, vec_add>(all, ec, temp);
  return temp;
}
}
