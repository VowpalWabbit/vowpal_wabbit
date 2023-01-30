// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/lrqfa.h"

#include "vw/common/random.h"
#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/parse_args.h"  // for spoof_hex_encoded_namespaces
#include "vw/core/setup_base.h"
#include "vw/core/text_utils.h"

#include <cfloat>
#include <string>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class lrqfa_state
{
public:
  VW::workspace* all = nullptr;
  std::string field_name = "";
  int k = 0;
  int field_id[256];
  size_t orig_size[256];

  lrqfa_state()
  {
    std::fill(field_id, field_id + 256, 0);
    std::fill(orig_size, orig_size + 256, 0);
  }
};

inline float cheesyrand(uint64_t x)
{
  uint64_t seed = x;

  return VW::details::merand48(seed);
}

constexpr inline bool example_is_test(VW::example& ec) { return ec.l.simple.label == FLT_MAX; }

template <bool is_learn>
void predict_or_learn(lrqfa_state& lrq, learner& base, VW::example& ec)
{
  VW::workspace& all = *lrq.all;

  memset(lrq.orig_size, 0, sizeof(lrq.orig_size));
  for (VW::namespace_index i : ec.indices) { lrq.orig_size[i] = ec.feature_space[i].size(); }

  size_t which = (is_learn && !example_is_test(ec)) ? ec.example_counter : 0;
  float first_prediction = 0;
  float first_loss = 0;
  unsigned int maxiter = (is_learn && !example_is_test(ec)) ? 2 : 1;
  unsigned int k = lrq.k;
  float sqrtk = static_cast<float>(std::sqrt(k));

  uint32_t stride_shift = lrq.all->weights.stride_shift();
  uint64_t weight_mask = lrq.all->weights.mask();
  for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
  {
    // Add left LRQ features, holding right LRQ features fixed
    //     and vice versa

    for (std::string::const_iterator i1 = lrq.field_name.begin(); i1 != lrq.field_name.end(); ++i1)
    {
      for (std::string::const_iterator i2 = i1 + 1; i2 != lrq.field_name.end(); ++i2)
      {
        unsigned char left = (which % 2) ? *i1 : *i2;
        unsigned char right = ((which + 1) % 2) ? *i1 : *i2;
        unsigned int lfd_id = lrq.field_id[left];
        unsigned int rfd_id = lrq.field_id[right];
        for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn)
        {
          auto& fs = ec.feature_space[left];
          float lfx = fs.values[lfn];
          uint64_t lindex = fs.indices[lfn];
          for (unsigned int n = 1; n <= k; ++n)
          {
            uint64_t lwindex = (lindex +
                (static_cast<uint64_t>(rfd_id * k + n) << stride_shift));  // a feature has k weights in each field
            float* lw = &all.weights[lwindex & weight_mask];
            // perturb away from saddle point at (0, 0)
            if (is_learn)
            {
              if (!example_is_test(ec) && *lw == 0) { *lw = cheesyrand(lwindex) * 0.5f / sqrtk; }
            }

            for (unsigned int rfn = 0; rfn < lrq.orig_size[right]; ++rfn)
            {
              auto& rfs = ec.feature_space[right];
              //                    feature* rf = ec.atomics[right].begin + rfn;
              // NB: ec.ft_offset added by base learner
              float rfx = rfs.values[rfn];
              uint64_t rindex = rfs.indices[rfn];
              uint64_t rwindex = (rindex + (static_cast<uint64_t>(lfd_id * k + n) << stride_shift));

              rfs.push_back(*lw * lfx * rfx, rwindex);
              if (all.audit || all.hash_inv)
              {
                std::stringstream new_feature_buffer;
                new_feature_buffer << right << '^' << rfs.space_names[rfn].name << '^' << n;
                rfs.space_names.emplace_back("lrqfa", new_feature_buffer.str());
              }
            }
          }
        }
      }
    }

    if (is_learn) { base.learn(ec); }
    else { base.predict(ec); }

    // Restore example
    if (iter == 0)
    {
      first_prediction = ec.pred.scalar;
      first_loss = ec.loss;
    }
    else
    {
      ec.pred.scalar = first_prediction;
      ec.loss = first_loss;
    }

    for (char i : lrq.field_name)
    {
      VW::namespace_index right = i;
      auto& rfs = ec.feature_space[right];
      rfs.values.resize(lrq.orig_size[right]);
      if (all.audit || all.hash_inv) { rfs.space_names.resize(lrq.orig_size[right]); }
    }
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::lrqfa_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string lrqfa;
  option_group_definition new_options("[Reduction] Low Rank Quadratics FA");
  new_options.add(
      make_option("lrqfa", lrqfa).keep().necessary().help("Use low rank quadratic features with field aware weights"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto lrq = VW::make_unique<lrqfa_state>();
  lrq->all = &all;

  if (lrqfa.find(':') != std::string::npos) { THROW("--lrqfa does not support wildcards ':'"); }

  std::string lrqopt = VW::decode_inline_hex(lrqfa, all.logger);
  size_t last_index = lrqopt.find_last_not_of("0123456789");
  new (&lrq->field_name) std::string(lrqopt.substr(0, last_index + 1));  // make sure there is no duplicates
  lrq->k = atoi(lrqopt.substr(last_index + 1).c_str());

  int fd_id = 0;
  for (char i : lrq->field_name) { lrq->field_id[static_cast<int>(i)] = fd_id++; }

  all.wpp = all.wpp * static_cast<uint64_t>(1 + lrq->k);
  auto base = stack_builder.setup_base_learner();
  size_t ws = 1 + lrq->field_name.size() * lrq->k;

  auto l = make_reduction_learner(std::move(lrq), require_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(lrqfa_setup))
               .set_params_per_weight(ws)
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .build();

  return l;
}
