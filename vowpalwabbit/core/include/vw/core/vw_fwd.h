// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// forward declarations
struct io_buf;
struct parameters;
struct dense_parameters;
struct features;
struct shared_data;
struct parser;

namespace VW
{
template <typename T, typename Enable = void>
struct v_array;

template <class T>
struct v_array<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>;

struct loss_function;
struct named_labels;
struct reduction_features;
struct example;
struct kskip_ngram_transformer;
struct label_parser;
struct polylabel;
struct rand_state;
struct setup_base_i;
struct workspace;

using multi_ex = std::vector<example*>;
using namespace_index = unsigned char;

struct action_score;
using action_scores = VW::v_array<action_score>;

namespace LEARNER
{
template <class T, class E>
struct learner;
using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;
}  // namespace LEARNER

namespace config
{
struct options_i;
}  // namespace config

namespace io
{
struct logger;
struct reader;
struct writer;
}  // namespace io

namespace details
{
struct cache_temp_buffer;
}

}  // namespace VW

using extent_term = std::pair<VW::namespace_index, uint64_t>;
