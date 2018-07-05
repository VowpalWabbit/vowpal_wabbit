// rl_sim_cpp.cpp : Defines the entry point for the console application.
//

#include <exception>
#include <iostream>
#include <boost/program_options.hpp>
#include "rl_sim.h"

// Shorter namespaces
namespace po = boost::program_options;

int main(int argc, char ** argv);

// Forward declare functions
po::variables_map process_cmd_line(const int argc, char** argv);
bool is_help(const po::variables_map& vm);
void init_rl(const po::variables_map& vm);

// Entry point
int main(int argc, char** argv) {
  try {
    const auto vm = process_cmd_line(argc, argv);
    if ( is_help(vm) ) return 0;
    rl_sim sim(vm);
    sim.loop();
  }
  catch ( const std::exception& e ) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}

// Implementation
po::variables_map process_cmd_line(const int argc, char** argv) {
  po::options_description desc("Options");
  desc.add_options()
    ( "help", "produce help message" )
    ( "json_config,j", po::value<std::string>()->
      default_value("client.json"), "JSON file with config information for hosted RL loop" );

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if ( is_help(vm) )
    std::cout << desc << std::endl;

  return vm;
}

bool is_help(const po::variables_map& vm) {
  return vm.count("help") > 0;
}

