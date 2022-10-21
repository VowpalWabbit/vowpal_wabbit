#pragma once

#include "prelude.h"
#include "typesys.h"

namespace autofb
{
  using typesys::universe;

  void generate_universe_types(
    const std::string& schema_ns, 
    const std::string& output_dir,
    universe& types = universe::instance()
  );

  void print_universe_types(
    const std::string& schema_ns, 
    universe& types = universe::instance()
  );
}