#pragma once

#include "vw_string_view.h"


namespace VW
{
struct example;
bool try_extract_random_seed(const VW::example& ex, VW::string_view& view);
}
