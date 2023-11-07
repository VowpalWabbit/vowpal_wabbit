// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/ccb_label.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/constant.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/interactions.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/reduction_features.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/vw.h"
#include "vw/core/vw_math.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_set>

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;

namespace
{
float ccb_weight(const VW::ccb_label& ld) { return ld.weight; }

VW::action_score convert_to_score(
    const VW::string_view& action_id_str, const VW::string_view& probability_str, VW::io::logger& logger)
{
  auto action_id = static_cast<uint32_t>(VW::details::int_of_string(action_id_str, logger));
  auto probability = VW::details::float_of_string(probability_str, logger);
  if (std::isnan(probability)) THROW("error NaN probability: " << probability_str);

  if (probability > 1.0)
  {
    logger.err_warn("invalid probability > 1 specified for an action, resetting to 1.");
    probability = 1.0;
  }
  if (probability < 0.0)
  {
    logger.err_warn("invalid probability < 0 specified for an action, resetting to 0.");
    probability = .0;
  }

  return {action_id, probability};
}

//<action>:<cost>:<probability>,<action>:<probability>,<action>:<probability>,…
VW::ccb_outcome* parse_outcome(VW::string_view outcome, VW::io::logger& logger)
{
  auto& ccb_outcome = *(new VW::ccb_outcome());

  std::vector<VW::string_view> split_commas;
  VW::tokenize(',', outcome, split_commas);

  std::vector<VW::string_view> split_colons;
  VW::tokenize(':', split_commas[0], split_colons);

  if (split_colons.size() != 3) THROW("Malformed ccb label");

  ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[2], logger));

  ccb_outcome.cost = VW::details::float_of_string(split_colons[1], logger);
  if (std::isnan(ccb_outcome.cost)) THROW("error NaN cost: " << split_colons[1]);

  split_colons.clear();

  for (size_t i = 1; i < split_commas.size(); i++)
  {
    VW::tokenize(':', split_commas[i], split_colons);
    if (split_colons.size() != 2) THROW("Must be action probability pairs");
    ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1], logger));
  }

  return &ccb_outcome;
}

void parse_explicit_inclusions(
    VW::ccb_label& ld, const std::vector<VW::string_view>& split_inclusions, VW::io::logger& logger)
{
  for (const auto& inclusion : split_inclusions)
  {
    ld.explicit_included_actions.push_back(VW::details::int_of_string(inclusion, logger));
  }
}
}  // namespace

namespace VW
{
VW::label_parser ccb_label_parser_global = {
    // default_label
    [](VW::polylabel& label) { label.conditional_contextual_bandit.reset_to_default(); },
    // parse_label
    [](VW::polylabel& label, ::VW::reduction_features& /*red_features*/, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /*ldict*/, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_ccb_label(label.conditional_contextual_bandit, reuse_mem, words, logger); },
    // cache_label
    [](const VW::polylabel& label, const ::VW::reduction_features& /*red_features*/, io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.conditional_contextual_bandit, upstream_name, text); },
    // read_cached_label
    [](VW::polylabel& label, ::VW::reduction_features& /*red_features*/, io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.conditional_contextual_bandit); },
    // get_weight
    [](const VW::polylabel& label, const ::VW::reduction_features& /*red_features*/)
    { return ccb_weight(label.conditional_contextual_bandit); },
    // test_label
    [](const VW::polylabel& label) { return label.conditional_contextual_bandit.is_test_label(); },
    // label type
    VW::label_type_t::CCB};

void parse_ccb_label(ccb_label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words,
    VW::io::logger& logger)
{
  ld.weight = 1.0;

  if (words.size() < 2) THROW("ccb labels may not be empty");
  if (!(words[0] == VW::details::CCB_LABEL)) { THROW("ccb labels require the first word to be ccb"); }

  auto type = words[1];
  if (type == VW::details::SHARED_TYPE)
  {
    if (words.size() > 2) THROW("shared labels may not have a cost");
    ld.type = VW::ccb_example_type::SHARED;
  }
  else if (type == VW::details::ACTION_TYPE)
  {
    if (words.size() > 2) THROW("action labels may not have a cost");
    ld.type = VW::ccb_example_type::ACTION;
  }
  else if (type == VW::details::SLOT_TYPE)
  {
    if (words.size() > 4) THROW("ccb slot label can only have a type cost and exclude list");
    ld.type = VW::ccb_example_type::SLOT;

    // Skip the first two words "ccb <type>"
    for (size_t i = 2; i < words.size(); i++)
    {
      auto is_outcome = words[i].find(':');
      if (is_outcome != VW::string_view::npos)
      {
        if (ld.outcome != nullptr) { THROW("There may be only 1 outcome associated with a slot.") }

        ld.outcome = parse_outcome(words[i], logger);
      }
      else
      {
        VW::tokenize(',', words[i], reuse_mem.tokens);
        parse_explicit_inclusions(ld, reuse_mem.tokens, logger);
      }
    }

    // If a full distribution has been given, check if it sums to 1, otherwise throw.
    if ((ld.outcome != nullptr) && ld.outcome->probabilities.size() > 1)
    {
      float total_pred = std::accumulate(ld.outcome->probabilities.begin(), ld.outcome->probabilities.end(), 0.f,
          [](float result_so_far, VW::action_score action_pred) { return result_so_far + action_pred.score; });

      // TODO do a proper comparison here.
      if (!VW::math::are_same(total_pred, 1.f))
      {
        THROW("When providing all prediction probabilities they must add up to 1.f, instead summed to " << total_pred);
      }
    }
  }
  else { THROW("unknown label type: " << type); }
}

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::ccb_outcome& ccbo)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ccbo.cost);
  bytes += read_model_field(io, ccbo.probabilities);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::ccb_outcome& ccbo, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ccbo.cost, upstream_name + "_cost", text);
  bytes += write_model_field(io, ccbo.probabilities, upstream_name + "_probabilities", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::ccb_label& ccb)
{
  size_t bytes = 0;
  // Since read_cached_features doesn't default the label we must do it here.
  if (ccb.outcome != nullptr) { ccb.outcome->probabilities.clear(); }
  ccb.explicit_included_actions.clear();
  bytes += read_model_field(io, ccb.type);
  bool outcome_is_present;
  bytes += read_model_field(io, outcome_is_present);
  if (outcome_is_present)
  {
    ccb.outcome = new VW::ccb_outcome();
    bytes += read_model_field(io, *ccb.outcome);
  }
  bytes += read_model_field(io, ccb.explicit_included_actions);
  bytes += read_model_field(io, ccb.weight);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::ccb_label& ccb, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ccb.type, upstream_name + "_type", text);
  bytes += write_model_field(io, ccb.outcome != nullptr, upstream_name + "_outcome_is_present", text);
  if (!(ccb.outcome == nullptr)) { bytes += write_model_field(io, *ccb.outcome, upstream_name + "_outcome", text); }
  bytes += write_model_field(io, ccb.explicit_included_actions, upstream_name + "_explicit_included_actions", text);
  bytes += write_model_field(io, ccb.weight, upstream_name + "_weight", text);
  return bytes;
}
}  // namespace model_utils
bool ccb_label::is_test_label() const { return outcome == nullptr; }
bool ccb_label::is_labeled() const { return !is_test_label(); }
void ccb_label::reset_to_default()
{
  // This is tested against nullptr, so unfortunately as things are this must be deleted when not used.
  if (outcome != nullptr)
  {
    delete outcome;
    outcome = nullptr;
  }

  explicit_included_actions.clear();
  type = ccb_example_type::UNSET;
  weight = 1.0;
}
ccb_label::~ccb_label()
{
  if (outcome != nullptr)
  {
    delete outcome;
    outcome = nullptr;
  }
}
ccb_label& ccb_label::operator=(const ccb_label& other)
{
  if (this == &other) { return *this; }

  if (outcome != nullptr)
  {
    delete outcome;
    outcome = nullptr;
  }

  type = other.type;
  outcome = nullptr;
  if (other.outcome != nullptr)
  {
    outcome = new ccb_outcome();
    *outcome = *other.outcome;
  }
  explicit_included_actions = other.explicit_included_actions;
  weight = other.weight;
  return *this;
}
ccb_label::ccb_label(const ccb_label& other)
{
  type = other.type;
  outcome = nullptr;
  if (other.outcome != nullptr)
  {
    outcome = new ccb_outcome();
    *outcome = *other.outcome;
  }
  explicit_included_actions = other.explicit_included_actions;
  weight = other.weight;
}
ccb_label& ccb_label::operator=(ccb_label&& other) noexcept
{
  std::swap(type, other.type);
  std::swap(outcome, other.outcome);
  std::swap(explicit_included_actions, other.explicit_included_actions);
  std::swap(weight, other.weight);
  return *this;
}
ccb_label::ccb_label(ccb_label&& other) noexcept
{
  type = ccb_example_type::UNSET;
  outcome = nullptr;
  weight = 0.f;

  std::swap(type, other.type);
  std::swap(outcome, other.outcome);
  std::swap(explicit_included_actions, other.explicit_included_actions);
  std::swap(weight, other.weight);
}
}  // namespace VW
