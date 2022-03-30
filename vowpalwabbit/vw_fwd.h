// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// forward declarations
class io_buf;
class parameters;
struct features;
struct random_state;
struct shared_data;
struct parser;

namespace VW
{
template <typename T, typename Enable = void>
struct v_array;

template <class T>
struct v_array<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type>;

struct label_parser;
struct example;
using multi_ex = std::vector<example*>;
using namespace_index = unsigned char;
struct workspace;
struct setup_base_i;
struct kskip_ngram_transformer;
struct rand_state;
class named_labels;
struct setup_base_i;
class loss_function;

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

enum class log_level;
using logger_output_func_t = void (*)(void*, VW::io::log_level, const std::string&);

}  // namespace io

namespace details
{
struct cache_temp_buffer;
}

#ifdef BUILD_EXTERNAL_PARSER
namespace external
{
class parser;
struct parser_options;
}  // namespace external
#endif

}  // namespace VW

using extent_term = std::pair<VW::namespace_index, uint64_t>;
