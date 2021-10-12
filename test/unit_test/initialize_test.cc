// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>

#include "vw.h"

// An API break was introduced in PR #2418 where passing more than 1 positional parameters was an error. This broke the API contract outside of a scheduled deprecation.
BOOST_AUTO_TEST_CASE(multiple_positional_parameters_can_be_passed_even_though_only_one_is_read_test) {
  auto vw = VW::initialize("positional1 positional2 --quiet", nullptr, false, nullptr, nullptr);
  // Ideally verify that a warning is produced.
  VW::finish(*vw);
}
