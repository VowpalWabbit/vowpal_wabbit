#pragma once

#include "vw_string_view.h"

struct example;

namespace VW
{
bool try_extract_random_seed(const example& ex, VW::string_view& view);
}
