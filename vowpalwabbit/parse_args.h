// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "global_data.h"
#include "options.h"
#include "text_utils.h"
#include "vw_string_view.h"

// Used in parse_source
struct input_options
{
  bool daemon;
  bool foreground;
  size_t port;
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
#ifdef BUILD_EXTERNAL_PARSER
  // pointer because it is an incomplete type
  std::unique_ptr<VW::external::parser_options> ext_opts;
#endif
};

// trace listener + context need to be passed at initialization to capture all messages.
vw& parse_args(VW::config::options_i& options, trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
void parse_modules(VW::config::options_i& options, vw& all);
void parse_sources(VW::config::options_i& options, vw& all, io_buf& model, bool skip_model_load = false);

void merge_options_from_header_strings(const std::vector<std::string>& strings, bool skip_interactions,
    VW::config::options_i& options, bool& is_ccb_input_model);

VW_DEPRECATED("Moved and renamed: use VW::decode_inline_hex instead")
inline std::string spoof_hex_encoded_namespaces(const std::string& arg) { return VW::decode_inline_hex(arg); }

VW_DEPRECATED("Moved: use VW::ends_with instead")
inline bool ends_with(const std::string& full_string, const std::string& ending)
{
  return VW::ends_with(full_string, ending);
}

std::vector<extent_term> parse_full_name_interactions(vw& all, VW::string_view str);