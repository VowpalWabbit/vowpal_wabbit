// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "slates_label.h"

#include "cache.h"
#include "parser.h"
#include "vw_string_view.h"
#include "constant.h"
#include "vw_math.h"
#include <numeric>
namespace VW
{
namespace slates
{
void default_label(void* v);

#define READ_CACHED_VALUE(DEST, TYPE)                                      \
  next_read_size = sizeof(TYPE);                                           \
  if (cache.buf_read(read_ptr, next_read_size) < next_read_size) return 0; \
  DEST = *(TYPE*)read_ptr;                                                 \
  read_count += sizeof(TYPE);

#define WRITE_CACHED_VALUE(VALUE, TYPE) \
  *(TYPE*)c = VALUE;                    \
  c += sizeof(TYPE);

size_t read_cached_label(shared_data* /*sd*/, void* v, io_buf& cache)
{
  // Since read_cached_features doesn't default the label we must do it here.
  default_label(v);
  auto* ld = static_cast<slates::label*>(v);

  size_t read_count = 0;
  char* read_ptr;
  size_t next_read_size = 0;

  READ_CACHED_VALUE(ld->type, slates::example_type);
  READ_CACHED_VALUE(ld->weight, float);
  READ_CACHED_VALUE(ld->labeled, bool);
  READ_CACHED_VALUE(ld->cost, float);
  READ_CACHED_VALUE(ld->slot_id, uint32_t);

  uint32_t size_probs = 0;
  READ_CACHED_VALUE(size_probs, uint32_t);

  for (uint32_t i = 0; i < size_probs; i++)
  {
    ACTION_SCORE::action_score a_s;
    READ_CACHED_VALUE(a_s, ACTION_SCORE::action_score);
    ld->probabilities.push_back(a_s);
  }
  return read_count;
}

void cache_label(void* v, io_buf& cache)
{
  char* c;
  auto* ld = static_cast<slates::label*>(v);
  size_t size = sizeof(ld->type) + sizeof(ld->weight) + sizeof(ld->labeled) + sizeof(ld->cost) + sizeof(ld->slot_id) +
      sizeof(uint32_t)  // Size of probabilities
      + sizeof(ACTION_SCORE::action_score) * ld->probabilities.size();

  cache.buf_write(c, size);
  WRITE_CACHED_VALUE(ld->type, slates::example_type);
  WRITE_CACHED_VALUE(ld->weight, float);
  WRITE_CACHED_VALUE(ld->labeled, bool);
  WRITE_CACHED_VALUE(ld->cost, float);
  WRITE_CACHED_VALUE(VW::convert(ld->slot_id), uint32_t);
  WRITE_CACHED_VALUE(VW::convert(ld->probabilities.size()), uint32_t);
  for (const auto& score : ld->probabilities) { WRITE_CACHED_VALUE(score, ACTION_SCORE::action_score); }
}

float weight(void* v) { return static_cast<polylabel*>(v)->slates.weight; }

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
  ld.probabilities.delete_v();
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

// Slates labels come in three types, shared, action and slot with the following structure:
// slates shared [global_cost]
// slates action <slot_id>
// slates slot [chosen_action_id:probability[,action_id:probability...]]
//
// For a more complete description of the grammar, including examples see:
// https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Slates
void parse_label(parser* p, shared_data* /*sd*/, void* v, std::vector<VW::string_view>& words)
{
  auto& ld = static_cast<polylabel*>(v)->slates;
  ld.weight = 1;

  if (words.empty()) { THROW("Slates labels may not be empty"); }
  if (!(words[0] == SLATES_LABEL)) { THROW("Slates labels require the first word to be slates"); }

  if (words.size() == 1) { THROW("Slates labels require a type. It must be one of: [shared, action, slot]"); }

  const auto& type = words[1];
  if (type == SHARED_TYPE)
  {
    // There is a cost defined.
    if (words.size() == 3)
    {
      ld.cost = float_of_string(words[2]);
      ld.labeled = true;
    }
    else if (words.size() != 2)
    {
      THROW("Slates shared labels must be of the form: slates shared [global_cost]");
    }
    ld.type = example_type::shared;
  }
  else if (type == ACTION_TYPE)
  {
    if (words.size() != 3) { THROW("Slates action labels must be of the form: slates action <slot_id>"); }

    char* char_after_int = nullptr;
    ld.slot_id = int_of_string(words[2], char_after_int);
    if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
    { THROW("Slot id seems to be malformed"); }

    ld.type = example_type::action;
  }
  else if (type == SLOT_TYPE)
  {
    if (words.size() == 3)
    {
      ld.labeled = true;
      tokenize(',', words[2], p->parse_name);

      std::vector<VW::string_view> split_colons;
      for (auto& token : p->parse_name)
      {
        tokenize(':', token, split_colons);
        if (split_colons.size() != 2) { THROW("Malformed action score token"); }

        // Element 0 is the action, element 1 is the probability
        ld.probabilities.push_back(
            {static_cast<uint32_t>(int_of_string(split_colons[0])), float_of_string(split_colons[1])});
      }

      // If a full distribution has been given, check if it sums to 1, otherwise throw.
      if (ld.probabilities.size() > 1)
      {
        float total_pred = std::accumulate(ld.probabilities.begin(), ld.probabilities.end(), 0.f,
            [](float result_so_far, const ACTION_SCORE::action_score& action_pred) {
              return result_so_far + action_pred.score;
            });

        if (!VW::math::are_same(total_pred, 1.f))
        {
          THROW(
              "When providing all prediction probabilities they must add up to 1.0, instead summed to " << total_pred);
        }
      }
    }
    else if (words.size() > 3)
    {
      THROW(
          "Slates shared labels must be of the form: slates slot "
          "[chosen_action_id:probability[,action_id:probability...]]");
    }
    ld.type = example_type::slot;
  }
  else
  {
    THROW("Unknown slates label type: " << type);
  }
}

// Export the definition of this label parser.
label_parser slates_label_parser = {
    default_label, parse_label, cache_label, read_cached_label, delete_label, weight, copy_label, test_label};
}  // namespace slates
}  // namespace VW