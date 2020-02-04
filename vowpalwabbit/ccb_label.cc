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

#include <numeric>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include "vw_string_view.h"

using namespace LEARNER;
using namespace VW;
using namespace VW::config;

namespace CCB
{
void default_label(void* v);

size_t read_cached_label(shared_data*, void* v, io_buf& cache)
{
  // Since read_cached_features doesn't default the label we must do it here.
  default_label(v);
  CCB::label* ld = static_cast<CCB::label*>(v);

  if (ld->outcome)
  {
    ld->outcome->probabilities.clear();
  }
  ld->explicit_included_actions.clear();

  size_t read_count = 0;
  char* read_ptr;

  size_t next_read_size = sizeof(ld->type);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  ld->type = *(CCB::example_type*)read_ptr;
  read_count += sizeof(ld->type);

  bool is_outcome_present;
  next_read_size = sizeof(bool);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  is_outcome_present = *(bool*)read_ptr;
  read_count += sizeof(is_outcome_present);

  if (is_outcome_present)
  {
    ld->outcome = new CCB::conditional_contextual_bandit_outcome();
    ld->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

    next_read_size = sizeof(ld->outcome->cost);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    ld->outcome->cost = *(float*)read_ptr;
    read_count += sizeof(ld->outcome->cost);

    uint32_t size_probs;
    next_read_size = sizeof(size_probs);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    size_probs = *(uint32_t*)read_ptr;
    read_count += sizeof(size_probs);

    for (uint32_t i = 0; i < size_probs; i++)
    {
      ACTION_SCORE::action_score a_s;
      next_read_size = sizeof(a_s);
      if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
        return 0;
      a_s = *(ACTION_SCORE::action_score*)read_ptr;
      read_count += sizeof(a_s);

      ld->outcome->probabilities.push_back(a_s);
    }
  }

  uint32_t size_includes;
  next_read_size = sizeof(size_includes);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  size_includes = *(uint32_t*)read_ptr;
  read_count += sizeof(size_includes);

  for (uint32_t i = 0; i < size_includes; i++)
  {
    uint32_t include;
    next_read_size = sizeof(include);
    if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
      return 0;
    include = *(uint32_t*)read_ptr;
    read_count += sizeof(include);
    ld->explicit_included_actions.push_back(include);
  }

  next_read_size = sizeof(ld->weight);
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
    return 0;
  ld->weight = *(float*)read_ptr;
  return read_count;
}

float ccb_weight(void* v)
{
  CCB::label* ld = (CCB::label*)v;
  return ld->weight;
}

void cache_label(void* v, io_buf& cache)
{
  char* c;
  CCB::label* ld = static_cast<CCB::label*>(v);
  size_t size = sizeof(uint8_t)  // type
      + sizeof(bool)             // outcome exists?
      + (ld->outcome == nullptr ? 0
                                : sizeof(ld->outcome->cost)                                    // cost
                    + sizeof(uint32_t)                                                         // probabilities size
                    + sizeof(ACTION_SCORE::action_score) * ld->outcome->probabilities.size())  // probabilities
      + sizeof(uint32_t)  // explicit_included_actions size
      + sizeof(uint32_t) * ld->explicit_included_actions.size() + sizeof(ld->weight);

  cache.buf_write(c, size);

  *(uint8_t*)c = static_cast<uint8_t>(ld->type);
  c += sizeof(ld->type);

  *(bool*)c = ld->outcome != nullptr;
  c += sizeof(bool);

  if (ld->outcome != nullptr)
  {
    *(float*)c = ld->outcome->cost;
    c += sizeof(ld->outcome->cost);

    *(uint32_t*)c = convert(ld->outcome->probabilities.size());
    c += sizeof(uint32_t);

    for (const auto& score : ld->outcome->probabilities)
    {
      *(ACTION_SCORE::action_score*)c = score;
      c += sizeof(ACTION_SCORE::action_score);
    }
  }

  *(uint32_t*)c = convert(ld->explicit_included_actions.size());
  c += sizeof(uint32_t);

  for (const auto& included_action : ld->explicit_included_actions)
  {
    *(uint32_t*)c = included_action;
    c += sizeof(included_action);
  }

  *(float*)c = ld->weight;
  c += sizeof(ld->weight);
}

void default_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);

  // This is tested against nullptr, so unfortunately as things are this must be deleted when not used.
  if (ld->outcome)
  {
    ld->outcome->probabilities.delete_v();
    delete ld->outcome;
    ld->outcome = nullptr;
  }

  ld->explicit_included_actions.clear();
  ld->type = example_type::unset;
  ld->weight = 1.0;
}

bool test_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  return ld->outcome == nullptr;
}

void delete_label(void* v)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  if (ld->outcome)
  {
    ld->outcome->probabilities.delete_v();
    delete ld->outcome;
    ld->outcome = nullptr;
  }
  ld->explicit_included_actions.delete_v();
}

void copy_label(void* dst, void* src)
{
  CCB::label* ldDst = static_cast<CCB::label*>(dst);
  CCB::label* ldSrc = static_cast<CCB::label*>(src);

  if (ldSrc->outcome)
  {
    ldDst->outcome = new CCB::conditional_contextual_bandit_outcome();
    ldDst->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

    ldDst->outcome->cost = ldSrc->outcome->cost;
    copy_array(ldDst->outcome->probabilities, ldSrc->outcome->probabilities);
  }

  copy_array(ldDst->explicit_included_actions, ldSrc->explicit_included_actions);
  ldDst->type = ldSrc->type;
  ldDst->weight = ldSrc->weight;
}

ACTION_SCORE::action_score convert_to_score(const VW::string_view& action_id_str, const VW::string_view& probability_str)
{
  auto action_id = static_cast<uint32_t>(int_of_string(action_id_str));
  auto probability = float_of_string(probability_str);
  if (std::isnan(probability))
    THROW("error NaN probability: " << probability_str);

  if (probability > 1.0)
  {
    std::cerr << "invalid probability > 1 specified for an outcome, resetting to 1.\n";
    probability = 1.0;
  }
  if (probability < 0.0)
  {
    std::cerr << "invalid probability < 0 specified for an outcome, resetting to 0.\n";
    probability = .0;
  }

  return {action_id, probability};
}

//<action>:<cost>:<probability>,<action>:<probability>,<action>:<probability>,…
CCB::conditional_contextual_bandit_outcome* parse_outcome(VW::string_view& outcome)
{
  auto& ccb_outcome = *(new CCB::conditional_contextual_bandit_outcome());

  auto split_commas = v_init<VW::string_view>();
  tokenize(',', outcome, split_commas);

  auto split_colons = v_init<VW::string_view>();
  tokenize(':', split_commas[0], split_colons);

  if (split_colons.size() != 3)
    THROW("Malformed ccb label");

  ccb_outcome.probabilities = v_init<ACTION_SCORE::action_score>();
  ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[2]));

  ccb_outcome.cost = float_of_string(split_colons[1]);
  if (std::isnan(ccb_outcome.cost))
    THROW("error NaN cost: " << split_colons[1]);

  split_colons.clear();

  for (size_t i = 1; i < split_commas.size(); i++)
  {
    tokenize(':', split_commas[i], split_colons);
    if (split_colons.size() != 2)
      THROW("Must be action probability pairs");
    ccb_outcome.probabilities.push_back(convert_to_score(split_colons[0], split_colons[1]));
  }

  split_colons.delete_v();
  split_commas.delete_v();

  return &ccb_outcome;
}

void parse_explicit_inclusions(CCB::label* ld, v_array<VW::string_view>& split_inclusions)
{
  for (const auto& inclusion : split_inclusions)
  {
    ld->explicit_included_actions.push_back(int_of_string(inclusion));
  }
}

void parse_label(parser* p, shared_data*, void* v, v_array<VW::string_view>& words)
{
  CCB::label* ld = static_cast<CCB::label*>(v);
  ld->weight = 1.0;

  if (words.size() < 2)
    THROW("ccb labels may not be empty");
  if (!(words[0] == "ccb"))
  {
    THROW("ccb labels require the first word to be ccb");
  }

  auto type = words[1];
  if (type == "shared")
  {
    if (words.size() > 2)
      THROW("shared labels may not have a cost");
    ld->type = CCB::example_type::shared;
  }
  else if (type == "action")
  {
    if (words.size() > 2)
      THROW("action labels may not have a cost");
    ld->type = CCB::example_type::action;
  }
  else if (type == "slot")
  {
    if (words.size() > 4)
      THROW("ccb slot label can only have a type cost and exclude list");
    ld->type = CCB::example_type::slot;

    // Skip the first two words "ccb <type>"
    for (size_t i = 2; i < words.size(); i++)
    {
      auto is_outcome = words[i].find(':');
      if (is_outcome != VW::string_view::npos)
      {
        if (ld->outcome != nullptr)
        {
          THROW("There may be only 1 outcome associated with a slot.")
        }

        ld->outcome = parse_outcome(words[i]);
      }
      else
      {
        tokenize(',', words[i], p->parse_name);
        parse_explicit_inclusions(ld, p->parse_name);
      }
    }

    // If a full distribution has been given, check if it sums to 1, otherwise throw.
    if (ld->outcome && ld->outcome->probabilities.size() > 1)
    {
      float total_pred = std::accumulate(ld->outcome->probabilities.begin(), ld->outcome->probabilities.end(), 0.f,
          [](float result_so_far, ACTION_SCORE::action_score action_pred) {
            return result_so_far + action_pred.score;
          });

      // TODO do a proper comparison here.
      if (total_pred > 1.1f || total_pred < 0.9f)
      {
        THROW("When providing all predicition probabilties they must add up to 1.f");
      }
    }
  }
  else
  {
    THROW("unknown label type: " << type);
  }
}

// Export the definition of this label parser.
label_parser ccb_label_parser = {default_label, parse_label, cache_label, read_cached_label, delete_label, ccb_weight,
    copy_label, test_label, sizeof(CCB::label)};
}  // namespace CCB
