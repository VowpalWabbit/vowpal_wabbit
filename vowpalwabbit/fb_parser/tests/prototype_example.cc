// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.


#include "prototype_example.h"

namespace vwtest
{


  
  // namespace details
  // {
  //   using example_type_t = std::underlying_type<VW::parsers::flatbuffer::ExampleType>::type;
  //   constexpr example_type_t bad_example_type = static_cast<example_type_t>(VW::parsers::flatbuffer::ExampleType_MAX) + 1;

  //   template <typename T>
  //   struct example_root_type : public std::integral_constant<example_type_t, bad_example_type>
  //   {
  //   };

  //   template <>
  //   struct example_root_type<prototype_example> 
  //     : public std::integral_constant<example_type_t, VW::parsers::flatbuffer::ExampleType_Example>
  //   {
  //   };

  //   template <>
  //   struct example_root_type<prototype_multiexample> 
  //     : public std::integral_constant<example_type_t, VW::parsers::flatbuffer::ExampleType_MultiExample>
  //   {
  //   };

  //   template <>
  //   struct example_root_type<prototype_example_collection> 
  //     : public std::integral_constant<example_type_t, VW::parsers::flatbuffer::ExampleType_ExampleCollection>
  //   {
  //   };
  // }


}