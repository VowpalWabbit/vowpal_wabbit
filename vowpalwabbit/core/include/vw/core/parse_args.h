// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/config/options.h"
#include "vw/core/text_utils.h"
#include "vw/core/vw_fwd.h"

using namespace VW::details;

// Used in parse_source
struct input_options
{
  bool daemon;
  bool foreground;
  uint32_t port;
  std::string pid_file;
  std::string port_file;

  bool cache;
  std::vector<std::string> cache_files;
  bool json;
  bool dsjson;
  bool kill_cache;
  bool compressed;
  bool chain_hash_json;
  bool flatbuffer = false;
};

void parse_modules(VW::config::options_i& options, VW::workspace& all);
void parse_sources(VW::config::options_i& options, VW::workspace& all, io_buf& model, bool skip_model_load = false);

void merge_options_from_header_strings(const std::vector<std::string>& strings, bool skip_interactions,
    VW::config::options_i& options, bool& is_ccb_input_model);

VW_DEPRECATED("Moved and renamed: use VW::decode_inline_hex instead")
std::string spoof_hex_encoded_namespaces(const std::string& arg);

VW_DEPRECATED("Moved: use VW::ends_with instead")
inline bool ends_with(const std::string& full_string, const std::string& ending)
{
  return VW::ends_with(full_string, ending);
}

std::vector<extent_term> parse_full_name_interactions(VW::workspace& all, VW::string_view str);

namespace VW
{
namespace details
{
/**
 * @brief Extract namespace, feature name, and optional feature value from ignored feature string
 *
 * @param namespace_feature namespace|feature:feature_value. Feature value is optional and if it is supplied chain_hash
 * is applied
 * @return std::tuple<std::string, std::string> (namespace, feature)
 */
std::tuple<std::string, std::string> extract_ignored_feature(VW::string_view namespace_feature);
}  // namespace details
}  // namespace VW
