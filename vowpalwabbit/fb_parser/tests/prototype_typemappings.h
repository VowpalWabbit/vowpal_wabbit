#include "prototype_example_root.h"
#include "vw/fb_parser/generated/example_generated.h"

#pragma once

namespace vwtest
{
template <typename T>
struct fb_type
{
};

template <>
struct fb_type<prototype_namespace_t>
{
  using type = VW::parsers::flatbuffer::Namespace;
};

template <>
struct fb_type<prototype_example_t>
{
  using type = VW::parsers::flatbuffer::Example;

  constexpr static fb::ExampleType root_type = fb::ExampleType::ExampleType_Example;
};

template <>
struct fb_type<prototype_multiexample_t>
{
  using type = VW::parsers::flatbuffer::MultiExample;

  constexpr static fb::ExampleType root_type = fb::ExampleType::ExampleType_MultiExample;
};

template <>
struct fb_type<prototype_example_collection_t>
{
  using type = VW::parsers::flatbuffer::ExampleCollection;

  constexpr static fb::ExampleType root_type = fb::ExampleType::ExampleType_ExampleCollection;
};

using union_t = void;

template <>
struct fb_type<prototype_label_t>
{
  using type = union_t;
};
}  // namespace vwtest