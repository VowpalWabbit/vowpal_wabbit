/**
 * @brief Simple RL Inference API sample * 
 * 
 * @file basic_usage_cpp.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

#include <iostream>
#include <fstream>
#include "config_utility.h"
#include "live_model.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::config_collection& cc);

char const * const  uuid    = "uuid";
char const * const  context = R"({
                                  "GUser":{"id":"a","major":"eng","hobby":"hiking"},
                                  "_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}]})";
float reward  = 1.0f;
