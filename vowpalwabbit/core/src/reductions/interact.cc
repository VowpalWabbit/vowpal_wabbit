// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/interact.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <sstream>

using namespace VW::config;

namespace
{
class interact
{
public:
  // namespaces to interact
  unsigned char n1 = static_cast<unsigned char>(0);
  unsigned char n2 = static_cast<unsigned char>(0);
  VW::features feat_store;
  VW::workspace* all = nullptr;
  float n1_feat_sq = 0.f;
  size_t num_features = 0;
};

bool contains_valid_namespaces(VW::features& f_src1, VW::features& f_src2, interact& in, VW::io::logger& logger)
{
  // first feature must be 1 so we're sure that the anchor feature is present
  if (f_src1.size() == 0 || f_src2.size() == 0) { return false; }

  if (f_src1.values[0] != 1)
  {
    // Anchor feature must be a number instead of text so that the relative offsets functions correctly but I don't
    // think we are able to test for this here.
    logger.err_error("Namespace '{}' misses anchor feature with value 1", static_cast<char>(in.n1));
    return false;
  }

  if (f_src2.values[0] != 1)
  {
    logger.err_error("Namespace '{}' misses anchor feature with value 1", static_cast<char>(in.n2));
    return false;
  }

  return true;
}

void multiply(VW::features& f_dest, VW::features& f_src2, interact& in)
{
  f_dest.clear();
  VW::features& f_src1 = in.feat_store;
  VW::workspace* all = in.all;
  uint64_t weight_mask = all->weights.mask();
  uint64_t base_id1 = f_src1.indices[0] & weight_mask;
  uint64_t base_id2 = f_src2.indices[0] & weight_mask;

  f_dest.push_back(f_src1.values[0] * f_src2.values[0], f_src1.indices[0]);

  uint64_t prev_id1 = 0;
  uint64_t prev_id2 = 0;

  for (size_t i1 = 1, i2 = 1; i1 < f_src1.size() && i2 < f_src2.size();)
  {
    // calculating the relative offset from the namespace offset used to match features
    uint64_t cur_id1 = static_cast<uint64_t>(((f_src1.indices[i1] & weight_mask) - base_id1) & weight_mask);
    uint64_t cur_id2 = static_cast<uint64_t>(((f_src2.indices[i2] & weight_mask) - base_id2) & weight_mask);

    // checking for sorting requirement
    if (cur_id1 < prev_id1)
    {
      in.all->logger.out_error("interact features are out of order: {0} < {1}. Skipping features.", cur_id1, prev_id1);
      return;
    }

    if (cur_id2 < prev_id2)
    {
      in.all->logger.out_error("interact features are out of order: {0} < {1}. Skipping features.", cur_id2, prev_id2);
      return;
    }

    if (cur_id1 == cur_id2)
    {
      f_dest.push_back(f_src1.values[i1] * f_src2.values[i2], f_src1.indices[i1]);
      i1++;
      i2++;
    }
    else if (cur_id1 < cur_id2) { i1++; }
    else { i2++; }
    prev_id1 = cur_id1;
    prev_id2 = cur_id2;
  }
}

template <bool is_learn, bool print_all>
void predict_or_learn(interact& in, VW::LEARNER::learner& base, VW::example& ec)
{
  VW::features& f1 = ec.feature_space[in.n1];
  VW::features& f2 = ec.feature_space[in.n2];

  if (!contains_valid_namespaces(f1, f2, in, in.all->logger))
  {
    if (is_learn) { base.learn(ec); }
    else { base.predict(ec); }

    return;
  }

  in.num_features = ec.num_features;
  ec.num_features -= f1.size();
  ec.num_features -= f2.size();

  in.feat_store = f1;

  multiply(f1, f2, in);
  ec.reset_total_sum_feat_sq();
  ec.num_features += f1.size();

  // remove 2nd namespace
  size_t n2_i = 0;
  size_t indices_original_size = ec.indices.size();
  for (; n2_i < indices_original_size; ++n2_i)
  {
    if (ec.indices[n2_i] == in.n2)
    {
      ec.indices.erase(ec.indices.begin() + n2_i);
      break;
    }
  }

  base.predict(ec);
  if (is_learn) { base.learn(ec); }

  // re-insert namespace into the right position
  if (n2_i < indices_original_size) { ec.indices.insert(ec.indices.begin() + n2_i, in.n2); }

  f1 = in.feat_store;
  ec.num_features = in.num_features;
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::interact_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string s;
  option_group_definition new_options("[Reduction] Interact via Elementwise Multiplication");
  new_options.add(make_option("interact", s)
                      .keep()
                      .necessary()
                      .help("Put weights on feature products from namespaces <n1> and <n2>"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (s.length() != 2)
  {
    all.logger.err_error("Need two namespace arguments to interact: {} won't do EXITING", s);
    return nullptr;
  }

  auto data = VW::make_unique<interact>();

  data->n1 = static_cast<unsigned char>(s[0]);
  data->n2 = static_cast<unsigned char>(s[1]);
  all.logger.err_info("Interacting namespaces {0:c} and {1:c}", data->n1, data->n2);
  data->all = &all;

  auto l = make_reduction_learner(std::move(data), require_singleline(stack_builder.setup_base_learner()),
      predict_or_learn<true, true>, predict_or_learn<false, true>, stack_builder.get_setupfn_name(interact_setup))
               .build();
  return l;
}
