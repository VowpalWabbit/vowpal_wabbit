#include "config/options_cli.h"
#include "vw.h"
#include "vw/common/exception.h"

#include <memory>

int main(int argc, char** argv)
{
  std::unique_ptr<VW::config::options_i> ptr(
      new VW::config::options_cli(std::vector<std::string>(argv + 1, argv + argc)));
  try {
    VW::workspace* all = VW::initialize(*ptr);
  }
  catch(...)
  {
    exit(1);
  }
  return 0;
}
