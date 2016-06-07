/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"

vw& parse_args(int argc, char *argv[]);
void parse_modules(vw& all, io_buf& model);
void parse_sources(vw& all, io_buf& model);

LEARNER::base_learner* setup_base(vw& all);

string spoof_hex_encoded_namespaces(const string& arg);
// char** get_argv_from_string(string s, int& argc);
