// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "metric_sink.h"
#include "simulator.h"
#include "reductions_fwd.h"
#include "constant.h"

#include <functional>
#include <map>

using simulator::callback_map;

namespace ut_helper
{
void assert_metric(const std::string& metric_name, const size_t value, const VW::metric_sink& metrics)
{
  auto it = std::find_if(metrics.int_metrics_list.begin(), metrics.int_metrics_list.end(),
      [&metric_name](const std::pair<std::string, size_t>& element) { return element.first == metric_name; });

  if (it == metrics.int_metrics_list.end()) { BOOST_FAIL("could not find metric. fatal."); }
  else
  {
    BOOST_CHECK_EQUAL(it->second, value);
  }
}
}  // namespace ut_helper

BOOST_AUTO_TEST_CASE(assert_0th_event)
{
  const auto metric_name = std::string("total_learn_calls");
  const size_t zero = 0;
  const size_t num_iterations = 10;
  callback_map test_hooks;

  // technically runs after the 0th example is learned
  test_hooks.emplace(zero, [&metric_name, &zero](vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    ut_helper::assert_metric(metric_name, zero, metrics);
    return true;
  });

  // test executes right after learn call of the 10th example
  test_hooks.emplace(num_iterations, [&metric_name, &num_iterations](vw& all, multi_ex&) {
    VW::metric_sink metrics;
    all.l->persist_metrics(metrics);

    ut_helper::assert_metric(metric_name, num_iterations, metrics);
    return true;
  });

  auto ctr = simulator::_test_helper_hook(
      "--extra_metrics ut_metrics.json --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5", test_hooks,
      num_iterations);

  BOOST_CHECK_GT(ctr.back(), 0.1f);
}
