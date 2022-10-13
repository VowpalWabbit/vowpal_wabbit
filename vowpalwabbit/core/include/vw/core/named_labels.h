// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/string_view.h"
#include "vw/core/vw_fwd.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace VW
{
class named_labels
{
public:
  named_labels(std::string label_list);

  named_labels(const named_labels& other);
  named_labels& operator=(const named_labels& other);

  // If this object is moved then the internal pointers into the m_label_list string would be invalidated.
  // If the data were to be on the heap it should be okay but a small string optimization would break this.
  named_labels(named_labels&& other) noexcept = delete;
  named_labels& operator=(named_labels&& other) noexcept = delete;

  uint32_t getK() const;
  uint32_t get(string_view s, VW::io::logger& logger) const;
  string_view get(uint32_t v) const;

private:
  // NOTE: This ordering is critical. m_id2name and m_name2id contain pointers into m_label_list!
  std::string m_label_list;
  std::vector<string_view> m_id2name;
  std::unordered_map<string_view, uint32_t> m_name2id;
  uint32_t m_K;

  void initialize_maps_from_input_string();
};
}  // namespace VW
