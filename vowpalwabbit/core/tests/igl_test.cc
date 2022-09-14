// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
using namespace ::testing;

#include "vw/test_common/test_common.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/constant.h"  // FNV_prime
#include "vw/core/vw_math.h"
#include "vw/core/merge.h"

#include "vw/core/vw.h"
#include "vw/core/parser.h"

#include <gtest/gtest.h>

#include <map>
#include <iostream>

using example_vector = std::vector<std::vector<std::string>>;
using ftrl_weights_vector = std::vector<std::tuple<float, float, float, float, float, float>>;
using separate_weights_vector = std::vector<std::tuple<size_t, float, float, float, float, float, float>>;
int ex_num = 50;

example_vector sl_vector = {
{
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
},
{
"1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=3 |v v=click",
"-1 0.6 |c id=1 |a id=4 |v v=click",
"-1 0.6 |c id=1 |a id=0 |v v=click",
"-1 0.6 |c id=1 |a id=2 |v v=click",
"-1 0.6 |c id=1 |a id=1 |v v=click",
"-1 0.6 |c id=1 |a id=5 |v v=click",
"1 0.6 |c id=1 |a id=6 |v v=click",
},
{
"-1 0.6 |c id=1 |a id=0 |v v=none",
"1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=like",
"1 0.6 |c id=0 |a id=1 |v v=like",
"-1 0.6 |c id=0 |a id=6 |v v=like",
"-1 0.6 |c id=0 |a id=4 |v v=like",
},
{
"-1 0.6 |c id=0 |a id=2 |v v=banana",
"-1 0.6 |c id=0 |a id=6 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
},
{
"1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"1 0.6 |c id=0 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=3 |v v=like",
"-1 0.6 |c id=1 |a id=1 |v v=like",
"-1 0.6 |c id=1 |a id=2 |v v=like",
"-1 0.6 |c id=1 |a id=5 |v v=like",
"-1 0.6 |c id=1 |a id=4 |v v=like",
"1 0.6 |c id=1 |a id=6 |v v=like",
"-1 0.6 |c id=1 |a id=0 |v v=like",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"-1 0.6 |c id=0 |a id=5 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
},
{
"-1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"-1 0.6 |c id=0 |a id=2 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
"-1 0.6 |c id=0 |a id=3 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
},
{
"-1 0.6 |c id=1 |a id=1 |v v=banana",
"-1 0.6 |c id=1 |a id=2 |v v=banana",
"-1 0.6 |c id=1 |a id=4 |v v=banana",
"1 0.6 |c id=1 |a id=0 |v v=banana",
"-1 0.6 |c id=1 |a id=5 |v v=banana",
"-1 0.6 |c id=1 |a id=6 |v v=banana",
},
{
"-1 0.6 |c id=1 |a id=4 |v v=banana",
"-1 0.6 |c id=1 |a id=5 |v v=banana",
"-1 0.6 |c id=1 |a id=6 |v v=banana",
"-1 0.6 |c id=1 |a id=2 |v v=banana",
"1 0.6 |c id=1 |a id=0 |v v=banana",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
},
{
"-1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
"-1 0.6 |c id=1 |a id=3 |v v=none",
"1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=1 |v v=dislike",
"-1 0.6 |c id=1 |a id=4 |v v=dislike",
"1 0.6 |c id=1 |a id=0 |v v=dislike",
"-1 0.6 |c id=1 |a id=6 |v v=dislike",
},
{
"1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=click",
"-1 0.6 |c id=0 |a id=2 |v v=click",
"-1 0.6 |c id=0 |a id=4 |v v=click",
"-1 0.6 |c id=0 |a id=0 |v v=click",
"-1 0.6 |c id=0 |a id=6 |v v=click",
"-1 0.6 |c id=0 |a id=5 |v v=click",
"1 0.6 |c id=0 |a id=1 |v v=click",
},
{
"1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=2 |v v=like",
"-1 0.6 |c id=1 |a id=4 |v v=like",
"-1 0.6 |c id=1 |a id=5 |v v=like",
"1 0.6 |c id=1 |a id=6 |v v=like",
"-1 0.6 |c id=1 |a id=0 |v v=like",
},
{
"-1 0.6 |c id=1 |a id=4 |v v=banana",
"-1 0.6 |c id=1 |a id=2 |v v=banana",
"1 0.6 |c id=1 |a id=0 |v v=banana",
"-1 0.6 |c id=1 |a id=6 |v v=banana",
},
{
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=0 |v v=none",
"1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=none",
"1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=5 |v v=none",
"1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=6 |v v=none",
"1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=1 |v v=none",
"1 0.6 |c id=1 |a id=3 |v v=none",
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=4 |v v=click",
"-1 0.6 |c id=1 |a id=2 |v v=click",
"1 0.6 |c id=1 |a id=6 |v v=click",
"-1 0.6 |c id=1 |a id=1 |v v=click",
"-1 0.6 |c id=1 |a id=0 |v v=click",
"-1 0.6 |c id=1 |a id=5 |v v=click",
"-1 0.6 |c id=1 |a id=3 |v v=click",
},
{
"-1 0.6 |c id=0 |a id=1 |v v=none",
"1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
},
{
"-1 0.6 |c id=1 |a id=3 |v v=banana",
"1 0.6 |c id=1 |a id=0 |v v=banana",
"-1 0.6 |c id=1 |a id=6 |v v=banana",
"-1 0.6 |c id=1 |a id=2 |v v=banana",
"-1 0.6 |c id=1 |a id=1 |v v=banana",
"-1 0.6 |c id=1 |a id=4 |v v=banana",
},
{
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"-1 0.6 |c id=0 |a id=5 |v v=banana",
"-1 0.6 |c id=0 |a id=2 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
},
{
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=2 |v v=banana",
"-1 0.6 |c id=0 |a id=6 |v v=banana",
"-1 0.6 |c id=0 |a id=3 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"-1 0.6 |c id=0 |a id=5 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
},
{
"-1 0.6 |c id=1 |a id=5 |v v=none",
"-1 0.6 |c id=1 |a id=4 |v v=none",
"-1 0.6 |c id=1 |a id=1 |v v=none",
"-1 0.6 |c id=1 |a id=3 |v v=none",
"1 0.6 |c id=1 |a id=2 |v v=none",
"-1 0.6 |c id=1 |a id=0 |v v=none",
"-1 0.6 |c id=1 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=4 |v v=none",
"1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=2 |v v=banana",
"-1 0.6 |c id=0 |a id=1 |v v=banana",
"-1 0.6 |c id=0 |a id=0 |v v=banana",
"1 0.6 |c id=0 |a id=4 |v v=banana",
},
{
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
"-1 0.6 |c id=0 |a id=0 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"1 0.6 |c id=0 |a id=6 |v v=none",
},
{
"-1 0.6 |c id=0 |a id=3 |v v=none",
"-1 0.6 |c id=0 |a id=5 |v v=none",
"-1 0.6 |c id=0 |a id=2 |v v=none",
"1 0.6 |c id=0 |a id=6 |v v=none",
"-1 0.6 |c id=0 |a id=4 |v v=none",
"-1 0.6 |c id=0 |a id=1 |v v=none",
},
};

std::vector<std::string> multi_vector = {
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [0, 4, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [3, 5, 6, 4, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [2, 4, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [4, 2, 3, 0, 6, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "click"}}], "a": [3, 4, 0, 2, 1, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [0, 2, 4, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 5, 0, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 4, 0, 3, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "like"}}], "a": [3, 1, 6, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [2, 6, 1, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 2, 1, 3, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [3, 2, 4, 1, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "like"}}], "a": [3, 1, 2, 5, 4, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "banana"}}], "a": [6, 4, 0, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [3, 4, 0, 1, 5, 6, 2], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "banana"}}], "a": [6, 0, 2, 1, 3, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "banana"}}], "a": [1, 2, 4, 0, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [4, 5, 6, 2, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "banana"}}], "a": [3, 0, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [1, 5, 6, 3, 4, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [5, 2, 0, 3, 4, 1, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "dislike"}}], "a": [1, 4, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 4, 2, 0, 3, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "click"}}], "a": [3, 2, 4, 0, 6, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 3, 6, 2, 0, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 5, 0, 2, 3, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "like"}}], "a": [2, 4, 5, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "banana"}}], "a": [4, 2, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [2, 4, 3, 0, 5, 6, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [0, 4, 5, 3, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [6, 2, 3, 4, 0, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [0, 4, 3, 1, 5, 2, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [5, 0, 6, 2, 3, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [3, 5, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [5, 4, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [6, 0, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 3, 1, 6, 0, 4, 2], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [1, 3, 5, 0, 4, 2, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "click"}}], "a": [4, 2, 6, 1, 0, 5, 3], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [1, 5, 4, 6, 0, 3, 2], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "banana"}}], "a": [3, 0, 6, 2, 1, 4], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [0, 5, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [0, 1, 6, 5, 2, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [3, 4, 6, 1, 2, 0], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [0, 1, 4, 2, 5, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 1.0, "_label_probability": 0.14285714285714285, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "banana"}}], "a": [2, 6, 3, 0, 5, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 1.0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [5, 4, 1, 3, 2, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [4, 3, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 1.0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "banana"}}], "a": [2, 1, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 1.0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "none"}}], "a": [4, 1, 0, 5, 3, 2, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [3, 5, 2, 6, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
};

std::vector<std::string> igl_dsjson_vector = {
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [0, 4, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [3, 5, 6, 4, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [2, 4, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"v": {"v=none": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [4, 2, 3, 0, 6, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "click"}}], "a": [3, 4, 0, 2, 1, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"v": {"v=click": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [0, 2, 4, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 5, 0, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 4, 0, 3, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "like"}}], "a": [3, 1, 6, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"v": {"v=like": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [2, 6, 1, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 2, 1, 3, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [3, 2, 4, 1, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "like"}}], "a": [3, 1, 2, 5, 4, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"v": {"v=like": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "banana"}}], "a": [6, 4, 0, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [3, 4, 0, 1, 5, 6, 2], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "banana"}}], "a": [6, 0, 2, 1, 3, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "banana"}}], "a": [1, 2, 4, 0, 5, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [4, 5, 6, 2, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "banana"}}], "a": [3, 0, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [1, 5, 6, 3, 4, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [5, 2, 0, 3, 4, 1, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "dislike"}}], "a": [1, 4, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"v": {"v=dislike": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 4, 2, 0, 3, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "click"}}], "a": [3, 2, 4, 0, 6, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"v": {"v=click": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 3, 6, 2, 0, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [6, 5, 0, 2, 3, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "like"}}], "a": [2, 4, 5, 6, 0], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"v": {"v=like": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "banana"}}], "a": [4, 2, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [2, 4, 3, 0, 5, 6, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [0, 4, 5, 3, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [6, 2, 3, 4, 0, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [0, 4, 3, 1, 5, 2, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [5, 0, 6, 2, 3, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [3, 5, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [5, 4, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [6, 0, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 1, "_labelIndex": 0, "o": [{"v": {"v": "none"}}], "a": [5, 3, 1, 6, 0, 4, 2], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [1, 3, 5, 0, 4, 2, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "click"}}], "a": [4, 2, 6, 1, 0, 5, 3], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"v": {"v=click": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [1, 5, 4, 6, 0, 3, 2], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=1": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=0": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "banana"}}], "a": [3, 0, 6, 2, 1, 4], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.2, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "banana"}}], "a": [0, 5, 2, 1, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.2, 0.2, 0.2, 0.2, 0.2], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [0, 1, 6, 5, 2, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=6": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=4": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 3, "_labelIndex": 2, "o": [{"v": {"v": "none"}}], "a": [3, 4, 6, 1, 2, 0], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=4": 1}}, {"a": {"id=6": 1}}, {"a": {"id=1": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [0, 1, 4, 2, 5, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=0": 1}}, {"a": {"id=1": 1}}, {"a": {"id=4": 1}}, {"a": {"id=2": 1}}, {"a": {"id=5": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
R"({"_label_cost": 1.0, "_label_probability": 0.14285714285714285, "_label_Action": 6, "_labelIndex": 5, "o": [{"v": {"v": "banana"}}], "a": [2, 6, 3, 0, 5, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=3": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 1.0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 5, "_labelIndex": 4, "o": [{"v": {"v": "none"}}], "a": [5, 4, 1, 3, 2, 0, 6], "c": {"c": {"id=1": 1}, "_multi": [{"a": {"id=5": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=0": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.25, "_label_Action": 2, "_labelIndex": 1, "o": [{"v": {"v": "none"}}], "a": [4, 3, 5, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 0})",
R"({"_label_cost": 1.0, "_label_probability": 0.25, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "banana"}}], "a": [2, 1, 0, 4], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=2": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=4": 1}}, {"v": {"v=banana": 1}}]}, "p": [0.25, 0.25, 0.25, 0.25], "_original_label_cost": 1.0})",
R"({"_label_cost": 0, "_label_probability": 0.14285714285714285, "_label_Action": 7, "_labelIndex": 6, "o": [{"v": {"v": "none"}}], "a": [4, 1, 0, 5, 3, 2, 6], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"a": {"id=0": 1}}, {"a": {"id=5": 1}}, {"a": {"id=3": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"v": {"v=none": 1}}]}, "p": [0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285, 0.14285714285714285], "_original_label_cost": 0})",
R"({"_label_cost": 0, "_label_probability": 0.16666666666666666, "_label_Action": 4, "_labelIndex": 3, "o": [{"v": {"v": "none"}}], "a": [3, 5, 2, 6, 4, 1], "c": {"c": {"id=0": 1}, "_multi": [{"a": {"id=3": 1}}, {"a": {"id=5": 1}}, {"a": {"id=2": 1}}, {"a": {"id=6": 1}}, {"a": {"id=4": 1}}, {"a": {"id=1": 1}}, {"v": {"v=none": 1}}]}, "p": [0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666, 0.16666666666666666], "_original_label_cost": 0})",
};

example_vector get_multiline_examples(size_t num) {
  example_vector multi_ex_vector = {
    {
      "shared |User user=Tom time_of_day=morning",
      "0:0.5:0.8 |Action article=sports",
      " |Action article=politics",
      " |Action article=music"
    },
    {
      "shared |User user=Anna time_of_day=afternoon",
      " |Action article=sports",
      "0:-1:0.1 |Action article=politics",
      " |Action article=music"
    }
  };
  if (multi_ex_vector.begin() + num > multi_ex_vector.end()) {
    THROW("number of examples is not valid");
  }
  example_vector result = {multi_ex_vector.begin(), multi_ex_vector.begin() + num};
  return result;
}

example_vector get_sl_examples(size_t num) {
  example_vector sl_ex_vector = {
    {
      "1 0.6 |User user=Tom time_of_day=morning |Action article=sports |Feedback click:1",
      "-1 0.6 |User user=Tom time_of_day=morning |Action article=politics |Feedback click:1",
      "-1 0.6 |User user=Tom time_of_day=morning |Action article=music |Feedback click:1"
    },
    {
      "-1 0.6 |User user=Anna time_of_day=afternoon |Action article=sports |Feedback click:1",
      "1 0.6 |User user=Anna time_of_day=afternoon |Action article=politics |Feedback click:1",
      "-1 0.6 |User user=Anna time_of_day=afternoon |Action article=music |Feedback click:1",
    }
  };

  if (sl_ex_vector.begin() + num > sl_ex_vector.end()) {
    THROW("number of examples is not valid");
  }
  example_vector result = {sl_ex_vector.begin(), sl_ex_vector.begin() + num};
  return result;
}

std::vector<std::string> get_dsjson_examples(size_t num) {
  std::vector<std::string> dsjson_ex_vector = {
    R"({
      "_label_cost": 0.5,
      "_label_probability": 0.8,
      "_label_Action": 1,
      "_labelIndex": 0,
      "o": [
        {
          "v": 1,
          "EventId": "4b49f8f0-92fc-401f-ad08-13fddd99a5cf",
          "ActionTaken": false
        }
      ],
      "Timestamp": "2022-03-09T00:31:34.0000000Z",
      "Version": "1",
      "EventId": "4b49f8f0-92fc-401f-ad08-13fddd99a5cf",
      "a": [
        0,
        1,
        2
      ],
      "c": {
        "User": {
          "user=Tom": "",
          "time_of_day=morning": ""
        },
        "_multi": [
          {
            "Action": {
              "article=sports": ""
            }
          },
          {
            "Action": {
              "article=politics": ""
            }
          },
          {
            "Action": {
              "article=music": ""
            }
          }
        ]
      },
      "p": [
        0.8,
        0.1,
        0.1
      ],
      "VWState": {
        "m": "N/A"
      }
    })",
    R"({
      "_label_cost": 0,
      "_label_probability": 0.1,
      "_label_Action": 2,
      "_labelIndex": 1,
      "o": [
        {
          "v": 1,
          "EventId": "4b49f8f0-92fc-401f-ad08-13fddd99a5cf",
          "ActionTaken": false
        }
      ],
      "Timestamp": "2022-03-11T00:31:34.0000000Z",
      "Version": "1",
      "EventId": "5678f8f0-92fc-401f-ad08-13fddd99a5cf",
      "a": [
        0,
        1,
        2
      ],
      "c": {
        "User": {
          "user=Anna": "",
          "time_of_day=afternoon": ""
        },
        "_multi": [
          {
            "Action": {
              "article=sports": ""
            }
          },
          {
            "Action": {
              "article=politics": ""
            }
          },
          {
            "Action": {
              "article=music": ""
            }
          }
        ]
      },
      "p": [
        0.8,
        0.1,
        0.1
      ],
      "VWState": {
        "m": "N/A"
      }
    })"
  };
  if (dsjson_ex_vector.begin() + num > dsjson_ex_vector.end()) {
    THROW("number of examples is not valid");
  }
  std::vector<std::string> result = {dsjson_ex_vector.begin(), dsjson_ex_vector.begin() + num};
  return result;
}

void print_weights(ftrl_weights_vector weights_vector) {
  for (auto& weights:weights_vector) {
    std::cout << std::get<0>(weights) << " "
      << std::get<1>(weights) << " "
      << std::get<2>(weights) << " "
      << std::get<3>(weights) << " "
      << std::get<4>(weights) << " "
      << std::get<5>(weights) << " "
    << std::endl;
  }
}

void print_separate_weights(separate_weights_vector weights_vector) {
  std::cout << std::hexfloat << std::endl;
  for (auto& weights:weights_vector) {
    std::cout << std::get<0>(weights) << " "
      << std::get<1>(weights) << " "
      << std::get<2>(weights) << " "
      << std::get<3>(weights) << " "
      << std::get<4>(weights) << " "
      << std::get<5>(weights) << " "
      << std::get<6>(weights) << " "
    << std::endl;
  }
}

std::vector<size_t> get_hash(separate_weights_vector weights_vector) {
  std::vector<size_t> result;
  for (auto& weights:weights_vector) {
    result.emplace_back(std::get<0>(weights));
  }
  return result;
}

ftrl_weights_vector get_weights(VW::workspace* vw) {
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();

  ftrl_weights_vector weights_vector;
  if (*iter != 0.0f) {
    weights_vector.emplace_back(*iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
  }

  auto end = weights.end();
  // TODO: next_non_zero will skip the entire row of weights if the first element is 0
  // need to fix that
  while (iter.next_non_zero(end) < end) {
    weights_vector.emplace_back(*iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
  }
  std::sort(weights_vector.begin(), weights_vector.end());
  VW::finish(*vw);
  return weights_vector;
}

separate_weights_vector get_separate_weights(VW::workspace* vw) {
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();
  auto end = weights.end();

  separate_weights_vector weights_vector;

  while(iter < end) {
    bool non_zero = false;
    for (int i=0; i < 6; i++) {
      if (*iter[i] != 0.f) {
        non_zero = true;
      }
    }

    if (non_zero) {
      weights_vector.emplace_back(iter.index_without_stride(), *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
    }
    ++iter;
  }

  // print_separate_weights(weights_vector);
  return weights_vector;
}

std::vector<separate_weights_vector> split_weights(VW::workspace* vw) {
  auto& weights = vw->weights.dense_weights;
  auto iter = weights.begin();

  auto end = weights.end();

  // ftrl_weights_vector weights_vector;
  separate_weights_vector sl_weights_vector;
  separate_weights_vector multi_weights_vector;
  std::vector<separate_weights_vector> result;

  while (iter < end) {
    bool non_zero = false;
    for (int i=0; i < 6; i++) {
      if (*iter[i] != 0.f) {
        non_zero = true;
      }
    }
    if (non_zero) {
      if ((iter.index_without_stride() & (2 - 1)) == 0) {
        // first model
        sl_weights_vector.emplace_back(iter.index_without_stride()>>1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      } else {
        // second model
        multi_weights_vector.emplace_back(iter.index_without_stride()>>1, *iter[0], *iter[1], *iter[2], *iter[3], *iter[4], *iter[5]);
      }
    }

    ++iter;
  }

  // print_separate_weights(sl_weights_vector);

  // print_separate_weights(multi_weights_vector);

  result.emplace_back(sl_weights_vector);
  result.emplace_back(multi_weights_vector);
  // std::sort(weights_vector.begin(), weights_vector.end());
  return result;
}

ftrl_weights_vector train_multiline_igl(example_vector examples) {
  auto* vw = VW::initialize(
    "--cb_explore_adf --coin --experimental_igl -q UA --noconstant" // --epsilon 0.2
  );

  for (auto& igl_multi_ex_str : examples) {
    VW::multi_ex igl_vw_example;
    for (const std::string& ex : igl_multi_ex_str) {
      igl_vw_example.push_back(VW::read_example(*vw, ex));
    }
    vw->learn(igl_vw_example);
    vw->finish_example(igl_vw_example);
  }

  return get_weights(vw);
}

ftrl_weights_vector train_sl_igl(example_vector sl_examples) {
  auto* vw = VW::initialize(
    "--link=logistic --loss_function=logistic --coin --noconstant --cubic UAF"
    // "--link=logistic --loss_function=logistic --coin --noconstant"
  );

  for (auto& sl_ex_str : sl_examples) {
    for (auto& ex_str : sl_ex_str) {
      VW::example* ex = VW::read_example(*vw, ex_str);
      vw->learn(*ex);
      vw->finish_example(*ex);
    }
  }

  return get_weights(vw);
}

ftrl_weights_vector train_dsjson_igl(VW::workspace* vw, std::vector<std::string> json_examples) {
  for (auto& json_text : json_examples) {
    auto examples = vwtest::parse_dsjson(*vw, json_text);

    vw->learn(examples);
    vw->finish_example(examples);
  }

  return get_weights(vw);
}

TEST(igl_test, igl_model_weights_are_equal_to_train_models_separately) {
  // two vw instance
  auto* sl_vw = VW::initialize(
    "--link=logistic --loss_function=logistic --coin --noconstant --readable_model psi.readable --cubic cav"  //--cubic cav
  );
  auto* multi_vw = VW::initialize("--cb_explore_adf --coin --noconstant --dsjson --readable_model pol.readable -q ca"); // -q ca

  // igl instance
  auto* igl_vw = VW::initialize("--cb_explore_adf --coin --experimental_igl --noconstant --dsjson --readable_model igl.readable -b 19 -q ca"); // -q ca

  // train separately
  for (int i = 0; i < ex_num; i++) {
    auto sl_examples = sl_vector[i];
    for (auto& ex_str : sl_examples) {
      VW::example* ex = VW::read_example(*sl_vw, ex_str);
      sl_vw->learn(*ex);
      sl_vw->finish_example(*ex);
    }

    auto multi_ex = multi_vector[i];
    auto examples = vwtest::parse_dsjson(*multi_vw, multi_ex);
    multi_vw->learn(examples);
    multi_vw->finish_example(examples);
  }

  // train IGL
  for (int i = 0; i < ex_num; i++) {
    auto json_text = igl_dsjson_vector[i];
    auto examples = vwtest::parse_dsjson(*igl_vw, json_text);

    igl_vw->learn(examples);
    igl_vw->finish_example(examples);
  }

  // separate weights
  separate_weights_vector sl_weights = get_separate_weights(sl_vw);
  separate_weights_vector multi_weights = get_separate_weights(multi_vw);

  // split IGL weights
  std::vector<separate_weights_vector> igl_weights = split_weights(igl_vw);

  VW::finish(*sl_vw);
  VW::finish(*multi_vw);
  VW::finish(*igl_vw);

  std::vector<size_t> sl_hash = get_hash(sl_weights);
  std::vector<size_t> multi_hash = get_hash(multi_weights);
  std::vector<size_t> igl_sl_hash = get_hash(igl_weights[0]);
  std::vector<size_t> igl_multi_hash = get_hash(igl_weights[1]);

  EXPECT_EQ(sl_hash, igl_sl_hash);
  EXPECT_EQ(multi_hash, igl_multi_hash);

  EXPECT_GT(sl_weights.size(), 0);
  EXPECT_EQ(sl_weights, igl_weights[0]);

  EXPECT_GT(multi_weights.size(), 0);
  EXPECT_EQ(multi_weights, igl_weights[1]);
}