#pragma once

#include <ostream>
#include <memory>
#include <cassert>
#include <iostream>
#include <cpprest/http_listener.h>
#include <chrono>
#include "factory_resolver.h"
#include "config_utility.h"
#include "live_model.h"

// Namespace manipulation for brevity
using namespace web::http::experimental::listener;
namespace r = reinforcement_learning;
namespace cfg = reinforcement_learning::utility::config;
namespace u = reinforcement_learning::utility;

int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::config_collection& cc);

#define RETURN_ON_ERROR(scodeexpr, status) do {                 \
  const auto __FILE__##scode = (scodeexpr);                     \
  if( __FILE__##scode != r::error_code::success ) {             \
    std::cout << status.get_error_msg() << std::endl;           \
    return __FILE__##scode;                                     \
  }                                                             \
} while ( 0 );                                                  \

#define RETURN_ON_ERROR_STR(scodeexpr, str) do {                \
  const auto __FILE__##scode = (scodeexpr);                     \
  if( __FILE__##scode != 0 ) {                                  \
    std::cout << str << std::endl;                              \
    return __FILE__##scode;                                     \
  }                                                             \
} while ( 0 );                                                  \


char const * const  uuid    = "uuid";
char const * const  context = R"({
                                  "User":{"id":"a","major":"eng","hobby":"hiking"},
                                  "_multi":[{"a1":"f1"},{"a2":"f2"}]})";
char const * const  reward  = R"({})";
