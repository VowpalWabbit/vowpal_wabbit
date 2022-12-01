// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#if !defined(__APPLE__) && !defined(_WIN32)
#  define __STDCPP_MATH_SPEC_FUNCS__ 201003L
#  define __STDCPP_WANT_MATH_SPEC_FUNCS__ 1
#endif

#include "vw/core/io_buf.h"
#include "vw/core/metric_sink.h"

namespace VW
{
class robust_mixture
{
public:

};

class off_policy_cs
{

};

class ddrm : off_policy_cs
{
public:
    robust_mixture lower;
    robust_mixture upper;
};

namespace model_utils
{
/*size_t read_model_field(io_buf&, VW::incremental_f_sum&);
size_t write_model_field(io_buf&, const VW::incremental_f_sum&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::confidence_sequence&);
size_t write_model_field(io_buf&, const VW::confidence_sequence&, const std::string&, bool);*/
}  // namespace model_utils
}  // namespace VW
