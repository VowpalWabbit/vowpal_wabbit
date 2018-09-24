#include "console_tracer.h"
#include <iostream>

namespace reinforcement_learning {
  void console_tracer::log(const std::string& msg) {
    std::cout << msg << std::endl;
  }
}
