// main.cc : Defines the entry point for the console application.
//

#include "example.h"

#include "config_utility.h"
#include "live_model.h"
#include <memory>
#include <cassert>
#include <iostream>
#include "factory_resolver.h"
#include "local_file_override.h"
#include "constants.h"
#include <cpprest/http_listener.h>

// namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace cfg_util = reinforcement_learning::utility::config;
namespace util = reinforcement_learning::utility;

// forward declartion of functions
std::unique_ptr<r::live_model> init();
void use_this_action(size_t choosen_action);

const auto HTTP_URL = U("http://localhost:8951");

/**
 * \brief Basic usage of reinforcement learning API
 */
void basic_usage() {
  // Create an interface to reinforcement learning loop 
  // and initialize it
  auto rl = init();

  // Response class
  r::ranking_response response;

  // Choose an action
  auto scode = rl->choose_rank(uuid, context, response);
  assert(scode == r::error_code::success);
  size_t choosen_action;
  scode = response.get_choosen_action_id(choosen_action);
  assert(scode == r::error_code::success);

  // Use the response
  use_this_action(choosen_action);

  // Report reward recieved
  scode = rl->report_outcome(uuid, reward);
  assert(scode == r::error_code::success);
}

std::unique_ptr<r::live_model> init() {

  //create a configuration object from json data
  auto config = cfg_util::init_from_json(JSON_CFG_DATA);
  config.set(r::name::MODEL_BLOB_URI,"http://localhost:8951");
  //create the rl live_model, and initialize it with the config
  auto rl = std::make_unique<r::live_model>(config);
  rl->init(nullptr);

  return rl;
}

void use_this_action(size_t choosen_action) {
  std::cout << "The choosen action is: " << choosen_action << std::endl;
}

bool start_mock_server();

int main() {
  start_mock_server();
  basic_usage();
  override_transport_usage();

  return 0;
}

using namespace web::http::experimental::listener;
std::unique_ptr<http_listener> g_http;

void handle_get(web::http::http_request request) {
  web::http::http_response resp;
  auto& headers = resp.headers();
  headers.set_date(utility::datetime::utc_now());
  headers.add(U("Last-Modified"), headers.date());
  resp.set_status_code(web::http::status_codes::OK);
  resp.set_body("Test content");
  request.reply(resp);
}

bool start_mock_server() {
  const ::utility::string_t surl(HTTP_URL);
  web::uri_builder addr(surl);
  g_http = std::make_unique<http_listener>(addr.to_uri());
  g_http->support(web::http::methods::GET, handle_get);
  g_http->support(web::http::methods::HEAD, handle_get);
  g_http->open().wait();
  return true;
}
