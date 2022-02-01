
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "simulator.h"
#include "reductions_fwd.h"
#include "ae.h"
#include "metric_sink.h"

#include <functional>
#include <map>
#include <utility>

using simulator::callback_map;
using simulator::cb_sim;
using namespace VW::ae;

BOOST_AUTO_TEST_CASE(ae_test)
{
  // we initialize the reduction pointing to position 0 as champ, that config is hard-coded to empty
  auto ctr =
      simulator::_test_helper("--agedexp --model_count 3 --cb_explore_adf --quiet --epsilon 0.2 --random_seed 5");
  float with_save = ctr.back();
}

BOOST_AUTO_TEST_CASE(ae_save_load)
{
  callback_map empty_hooks;
  auto ctr = simulator::_test_helper_hook("--agedexp --model_count 5 --cb_explore_adf --quiet", empty_hooks);
  float without_save = ctr.back();
  BOOST_CHECK_GT(without_save, 0.3f);

  ctr = simulator::_test_helper_save_load("--agedexp --model_count 5 --cb_explore_adf --quiet");

  float with_save = ctr.back();
  BOOST_CHECK_GT(with_save, 0.3f);

  BOOST_CHECK_CLOSE(without_save, with_save, FLOAT_TOL);
}
