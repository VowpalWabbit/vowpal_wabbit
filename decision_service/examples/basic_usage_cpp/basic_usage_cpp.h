#pragma once

#include <ostream>
#include <iostream>
#include <memory>
#include <cassert>
#include <iostream>
#include <cpprest/http_listener.h>
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

#define RETURN_ON_ERROR(scode, status) do {                     \
  if( scode != r::error_code::success ) {                       \
    std::cout << status.get_error_msg() << std::endl;           \
    return scode;                                               \
  }                                                             \
} while ( 0 );                                                  \

#define RETURN_ON_ERROR_STR(scode, str) do {    \
  if( scode != 0 ) {                          \
    std::cout << str << std::endl;            \
    return scode;                             \
  }                                           \
} while ( 0 );                                \


char const * const  uuid    = "uuid";
char const * const  context = R"({"_multi":[{"n1":"v1"},{"n2":"v2"}]})";
char const * const  reward  = R"({})";
