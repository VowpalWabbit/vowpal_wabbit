// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/text_utils.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace details
{
// Used in parse_source
class input_options
{
public:
  bool daemon;
  bool foreground;
  uint32_t port;
  std::string pid_file;
  std::string port_file;
  uint64_t num_children;
  // If a model was saved in daemon or active learning mode, force it to accept
  // local input when loaded instead.
  bool no_daemon = false;

  bool cache;
  std::vector<std::string> cache_files;
  bool json;
  bool dsjson;
  bool kill_cache;
  bool compressed;
  bool chain_hash_json;
  bool flatbuffer = false;
#ifdef VW_BUILD_CSV
  std::unique_ptr<VW::parsers::csv::csv_parser_options> csv_opts;
#endif
  bool stdin_off = false;
};

void merge_options_from_header_strings(const std::vector<std::string>& strings, bool skip_interactions,
    VW::config::options_i& options, bool& is_ccb_input_model);

std::vector<extent_term> parse_full_name_interactions(VW::workspace& all, VW::string_view str);

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

VW_DEPRECATED("Moved and renamed: use VW::decode_inline_hex instead")
std::string spoof_hex_encoded_namespaces(const std::string& arg);

VW_DEPRECATED("Moved: use VW::ends_with instead")
inline bool ends_with(const std::string& full_string, const std::string& ending)
{
  return VW::ends_with(full_string, ending);
}