// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// forward declarations
class io_buf;
class parameters;
class dense_parameters;
class features;
class shared_data;
class parser;

namespace VW
{
template <typename T, typename Enable = void>
class v_array;

template <class T>
class v_array<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>;

class loss_function;
class named_labels;
class reduction_features;
class example;
class kskip_ngram_transformer;
class label_parser;
class polylabel;
class rand_state;
class setup_base_i;
class workspace;

using multi_ex = std::vector<example*>;
using namespace_index = unsigned char;

class action_score;
using action_scores = VW::v_array<action_score>;

namespace LEARNER
{
template <class T, class E>
class learner;
using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;
}  // namespace LEARNER

namespace config
{
class options_i;
}  // namespace config

namespace io
{
class logger;
class reader;
class writer;
}  // namespace io

namespace details
{
class cache_temp_buffer;
}

}  // namespace VW

using extent_term = std::pair<VW::namespace_index, uint64_t>;
