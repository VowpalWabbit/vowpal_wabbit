// main.cc : Defines the entry point for the console application.
//

#include "example.h"

#include <memory>
#include <cassert>
#include <iostream>
#include <cpprest/http_listener.h>
#include <boost/program_options.hpp>
#include "factory_resolver.h"
#include "constants.h"
#include "config_utility.h"
#include "live_model.h"
#include "../../rlclientlib/str_util.h"

// Namespace manipulation for brevity
using namespace web::http::experimental::listener;
namespace r = reinforcement_learning;
namespace cfg_util = reinforcement_learning::utility::config;
namespace po = boost::program_options;
namespace u = reinforcement_learning::utility;

// Globals & Constants
std::unique_ptr<http_listener> g_http;
const auto HTTP_URL = U("http://localhost:8951");

// Forward declartion of functions
po::variables_map process_cmd_line(int argc, char** argv);
bool start_mock_server(const po::variables_map& vm);
void basic_usage(const po::variables_map& vm);
void override_transport_usage(const po::variables_map& vm);

// App entry point
int main(int argc, char** argv) {
  try {
    po::variables_map vm = process_cmd_line(argc, argv);
    start_mock_server(vm);
    basic_usage(vm);
    override_transport_usage(vm);
  }
  catch ( const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }  
  return 0;
}

// Implementation
po::variables_map process_cmd_line(const int argc, char** argv) {
  po::options_description desc("Options");
  desc.add_options()
    ( "local_server,l", po::value<bool>()->default_value(false)   , "Start local server to test against" )

    ( "json_config,j",  po::value<std::string>()->
                        default_value("client.json")              , "JSON file with config information for hosted RL loop" );

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  return vm;
}

std::string load_config_file(const std::string& file_name) {
  std::ifstream fs;
  fs.open(file_name);
  if ( !fs.good() )
    throw std::runtime_error(u::concat("Unable to open file. ", file_name));
  std::stringstream buffer;
  buffer << fs.rdbuf();
  return buffer.str();
}

std::unique_ptr<r::live_model> init(const po::variables_map& vm) {
  //create a configuration object from json data
  const auto json_config = vm["json_config"].as<std::string>();
  auto const config_str = load_config_file(json_config);
  auto config = cfg_util::create_from_json(config_str);
  config.set(r::name::MODEL_BLOB_URI,"http://localhost:8951");
  //create the rl live_model, and initialize it with the config
  auto rl = std::make_unique<r::live_model>(config);
  r::api_status status;
  const auto scode = rl->init(&status);
  if(scode != r::error_code::success) 
    throw std::runtime_error(status.get_error_msg());
  return rl;
}

void use_this_action(size_t choosen_action) {
  std::cout << "The choosen action is: " << choosen_action << std::endl;
}


void handle_get(web::http::http_request request) {
  web::http::http_response resp;
  auto& headers = resp.headers();
  headers.set_date(utility::datetime::utc_now());
  headers.add(U("Last-Modified"), headers.date());
  resp.set_status_code(web::http::status_codes::OK);
  resp.set_body("Test content");
  request.reply(resp);
}

bool start_mock_server(const po::variables_map& vm) {
  if ( !vm["local_server"].as<bool>() )
    return false;

  const ::utility::string_t surl(HTTP_URL);
  web::uri_builder addr(surl);
  g_http = std::make_unique<http_listener>(addr.to_uri());
  g_http->support(web::http::methods::GET, handle_get);
  g_http->support(web::http::methods::HEAD, handle_get);
  g_http->open().wait();
  return true;
}


/**
* \brief Basic usage of reinforcement learning API
*/
void basic_usage(const po::variables_map& vm) {
  // Create an interface to reinforcement learning loop 
  // and initialize it
  auto rl = init(vm);

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
