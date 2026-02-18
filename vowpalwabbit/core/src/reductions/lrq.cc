// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/lrq.h"

#include "vw/common/random.h"
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parse_args.h"  // for spoof_hex_encoded_namespaces
#include "vw/core/setup_base.h"
#include "vw/core/text_utils.h"

#include <cfloat>
#include <cstring>
#include <map>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class lrq_state
{
public:
  VW::workspace* all = nullptr;  // feature creation, audit, hash_inv
  bool lrindices[256];
  size_t orig_size[256];
  std::set<std::string> lrpairs;
  bool dropout = false;
  uint64_t seed = 0;
  uint64_t initial_seed = 0;

  lrq_state()
  {
    std::fill(lrindices, lrindices + 256, false);
    std::fill(orig_size, orig_size + 256, 0);
  }
};

bool valid_int(const char* s)
{
  char* endptr;

  int v = strtoul(s, &endptr, 0);
  (void)v;

  return (*s != '\0' && *endptr == '\0');
}

inline bool cheesyrbit(uint64_t& seed) { return VW::details::merand48(seed) > 0.5; }

inline float cheesyrand(uint64_t x)
{
  uint64_t seed = x;

  return VW::details::merand48(seed);
}

constexpr inline bool example_is_test(VW::example& ec) { return ec.l.simple.label == FLT_MAX; }

void reset_seed(lrq_state& lrq)
{
  if (lrq.all->reduction_state.bfgs) { lrq.seed = lrq.initial_seed; }
}

template <bool is_learn>
void predict_or_learn(lrq_state& lrq, learner& base, VW::example& ec)
{
  VW::workspace& all = *lrq.all;

  // Remember original features

  memset(lrq.orig_size, 0, sizeof(lrq.orig_size));
  for (VW::namespace_index i : ec.indices)
  {
    if (lrq.lrindices[i]) { lrq.orig_size[i] = ec.feature_space[i].size(); }
  }

  size_t which = (is_learn && !example_is_test(ec)) ? ec.example_counter : 0;
  float first_prediction = 0;
  float first_loss = 0;
  float first_uncertainty = 0;
  unsigned int maxiter = (is_learn && !example_is_test(ec)) ? 2 : 1;

  bool do_dropout = lrq.dropout && is_learn && !example_is_test(ec);
  float scale = (!lrq.dropout || do_dropout) ? 1.f : 0.5f;

  uint32_t stride_shift = lrq.all->weights.stride_shift();
  for (unsigned int iter = 0; iter < maxiter; ++iter, ++which)
  {
    // Add left LRQ features, holding right LRQ features fixed
    //     and vice versa
    // NOTE: when a namespace appears in multiple LRQ pairs (e.g., --lrq ab2 --lrq ac2),
    // the generated features accumulate across pairs, which may cause weight index collisions.

    for (std::string const& i : lrq.lrpairs)
    {
      unsigned char left = i[which % 2];
      unsigned char right = i[(which + 1) % 2];
      unsigned int k = atoi(i.c_str() + 2);

      auto& left_fs = ec.feature_space[left];
      for (unsigned int lfn = 0; lfn < lrq.orig_size[left]; ++lfn)
      {
        float lfx = left_fs.values[lfn];
        uint64_t lindex = left_fs.indices[lfn] + ec.ft_offset;
        for (unsigned int n = 1; n <= k; ++n)
        {
          if (!do_dropout || cheesyrbit(lrq.seed))
          {
            uint64_t lwindex = (lindex + (static_cast<uint64_t>(n) << stride_shift));
            VW::weight* lw = &lrq.all->weights[lwindex];

            // perturb away from saddle point at (0, 0)
            if (is_learn)
            {
              if (!example_is_test(ec) && *lw == 0)
              {
                *lw = cheesyrand(lwindex);  // not sure if lw needs a weight mask?
              }
            }

            auto& right_fs = ec.feature_space[right];
            for (unsigned int rfn = 0; rfn < lrq.orig_size[right]; ++rfn)
            {
              // NB: ec.ft_offset added by base learner
              float rfx = right_fs.values[rfn];
              uint64_t rindex = right_fs.indices[rfn];
              uint64_t rwindex = (rindex + (static_cast<uint64_t>(n) << stride_shift));

              right_fs.push_back(scale * *lw * lfx * rfx, rwindex);

              if (all.output_config.audit || all.output_config.hash_inv)
              {
                std::stringstream new_feature_buffer;
                new_feature_buffer << right << '^' << right_fs.space_names[rfn].name << '^' << n;
                right_fs.space_names.emplace_back("lrq", new_feature_buffer.str());
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
      first_uncertainty = ec.confidence;
    }
    else
    {
      ec.pred.scalar = first_prediction;
      ec.loss = first_loss;
      ec.confidence = first_uncertainty;
    }

    for (std::string const& i : lrq.lrpairs)
    {
      unsigned char right = i[(which + 1) % 2];
      ec.feature_space[right].truncate_to(lrq.orig_size[right]);
    }
  }  // end for(max_iter)
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::lrq_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto lrq = VW::make_unique<lrq_state>();
  std::vector<std::string> lrq_names;
  option_group_definition new_options("[Reduction] Low Rank Quadratics");
  new_options.add(make_option("lrq", lrq_names).keep().necessary().help("Use low rank quadratic features"))
      .add(make_option("lrqdropout", lrq->dropout).keep().help("Use dropout training for low rank quadratic features"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  uint32_t maxk = 0;
  lrq->all = &all;

  for (const auto& name : lrq_names)
  {
    if (name.find(':') != std::string::npos) { THROW("--lrq does not support wildcards ':'"); }
  }

  for (auto& lrq_name : lrq_names) { lrq_name = VW::decode_inline_hex(lrq_name, all.logger); }

  new (&lrq->lrpairs) std::set<std::string>(lrq_names.begin(), lrq_names.end());

  lrq->initial_seed = lrq->seed = all.get_random_state()->get_current_state() | 8675309;

  if (!all.output_config.quiet)
  {
    *(all.output_runtime.trace_message) << "creating low rank quadratic features for pairs: ";
    if (lrq->dropout) { *(all.output_runtime.trace_message) << "(using dropout) "; }
  }

  for (std::string const& i : lrq->lrpairs)
  {
    if (!all.output_config.quiet)
    {
      if ((i.length() < 3) || !valid_int(i.c_str() + 2))
        THROW("Low-rank quadratic features must involve two sets and a rank: " << i);

      *(all.output_runtime.trace_message) << i << " ";
    }
    unsigned int k = atoi(i.c_str() + 2);

    lrq->lrindices[static_cast<int>(i[0])] = true;
    lrq->lrindices[static_cast<int>(i[1])] = true;

    maxk = std::max(maxk, k);
  }

  if (!all.output_config.quiet) { *(all.output_runtime.trace_message) << std::endl; }

  // Warn if any namespace appears in multiple LRQ pairs (potential weight collision)
  {
    std::map<unsigned char, int> ns_count;
    for (const auto& pair : lrq->lrpairs)
    {
      ns_count[static_cast<unsigned char>(pair[0])]++;
      ns_count[static_cast<unsigned char>(pair[1])]++;
    }
    for (const auto& [ns, count] : ns_count)
    {
      if (count > 1)
      {
        all.logger.err_warn(
            "Namespace '{}' appears in {} LRQ pairs. This may cause weight collisions.", static_cast<char>(ns), count);
      }
    }
  }

  all.reduction_state.total_feature_width = all.reduction_state.total_feature_width * static_cast<uint64_t>(1 + maxk);
  auto base = stack_builder.setup_base_learner(1 + maxk);

  auto l = make_reduction_learner(std::move(lrq), require_singleline(base), predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(lrq_setup))
               .set_feature_width(1 + maxk)
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .set_end_pass(reset_seed)
               .build();

  return l;
}
