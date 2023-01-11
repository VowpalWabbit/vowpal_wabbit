#include "vw/config/options_cli.h"
#include "vw/core/memory.h"
#include "vw/core/vw.h"

int main()
{
  auto workspace = VW::initialize(VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--quiet"}));
  workspace->finish();
}
