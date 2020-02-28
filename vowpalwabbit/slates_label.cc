// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "slates_label.h"

#include "conditional_contextual_bandit.h"
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
//   // Since read_cached_features doesn't default the label we must do it here.
//   default_label(v);
//   CCB::label* ld = static_cast<CCB::label*>(v);

//   if (ld->outcome)
//   {
//     ld->outcome->probabilities.clear();
//   }
//   ld->explicit_included_actions.clear();

//   size_t read_count = 0;
//   char* read_ptr;

//   size_t next_read_size = sizeof(ld->type);
//   if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//     return 0;
//   ld->type = *(CCB::example_type*)read_ptr;
//   read_count += sizeof(ld->type);

//   bool is_outcome_present;
//   next_read_size = sizeof(bool);
//   if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//     return 0;
//   is_outcome_present = *(bool*)read_ptr;
//   read_count += sizeof(is_outcome_present);

//   if (is_outcome_present)
//   {
//     ld->outcome = new CCB::conditional_contextual_bandit_outcome();
//     ld->outcome->probabilities = v_init<ACTION_SCORE::action_score>();

//     next_read_size = sizeof(ld->outcome->cost);
//     if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//       return 0;
//     ld->outcome->cost = *(float*)read_ptr;
//     read_count += sizeof(ld->outcome->cost);

//     uint32_t size_probs;
//     next_read_size = sizeof(size_probs);
//     if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//       return 0;
//     size_probs = *(uint32_t*)read_ptr;
//     read_count += sizeof(size_probs);

//     for (uint32_t i = 0; i < size_probs; i++)
//     {
//       ACTION_SCORE::action_score a_s;
//       next_read_size = sizeof(a_s);
//       if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//         return 0;
//       a_s = *(ACTION_SCORE::action_score*)read_ptr;
//       read_count += sizeof(a_s);

//       ld->outcome->probabilities.push_back(a_s);
//     }
//   }

//   uint32_t size_includes;
//   next_read_size = sizeof(size_includes);
//   if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//     return 0;
//   size_includes = *(uint32_t*)read_ptr;
//   read_count += sizeof(size_includes);

//   for (uint32_t i = 0; i < size_includes; i++)
//   {
//     uint32_t include;
//     next_read_size = sizeof(include);
//     if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//       return 0;
//     include = *(uint32_t*)read_ptr;
//     read_count += sizeof(include);
//     ld->explicit_included_actions.push_back(include);
//   }

//   next_read_size = sizeof(ld->weight);
//   if (cache.buf_read(read_ptr, next_read_size) < next_read_size)
//     return 0;
//   ld->weight = *(float*)read_ptr;
//   return read_count;
return 0;
}

float weight(void* v)
{

  return static_cast<polylabel*>(v)->slates.weight;
}

void cache_label(void* v, io_buf& cache)
{
//   char* c;
//   CCB::label* ld = static_cast<CCB::label*>(v);
//   size_t size = sizeof(uint8_t)  // type
//       + sizeof(bool)             // outcome exists?
//       + (ld->outcome == nullptr ? 0
//                                 : sizeof(ld->outcome->cost)                                    // cost
//                     + sizeof(uint32_t)                                                         // probabilities size
//                     + sizeof(ACTION_SCORE::action_score) * ld->outcome->probabilities.size())  // probabilities
//       + sizeof(uint32_t)  // explicit_included_actions size
//       + sizeof(uint32_t) * ld->explicit_included_actions.size() + sizeof(ld->weight);

//   cache.buf_write(c, size);

//   *(uint8_t*)c = static_cast<uint8_t>(ld->type);
//   c += sizeof(ld->type);

//   *(bool*)c = ld->outcome != nullptr;
//   c += sizeof(bool);

//   if (ld->outcome != nullptr)
//   {
//     *(float*)c = ld->outcome->cost;
//     c += sizeof(ld->outcome->cost);

//     *(uint32_t*)c = convert(ld->outcome->probabilities.size());
//     c += sizeof(uint32_t);

//     for (const auto& score : ld->outcome->probabilities)
//     {
//       *(ACTION_SCORE::action_score*)c = score;
//       c += sizeof(ACTION_SCORE::action_score);
//     }
//   }

//   *(uint32_t*)c = convert(ld->explicit_included_actions.size());
//   c += sizeof(uint32_t);

//   for (const auto& included_action : ld->explicit_included_actions)
//   {
//     *(uint32_t*)c = included_action;
//     c += sizeof(included_action);
//   }

//   *(float*)c = ld->weight;
//   c += sizeof(ld->weight);
}

void default_label(void* v)
{
  auto& label = static_cast<polylabel*>(v)->slates;
  label.type = example_type::unset;
  label.weight = 1.f;
  label.labeled = false;
  label.cost = 0.f;
  label.slot_id = 0;
  label.probabilities.clear();
}

bool test_label(void* v)
{
  auto& ld = static_cast<polylabel*>(v)->slates;
  return ld.labeled == false;
}

void delete_label(void* v)
{
  auto& ld = static_cast<polylabel*>(v)->slates;
  label.probabilities.delete_v();
}

void copy_label(void* dst, void* src)
{
  auto& ldDst = static_cast<polylabel*>(dst)->slates;
  auto& ldSrc = static_cast<polylabel*>(src)->slates;

  ldDst.type = ldSrc.type;
  ldDst.weight = ldSrc.weight;
  ldDst.labeled = ldSrc.labeled;
  ldDst.cost = ldSrc.cost;
  ldDst.slot_id = ldSrc.slot_id;
  copy_array(ldDst.probabilities, ldSrc.probabilities);
}

void parse_label(parser* p, shared_data*, void* v, v_array<VW::string_view>& words)
{
  auto& ld = static_cast<polylabel*>(v)->slates;
  ld.weight = 1;

  if (words.size() < 2)
    THROW("slates labels may not be empty");
  if (!(words[0] == "slates"))
  {
    THROW("slates labels require the first word to be slates");
  }

  const auto& type = words[1];
  if (type == "shared")
  {
    // There is a cost defined.
    if(words.size() == 3)
    {
      ld.cost = float_of_string(words[2]);
      ld.labeled = true;
    }
    else if(words.size() != 2)
    {
      THROW("Slates shared labels must be of the form: slates shared [global_cost]");
    }
    
    ld.type = CCB::example_type::shared;
  }
  else if (type == "action")
  {
    if(words.size() > 3)
    {
      THROW("Slates action labels must be of the form: slates action <slot_id>");
    }

    ld.slot_id = int_of_string(words[2]);
    ld.type = CCB::example_type::action;
  }
  else if (type == "slot")
  {
    if (words.size() == 3)
    {
      tokenize(',', words[i], p->parse_name);
      auto split_colons = v_init<VW::string_view>();
      for(auto& token : p->parse_name) {
        tokenize(':', token, split_colons);
        if(split_colons.size() != 2)
        {
          THROW("Malformed action score token");
        }

        

      }
      
    }
    else if (words.size() != 2)
    {
      THROW("Slates shared labels must be of the form: slates slot [chosen_action_id:probability[,action_id:probability...]]");
    }
    ld.type = CCB::example_type::slot;

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
label_parser slates_label_parser = {default_label, parse_label, cache_label, read_cached_label, delete_label, weight,
    copy_label, test_label, sizeof(slates::label)};
}  // namespace CCB
