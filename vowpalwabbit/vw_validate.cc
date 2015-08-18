#pragma once

#include "vw_validate.h"

namespace VW
{
    void validate_min_max_label(vw& all)
    {
        if (all.sd->max_label < all.sd->min_label)
        {
            THROW("Max label cannot be less than min label.");
        }
    }

    void validate_default_bits(vw& all, uint32_t local_num_bits)
    {
        if (all.default_bits != true && all.num_bits != local_num_bits)
        {
            THROW("-b bits mismatch: command-line " << all.num_bits << " != " << local_num_bits << " stored in model");
        }
    }

    void validate_num_bits(vw& all)
    {
        if (all.num_bits > min(31, sizeof(size_t) * 8 - 3))
        {
            THROW("Only " << min(31, sizeof(size_t) * 8 - 3) << " or fewer bits allowed.  If this is a serious limit, speak up.");
        }
    }
}