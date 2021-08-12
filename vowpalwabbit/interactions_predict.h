// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include "constant.h"
#include "feature_group.h"
#include "example_predict.h"
#include <vector>
#include <string>

const static std::pair<std::string, std::string> EMPTY_AUDIT_STRINGS = std::make_pair("", "");

namespace INTERACTIONS
{

template <bool audit, typename FuncT, typename AuditFuncT>
inline void do_inter(example_predict& ex, const std::vector<namespace_index>& interaction,
    namespace_index last_term,
    size_t offset, size_t current_inter_index, size_t feature_index, float value, const FuncT& func, const AuditFuncT& audit_func)
{
  assert(current_inter_index < interaction.size());
  auto current_term = interaction[current_inter_index];

  auto& current_ns = ex.feature_space[current_term];
  auto begin = current_ns.audit_cbegin();

  if (last_term == current_term) {
    assert(offset == 0 || offset < current_ns.size());
    begin += offset;
  }

  if ((interaction.size() - 1) == current_inter_index)
  {
    for (; begin != current_ns.audit_cend(); ++begin)
    {
      if (audit) { audit_func(begin.audit() == nullptr ? &EMPTY_AUDIT_STRINGS : begin.audit()); }
      auto new_value = value * begin.value();
      auto index = FNV_prime * (feature_index ^ static_cast<uint64_t>(begin.index()));
      func(index, new_value);
      if (audit) { audit_func(nullptr); }
    }
  }
  else
  {
    auto i = 0;
    for (; begin != current_ns.audit_cend(); ++begin)
    {
      if (audit) { audit_func(begin.audit() == nullptr ? &EMPTY_AUDIT_STRINGS : begin.audit()); }

      auto new_value = value * begin.value();
      auto index = FNV_prime * (feature_index ^ static_cast<uint64_t>(begin.index()));
      do_inter<audit>(ex, interaction, current_term, i, current_inter_index + 1, index, new_value, func, audit_func);
      i++;

      if (audit) { audit_func(nullptr); }
    }
  }
}

template <bool audit, class DataT, void (*FuncT)(DataT&, const float, float&), class WeightsT, typename AuditFuncT>
inline void setup_call_with_lambda(DataT& data, WeightsT& weights, example_predict& ex,
    const std::vector<namespace_index>& interactions, namespace_index last_term, size_t offset,
    size_t current_inter_index, size_t feature_index, float value, size_t& num, const AuditFuncT& audit_func)
{
  do_inter<audit>(
      ex, interactions, last_term, offset, current_inter_index, feature_index, value, [&](uint64_t index, float value) {
        num++;
        FuncT(data, value, weights[index]);
      },
      audit_func);
}

template <bool audit, class DataT, void (*FuncT)(DataT&, const float, float), class WeightsT, typename AuditFuncT>
inline void setup_call_with_lambda(DataT& data, WeightsT& weights, example_predict& ex,
    const std::vector<namespace_index>& interactions, namespace_index last_term, size_t offset,
    size_t current_inter_index, size_t feature_index, float value, size_t& num, const AuditFuncT& audit_func)
{
  do_inter<audit>(
      ex, interactions, last_term, offset, current_inter_index, feature_index, value, [&](uint64_t index, float value) {
        FuncT(data, value, weights[index]);
        num++;
      },
      audit_func);
}

template <bool audit, class DataT, void (*FuncT)(DataT&, float, uint64_t), class WeightsT, typename AuditFuncT>
inline void setup_call_with_lambda(DataT& data, WeightsT& weights, example_predict& ex,
    const std::vector<namespace_index>& interactions, namespace_index last_term, size_t offset,
    size_t current_inter_index, size_t feature_index, float value, size_t& num, const AuditFuncT& audit_func)
{
  do_inter<audit>(
      ex, interactions, last_term, offset, current_inter_index, feature_index, value, [&](uint64_t index, float value) {
        FuncT(data, value, index);
        num++;
      },
      audit_func);
}

// this templated function generates new features for given example and set of interactions
// and passes each of them to given function FuncT()
// it must be in header file to avoid compilation problems
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT), bool audit,
    void (*audit_func)(DataT&, const audit_strings*),
    class WeightsT>  // nullptr func can't be used as template param in old compilers
inline void generate_interactions(const std::vector<std::vector<namespace_index>>& interactions, bool permutations,
    example_predict& ec, DataT& dat, WeightsT& weights,
    size_t& num_features)  // default value removed to eliminate ambiguity in old complers
{
  num_features = 0;
  permutations; // Permuatations ignored...

  auto audit_func_l = [&](const audit_strings* astr) { audit_func(dat, astr); };

  for (const auto& ns : interactions)
  {  // current list of namespaces to interact.
    auto& current_ns = ec.feature_space[ns[0]];
    auto begin = current_ns.audit_cbegin();
    auto i = 0;
    for (; begin != current_ns.audit_cend(); ++begin)
    {
      if (audit) { audit_func_l(begin.audit() == nullptr ? &EMPTY_AUDIT_STRINGS : begin.audit()); }

      auto value = begin.value();
      auto index = FNV_prime * static_cast<uint64_t>(begin.index());
      setup_call_with_lambda<audit, DataT, FuncT, WeightsT>(
          dat, weights, ec, ns, ns[0], i, 1, index, value, num_features, audit_func_l);
      i++;
      if (audit) { audit_func_l(nullptr); }
    }
  }
}

}  // namespace INTERACTIONS
