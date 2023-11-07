// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/slates_label.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/constant.h"
#include "vw/core/model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/vw_math.h"

#include <numeric>

namespace VW
{
namespace slates
{
void default_label(slates::label& v);

float weight(const slates::label& ld) { return ld.weight; }

void default_label(slates::label& ld) { ld.reset_to_default(); }

bool test_label(const slates::label& ld) { return ld.labeled == false; }

// Slates labels come in three types, shared, action and slot with the following structure:
// slates shared [global_cost]
// slates action <slot_id>
// slates slot [chosen_action_id:probability[,action_id:probability...]]
//
// For a more complete description of the grammar, including examples see:
// https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Slates

void parse_label(slates::label& ld, VW::label_parser_reuse_mem& reuse_mem, const std::vector<VW::string_view>& words,
    VW::io::logger& logger)
{
  ld.weight = 1;

  if (words.empty()) { THROW("Slates labels may not be empty"); }
  if (!(words[0] == VW::details::SLATES_LABEL)) { THROW("Slates labels require the first word to be slates"); }

  if (words.size() == 1) { THROW("Slates labels require a type. It must be one of: [shared, action, slot]"); }

  const auto& type = words[1];
  if (type == VW::details::SHARED_TYPE)
  {
    // There is a cost defined.
    if (words.size() == 3)
    {
      ld.cost = VW::details::float_of_string(words[2], logger);
      ld.labeled = true;
    }
    else if (words.size() != 2) { THROW("Slates shared labels must be of the form: slates shared [global_cost]"); }
    ld.type = example_type::SHARED;
  }
  else if (type == VW::details::ACTION_TYPE)
  {
    if (words.size() != 3) { THROW("Slates action labels must be of the form: slates action <slot_id>"); }

    char* char_after_int = nullptr;
    ld.slot_id = VW::details::int_of_string(words[2], char_after_int, logger);
    if (char_after_int != nullptr && *char_after_int != ' ' && *char_after_int != '\0')
    {
      THROW("Slot id seems to be malformed");
    }

    ld.type = example_type::ACTION;
  }
  else if (type == VW::details::SLOT_TYPE)
  {
    if (words.size() == 3)
    {
      ld.labeled = true;
      VW::tokenize(',', words[2], reuse_mem.tokens);

      std::vector<VW::string_view> split_colons;
      for (auto& token : reuse_mem.tokens)
      {
        VW::tokenize(':', token, split_colons);
        if (split_colons.size() != 2) { THROW("Malformed action score token"); }

        // Element 0 is the action, element 1 is the probability
        ld.probabilities.push_back({static_cast<uint32_t>(VW::details::int_of_string(split_colons[0], logger)),
            VW::details::float_of_string(split_colons[1], logger)});
      }

      // If a full distribution has been given, check if it sums to 1, otherwise throw.
      if (ld.probabilities.size() > 1)
      {
        float total_pred = std::accumulate(ld.probabilities.begin(), ld.probabilities.end(), 0.f,
            [](float result_so_far, const VW::action_score& action_pred) { return result_so_far + action_pred.score; });

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
    ld.type = example_type::SLOT;
  }
  else { THROW("Unknown slates label type: " << type); }
}

label_parser slates_label_parser = {
    // default_label
    [](polylabel& label) { default_label(label.slates); },
    // parse_label
    [](polylabel& label, reduction_features& /* red_features */, VW::label_parser_reuse_mem& reuse_mem,
        const VW::named_labels* /* ldict */, const std::vector<VW::string_view>& words, VW::io::logger& logger)
    { parse_label(label.slates, reuse_mem, words, logger); },
    // cache_label
    [](const polylabel& label, const reduction_features& /* red_features */, io_buf& cache,
        const std::string& upstream_name, bool text)
    { return VW::model_utils::write_model_field(cache, label.slates, upstream_name, text); },
    // read_cached_label
    [](polylabel& label, reduction_features& /* red_features */, io_buf& cache)
    { return VW::model_utils::read_model_field(cache, label.slates); },
    // get_weight
    [](const polylabel& label, const reduction_features& /* red_features */) { return weight(label.slates); },
    // test_label
    [](const polylabel& label) { return test_label(label.slates); },
    // label type
    label_type_t::SLATES};

}  // namespace slates
}  // namespace VW

VW::string_view VW::to_string(VW::slates::example_type ex_type)
{
#define CASE(type) \
  case type:       \
    return #type;

  using namespace VW::slates;
  switch (ex_type)
  {
    CASE(example_type::UNSET)
    CASE(example_type::SHARED)
    CASE(example_type::ACTION)
    CASE(example_type::SLOT)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown example_type enum";

#undef CASE
}

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, VW::slates::label& slates)
{
  // Since read_cached_features doesn't default the label we must do it here.
  size_t bytes = 0;
  bytes += read_model_field(io, slates.type);
  bytes += read_model_field(io, slates.weight);
  bytes += read_model_field(io, slates.labeled);
  bytes += read_model_field(io, slates.cost);
  bytes += read_model_field(io, slates.slot_id);
  bytes += read_model_field(io, slates.probabilities);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::slates::label& slates, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, slates.type, upstream_name + "_type", text);
  bytes += write_model_field(io, slates.weight, upstream_name + "_weight", text);
  bytes += write_model_field(io, slates.labeled, upstream_name + "_labeled", text);
  bytes += write_model_field(io, slates.cost, upstream_name + "_cost", text);
  bytes += write_model_field(io, slates.slot_id, upstream_name + "_slot_id", text);
  bytes += write_model_field(io, slates.probabilities, upstream_name + "_probabilities", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
