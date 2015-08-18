#pragma once
#include "global_data.h"

namespace VW
{
    void validate_version(vw& all);
    void validate_min_max_label(vw& all);
    void validate_default_bits(vw& all, uint32_t local_num_bits);
    void validate_num_bits(vw& all);
}
