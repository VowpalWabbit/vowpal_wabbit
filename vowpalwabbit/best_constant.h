#pragma once
#include <float.h>
#include "vw.h"

extern bool  is_more_than_two_labels_observed;
extern float first_observed_label;
extern float second_observed_label;

inline void count_label(float l)
{
    if (is_more_than_two_labels_observed || l == FLT_MAX) return;

    if (first_observed_label != FLT_MAX)
    {

        if (first_observed_label != l)
        {
            if (second_observed_label != FLT_MAX)
            {
                if (second_observed_label != l)
                    is_more_than_two_labels_observed = true;

            } else second_observed_label = l;
        }

    } else first_observed_label = l;

}

bool get_best_constant(vw& all, float& best_constant, float& best_constant_loss);
