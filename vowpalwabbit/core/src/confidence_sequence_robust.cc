// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include "vw/core/model_utils.h"

#include <cassert>
#include <cmath>

namespace VW
{
void lower_cs_base::add_obs(double x)
{
    THROW("Unimplemented");
}

double lower_cs_base::get_ci(double alpha)
{
    THROW("Unimplemented");
}

double countable_discrete_base::lambertw(double)
{
    // TODO: Find cpp equivalent
    return 0.0;
}

countable_discrete_base::countable_discrete_base(double lambda_max, double xi)
{
    assert(0.0 < lambda_max && lambda_max <= 1.0 + lambertw(-std::exp(-2.0)));
    assert(1.0 < xi);

    this->lambda_max = lambda_max;
    this->xi = xi;
    this->log_xi = std::log1p(xi - 1);
}

double countable_discrete_base::get_ci(double alpha)
{
    return lb_log_wealth(alpha);
}

double countable_discrete_base::get_lam_sqrt_tp1(double j)
{
    double log_den = (j + 0.5) * log_xi - 0.5 * std::log(t + 1);
    return lambda_max * std::exp(-log_den);
}




void off_policy_cs::add_obs(double w, double r)
{
    assert(w >= 0.0);
    assert(0.0 <= r && r <= 1.0);
    lower.add_obs(w * r);
    upper.add_obs(w * (1.0 - r));
}

std::pair<double, double> off_policy_cs::get_ci(double alpha)
{
    return std::make_pair(lower.get_ci(alpha / 2.0), upper.get_ci(alpha / 2.0));
}

namespace model_utils
{

}  // namespace model_utils
}  // namespace VW