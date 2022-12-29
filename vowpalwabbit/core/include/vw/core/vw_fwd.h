// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

// forward declarations
namespace VW
{
class parameters;
class dense_parameters;
class io_buf;
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
class metric_sink;
class shared_data;
class parser;
class features;

using namespace_index = unsigned char;

namespace LEARNER
{
template <class T, class E>
class learner;
using base_learner = learner<char, char>;
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

namespace parsers
{
namespace cache
{
namespace details
{
class cache_temp_buffer;
}

}  // namespace cache
}  // namespace parsers

}  // namespace VW
