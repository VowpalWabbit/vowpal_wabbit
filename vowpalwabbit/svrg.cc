
#include <assert.h>
#include <iostream>

#include "gd.h"
#include "vw.h"

using namespace std;
using namespace LEARNER;

namespace SVRG
{

#define W_INNER      0   // working "inner-loop" weights, updated per example
#define W_STABLE     1   // stable weights, updated per stage
#define W_STABLEGRAD 2   // gradient corresponding to stable weights

struct svrg
{ int stage_size;               // Number of data passes per stage.
  int prev_pass;                // To detect that we're in a new pass.
  int stable_grad_count;        // Number of data points that
  // contributed to the stable gradient
  // calculation.

  // The VW process' global state.
  vw* all;
};

// Mimic GD::inline_predict but with offset for predicting with either
// stable versus inner weights.

template<int offset>
inline void vec_add(float& p, const float x, float& w)
{ float* ws = &w;
  p += x * ws[offset];
}

template<int offset>
inline float inline_predict(vw& all, example& ec)
{ float acc = ec.l.simple.initial;
  GD::foreach_feature<float, vec_add<offset> >(all, ec, acc);
  return acc;
}

// -- Prediction, using inner vs. stable weights --

float predict_stable(const svrg& s, example& ec)
{ return GD::finalize_prediction(s.all->sd, inline_predict<W_STABLE>(*s.all, ec));
}

void predict(svrg& s, base_learner&, example& ec)
{ ec.partial_prediction = inline_predict<W_INNER>(*s.all, ec);
  ec.pred.scalar = GD::finalize_prediction(s.all->sd, ec.partial_prediction);
}

float gradient_scalar(const svrg& s, const example& ec, float pred)
{ return s.all->loss->first_derivative(s.all->sd, pred, ec.l.simple.label)
         * ec.weight;
}

// -- Updates, taking inner steps vs. accumulating a full gradient --

struct update
{ float g_scalar_stable;
  float g_scalar_inner;
  float eta;
  float norm;
};

inline void update_inner_feature(update& u, float x, float& w)
{ float* ws = &w;
  w -= u.eta * ((u.g_scalar_inner - u.g_scalar_stable) * x + ws[W_STABLEGRAD] / u.norm);
}

inline void update_stable_feature(float& g_scalar, float x, float& w)
{ float* ws = &w;
  ws[W_STABLEGRAD] += g_scalar * x;
}

void update_inner(const svrg& s, example& ec)
{ update u;
  // |ec| already has prediction according to inner weights.
  u.g_scalar_inner = gradient_scalar(s, ec, ec.pred.scalar);
  u.g_scalar_stable = gradient_scalar(s, ec, predict_stable(s, ec));
  u.eta = s.all->eta;
  u.norm = (float) s.stable_grad_count;
  GD::foreach_feature<update, update_inner_feature>(*s.all, ec, u);
}

void update_stable(const svrg& s, example& ec)
{ float g = gradient_scalar(s, ec, predict_stable(s, ec));
  GD::foreach_feature<float, update_stable_feature>(*s.all, ec, g);
}

void learn(svrg& s, base_learner& base, example& ec)
{ assert(ec.in_use);

  predict(s, base, ec);

  const int pass = (int) s.all->passes_complete;

  if (pass % (s.stage_size + 1) == 0)   // Compute exact gradient
  { if (s.prev_pass != pass && !s.all->quiet)
    { cout << "svrg pass " << pass << ": committing stable point" << endl;
      for (uint32_t j = 0; j < VW::num_weights(*s.all); j++)
      { float w = VW::get_weight(*s.all, j, W_INNER);
        VW::set_weight(*s.all, j, W_STABLE, w);
        VW::set_weight(*s.all, j, W_STABLEGRAD, 0.f);
      }
      s.stable_grad_count = 0;
      cout << "svrg pass " << pass << ": computing exact gradient" << endl;
    }
    update_stable(s, ec);
    s.stable_grad_count++;
  }
  else                          // Perform updates
  { if (s.prev_pass != pass && !s.all->quiet)
    { cout << "svrg pass " << pass << ": taking steps" << endl;
    }
    update_inner(s, ec);
  }

  s.prev_pass = pass;
}

void save_load(svrg& s, io_buf& model_file, bool read, bool text)
{ if (read)
  { initialize_regressor(*s.all);
  }

  if (model_file.files.size() > 0)
  { bool resume = s.all->save_resume;
    stringstream msg;
    msg << ":" << resume << "\n";
    bin_text_read_write_fixed(model_file, (char*) &resume, sizeof(resume), "",
                              read, msg, text);

    if (resume)
      GD::save_load_online_state(*s.all, model_file, read, text);
    else
      GD::save_load_regressor(*s.all, model_file, read, text);

  }
}

}

using namespace SVRG;

base_learner* svrg_setup(vw& all)
{ if (missing_option(all, false, "svrg", "Streaming Stochastic Variance Reduced Gradient"))
  { return NULL;
  }
  new_options(all, "SVRG options")
  ("stage_size", po::value<int>()->default_value(1), "Number of passes per SVRG stage");
  add_options(all);

  svrg& s = calloc_or_throw<svrg>();
  s.all = &all;
  s.stage_size = all.vm["stage_size"].as<int>();
  s.prev_pass = -1;
  s.stable_grad_count = 0;

  // Request more parameter storage (4 floats per feature)
  all.weights.stride_shift(2);
  learner<svrg>& l = init_learner(&s, learn, UINT64_ONE << all.weights.stride_shift());

  l.set_predict(predict);
  l.set_save_load(save_load);
  return make_base(l);
}
