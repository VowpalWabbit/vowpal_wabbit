#include "config/options_boost_po.h"

#include "vw.h"
#include "vw_exception.h"
#include <memory>

int main(int argc, char** argv)
{
  std::unique_ptr<VW::config::options_i> ptr(new VW::config::options_boost_po(argc, argv));
  try {
    VW::workspace* all = VW::initialize(*ptr);
  }
  catch(...)
  {
    exit(1);
  }
  return 0;
}
