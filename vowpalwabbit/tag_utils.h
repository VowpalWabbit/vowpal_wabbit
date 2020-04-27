#pragma once

#include "vw_string_view.h"

struct example;

namespace VW
{
  VW::string_view* extract_random_seed(const example& ex);
}
