/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"

// trace listener + context need to be passed at initialization to capture all messages.
vw& parse_args(int argc, char *argv[], trace_message_t trace_listener = nullptr, void* trace_context = nullptr);
void parse_modules(vw& all, io_buf& model);
void parse_sources(vw& all, io_buf& model, bool skipModelLoad = false);

LEARNER::base_learner* setup_base(vw& all);

std::string spoof_hex_encoded_namespaces(const std::string& arg);
// char** get_argv_from_string(string s, int& argc);
