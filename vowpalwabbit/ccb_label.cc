// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "conditional_contextual_bandit.h"
#include "reductions.h"
#include "example.h"
#include "global_data.h"
#include "cache.h"
#include "vw.h"
#include "interactions.h"
#include "label_dictionary.h"
#include "cb_adf.h"
#include "cb_algs.h"
#include "constant.h"
#include "example.h"
#include "vw_math.h"
#include "vw_string_view.h"
#include "parse_primitives.h"
#include "reduction_features.h"

#include "io/logger.h"

#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <cmath>

using namespace VW::LEARNER;
using namespace VW;
using namespace VW::config;

namespace logger = VW::io::logger;

namespace CCB
{
void default_label(label& ld);

size_t read_cached_label(shared_data*, label& ld, io_buf& cache)
{
  // Since read_cached_features doesn't default the label we must do it here.
  default_label(ld);

  if (ld.outcome) { ld.outcome->probabilities.clear(); }
  ld.explicit_included_actions.clear();

  size_t read_count = 0;
  char* read_ptr;

  size_t next_read_size = sizeof(ld.type);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size) return 0;
  ld.type = *reinterpret_cast<CCB::example_type*>(read_ptr);
  read_count += sizeof(ld.type);

  bool is_outcome_present;
  next_read_size = sizeof(bool);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size) { return 0; }
  is_outcome_present = *reinterpret_cast<bool*>(read_ptr);
  read_count += sizeof(is_outcome_present);

  if (is_outcome_present)
  {
    ld.outcome = new CCB::conditional_contextual_bandit_outcome();

    next_read_size = sizeof(ld.outcome->cost);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size) return 0;
    ld.outcome->cost = *reinterpret_cast<float*>(read_ptr);
    read_count += sizeof(ld.outcome->cost);

    uint32_t size_probs;
    next_read_size = sizeof(size_probs);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size) { return 0; }
    size_probs = *reinterpret_cast<uint32_t*>(read_ptr);
    read_count += sizeof(size_probs);

    for (uint32_t i = 0; i < size_probs; i++)
    {
      ACTION_SCORE::action_score a_s;
      next_read_size = sizeof(a_s);
      if (cache.buf_read(read_ptr, next_read_size) < next_read_size) { return 0; }
      a_s = *reinterpret_cast<ACTION_SCORE::action_score*>(read_ptr);
      read_count += sizeof(a_s);

      ld.outcome->probabilities.push_back(a_s);
    }
  }

  uint32_t size_includes;
  next_read_size = sizeof(size_includes);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size) { return 0; }
  size_includes = *reinterpret_cast<uint32_t*>(read_ptr);
  read_count += sizeof(size_includes);

  for (uint32_t i = 0; i < size_includes; i++)
  {
    uint32_t include;
    next_read_size = sizeof(include);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size) { return 0; }
    include = *reinterpret_cast<uint32_t*>(read_ptr);
    read_count += sizeof(include);
    ld.explicit_included_actions.push_back(include);
  }

  next_read_size = sizeof(ld.weight);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size) return 0;
  ld.weight = *reinterpret_cast<float*>(read_ptr);
  return read_count;
}

float ccb_weight(CCB::label& ld) { return ld.weight; }

void cache_label(label& ld, io_buf& cache)
{
  char* c;
  size_t size = sizeof(uint8_t)  // type
      + sizeof(bool)             // outcome exists?
      + (ld.outcome == nullptr ? 0
                               : sizeof(ld.outcome->cost)                                     // cost
                    + sizeof(uint32_t)                                                        // probabilities size
                    + sizeof(ACTION_SCORE::action_score) * ld.outcome->probabilities.size())  // probabilities
      + sizeof(uint32_t)  // explicit_included_actions size
      + sizeof(uint32_t) * ld.explicit_included_actions.size() + sizeof(ld.weight);

  cache.buf_write(c, size);

  *reinterpret_cast<uint8_t*>(c) = static_cast<uint8_t>(ld.type);
  c += sizeof(ld.type);

  *reinterpret_cast<bool*>(c) = ld.outcome != nullptr;
  c += sizeof(bool);

  if (ld.outcome != nullptr)
  {
    *reinterpret_cast<float*>(c) = ld.outcome->cost;
    c += sizeof(ld.outcome->cost);

    *reinterpret_cast<uint32_t*>(c) = convert(ld.outcome->probabilities.size());
    c += sizeof(uint32_t);

    for (const auto& score : ld.outcome->probabilities)
    {
      *reinterpret_cast<ACTION_SCORE::action_score*>(c) = score;
      c += sizeof(ACTION_SCORE::action_score);
    }
  }

  *reinterpret_cast<uint32_t*>(c) = convert(ld.explicit_included_actions.size());
  c += sizeof(uint32_t);

  for (const auto& included_action : ld.explicit_included_actions)
  {
    *reinterpret_cast<uint32_t*>(c) = included_action;
    c += sizeof(included_action);
  }

  *reinterpret_cast<float*>(c) = ld.weight;
  c += sizeof(ld.weight);
}

void default_label(label& ld)
{
  // This is tested against nullptr, so unfortunately as things are this must be deleted when not used.
  if (ld.outcome != nullptr)
  {
    delete ld.outcome;
    ld.outcome = nullptr;
  }

  ld.explicit_included_actions.clear();
  ld.type = example_type::unset;
  ld.weight = 1.0;
}

bool test_label(CCB::label& ld) { return ld.outcome == nullptr; }

ACTION_SCORE::action_score convert_to_score(
    const VW::string_view& action_id_str, const VW::string_view& probability_str)
{
  auto action_id = static_cast<uint32_t>(int_of_string(action_id_str));
  auto probability = float_of_string(probability_str);
  if (std::isnan(probability)) THROW("error NaN probability: " << probability_str);

  if (probability > 1.0)
  {
    logger::errlog_warn("invalid probability > 1 specified for an action, resetting to 1.");
    probability = 1.0;
  }
  if (probability < 0.0)
  {
    logger::errlog_warn("invalid probability < 0 specified for an action, resetting to 0.");
    probability = .0;
  }

  return {action_id, probability};
}

//<action>:<cost>:<probability>,<action>:<probability>,<action>:<probability>,…
CCB::conditional_contextual_bandit_outcome* parse_outcome(VW::string_view& outcome)
{
  auto& ccb_outcome = *(new CCB::conditional_contextual_bandit_outcome());

  std::vector<VW::string_view> split_commas;
  tokenize(',', outcome, split_commas);

  std::vector<VW::string_view> split_colons;
  tokenize(':', split_commas[0], split_colons);

  if (split_colons.size() != 3) THROW("Malformed ccb label");

  ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[2]));

  ccb_outcome.cost = float_of_string(split_colons[1]);
  if (std::isnan(ccb_outcome.cost)) THROW("error NaN cost: " << split_colons[1]);

  split_colons.clear();

  for (size_t i = 1; i < split_commas.size(); i++)
  {
    tokenize(':', split_commas[i], split_colons);
    if (split_colons.size() != 2) THROW("Must be action probability pairs");
    ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1]));
  }

  return &ccb_outcome;
}

void parse_explicit_inclusions(CCB::label& ld, const std::vector<VW::string_view>& split_inclusions)
{
  for (const auto& inclusion : split_inclusions) { ld.explicit_included_actions.push_back(int_of_string(inclusion)); }
}

void parse_label(parser* p, shared_data*, label& ld, std::vector<VW::string_view>& words, ::reduction_features&)
{
  ld.weight = 1.0;

  if (words.size() < 2) THROW("ccb labels may not be empty");
  if (!(words[0] == CCB_LABEL)) { THROW("ccb labels require the first word to be ccb"); }

  auto type = words[1];
  if (type == SHARED_TYPE)
  {
    if (words.size() > 2) THROW("shared labels may not have a cost");
    ld.type = CCB::example_type::shared;
  }
  else if (type == ACTION_TYPE)
  {
    if (words.size() > 2) THROW("action labels may not have a cost");
    ld.type = CCB::example_type::action;
  }
  else if (type == SLOT_TYPE)
  {
    if (words.size() > 4) THROW("ccb slot label can only have a type cost and exclude list");
    ld.type = CCB::example_type::slot;

    // Skip the first two words "ccb <type>"
    for (size_t i = 2; i < words.size(); i++)
    {
      auto is_outcome = words[i].find(':');
      if (is_outcome != VW::string_view::npos)
      {
        if (ld.outcome != nullptr) { THROW("There may be only 1 outcome associated with a slot.") }

        ld.outcome = parse_outcome(words[i]);
      }
      else
      {
        tokenize(',', words[i], p->parse_name);
        parse_explicit_inclusions(ld, p->parse_name);
      }
    }

    // If a full distribution has been given, check if it sums to 1, otherwise throw.
    if ((ld.outcome != nullptr) && ld.outcome->probabilities.size() > 1)
    {
      float total_pred = std::accumulate(ld.outcome->probabilities.begin(), ld.outcome->probabilities.end(), 0.f,
          [](float result_so_far, ACTION_SCORE::action_score action_pred) {
            return result_so_far + action_pred.score;
          });

      // TODO do a proper comparison here.
      if (!VW::math::are_same(total_pred, 1.f))
      {
        THROW("When providing all prediction probabilities they must add up to 1.f, instead summed to " << total_pred);
      }
    }
  }
  else
  {
    THROW("unknown label type: " << type);
  }
}

// clang-format off
label_parser ccb_label_parser = {
  // default_label
  [](polylabel* v) { default_label(v->conditional_contextual_bandit); },
  // parse_label
  [](parser* p, shared_data* sd, polylabel* v, std::vector<VW::string_view>& words, ::reduction_features& red_features) {
    parse_label(p, sd, v->conditional_contextual_bandit, words, red_features);
  },
  // cache_label
  [](polylabel* v, ::reduction_features&, io_buf& cache) { cache_label(v->conditional_contextual_bandit, cache); },
  // read_cached_label
  [](shared_data* sd, polylabel* v, ::reduction_features&, io_buf& cache) { return read_cached_label(sd, v->conditional_contextual_bandit, cache); },
  // get_weight
  [](polylabel* v, const ::reduction_features&) { return ccb_weight(v->conditional_contextual_bandit); },
  // test_label
  [](polylabel* v) { return test_label(v->conditional_contextual_bandit); },
  label_type_t::ccb
};
// clang-format on
}  // namespace CCB
