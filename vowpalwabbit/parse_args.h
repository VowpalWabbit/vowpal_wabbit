// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "global_data.h"
#include "options.h"

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
};

// trace listener + context need to be passed at initialization to capture all messages.
vw& parse_args(VW::config::options_i& options, trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
void parse_modules(VW::config::options_i& options, vw& all);
void parse_sources(VW::config::options_i& options, vw& all, io_buf& model, bool skipModelLoad = false);

LEARNER::base_learner* setup_base(VW::config::options_i& options, vw& all);

std::string spoof_hex_encoded_namespaces(const std::string& arg);
