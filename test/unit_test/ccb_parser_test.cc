#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "conditional_contextual_bandit.h"
#include "parser.h"

CCB::label parse_label(parser* p, std::string label)
{
  auto lp = CCB::ccb_label_parser;
  tokenize(' ', { const_cast<char*>(label.c_str()), const_cast<char*>(label.c_str()) + strlen(label.c_str()) }, p->words);
  CCB::label l;
  lp.default_label(&l);
  lp.parse_label(p, nullptr, &l, p->words);

  return l;
}

BOOST_AUTO_TEST_CASE(ccb_parse_label)
{
  parser p{8};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();

  auto label = parse_label(&p, "ccb shared");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
  BOOST_CHECK(label.outcome == nullptr);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::shared);

  label = parse_label(&p, "ccb action");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
  BOOST_CHECK(label.outcome == nullptr);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::action);

  label = parse_label(&p, "ccb decision");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
  BOOST_CHECK(label.outcome == nullptr);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::decision);

  label = parse_label(&p, "ccb decision 1,3,4");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 3);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[0], 1);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[1], 3);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[2], 4);
  BOOST_CHECK(label.outcome == nullptr);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::decision);

  label = parse_label(&p, "ccb decision 1:1.0:0.5 3");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 1);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[0], 3);
  BOOST_CHECK_CLOSE(label.outcome->cost, 1.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.outcome->probabilities.size(), 1);
  BOOST_CHECK_EQUAL(label.outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(label.outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::decision);

  label = parse_label(&p, "ccb decision 1:-2.0:0.5,2:0.25,3:0.25 3,4");
  BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 2);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[0], 3);
  BOOST_CHECK_EQUAL(label.explicit_included_actions[1], 4);
  BOOST_CHECK_CLOSE(label.outcome->cost, -2.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.outcome->probabilities.size(), 3);
  BOOST_CHECK_EQUAL(label.outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(label.outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.outcome->probabilities[1].action, 2);
  BOOST_CHECK_CLOSE(label.outcome->probabilities[1].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.outcome->probabilities[2].action, 3);
  BOOST_CHECK_CLOSE(label.outcome->probabilities[2].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::decision);

  BOOST_REQUIRE_THROW(parse_label(&p, "shared"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_label(&p, "other shared"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_label(&p, "other"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_label(&p, "ccb unknown"), VW::vw_exception);
  BOOST_REQUIRE_THROW(parse_label(&p, "ccb decision 1:1.0:0.5,4:0.7"), VW::vw_exception);
}

BOOST_AUTO_TEST_CASE(ccb_cache_label)
{
  io_buf io;
  io.init();
  io.space.resize(1000);
  io.space.end() = io.space.begin() + 1000;

  parser p{8};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();

  auto lp = CCB::ccb_label_parser;
  auto label = parse_label(&p, "ccb decision 1:-2.0:0.5,2:0.25,3:0.25 3,4");

  lp.cache_label(&label, io);
  io.head = io.space.begin();

  CCB::label uncached_label;
  lp.default_label(&uncached_label);
  lp.read_cached_label(nullptr, &uncached_label, io);

  BOOST_CHECK_EQUAL(uncached_label.explicit_included_actions.size(), 2);
  BOOST_CHECK_EQUAL(uncached_label.explicit_included_actions[0], 3);
  BOOST_CHECK_EQUAL(uncached_label.explicit_included_actions[1], 4);
  BOOST_CHECK_CLOSE(uncached_label.outcome->cost, -2.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label.outcome->probabilities.size(), 3);
  BOOST_CHECK_EQUAL(uncached_label.outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(uncached_label.outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label.outcome->probabilities[1].action, 2);
  BOOST_CHECK_CLOSE(uncached_label.outcome->probabilities[1].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label.outcome->probabilities[2].action, 3);
  BOOST_CHECK_CLOSE(uncached_label.outcome->probabilities[2].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(uncached_label.type, CCB::example_type::decision);
}

BOOST_AUTO_TEST_CASE(ccb_copy_label)
{
  parser p{8};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();
  auto lp = CCB::ccb_label_parser;

  auto label = parse_label(&p, "ccb decision 1:-2.0:0.5,2:0.25,3:0.25 3,4");

  CCB::label copied_to;
  lp.default_label(&copied_to);

  lp.copy_label(&copied_to, &label);

  BOOST_CHECK_EQUAL(copied_to.explicit_included_actions.size(), 2);
  BOOST_CHECK_EQUAL(copied_to.explicit_included_actions[0], 3);
  BOOST_CHECK_EQUAL(copied_to.explicit_included_actions[1], 4);
  BOOST_CHECK_CLOSE(copied_to.outcome->cost, -2.0f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to.outcome->probabilities.size(), 3);
  BOOST_CHECK_EQUAL(copied_to.outcome->probabilities[0].action, 1);
  BOOST_CHECK_CLOSE(copied_to.outcome->probabilities[0].score, .5f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to.outcome->probabilities[1].action, 2);
  BOOST_CHECK_CLOSE(copied_to.outcome->probabilities[1].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to.outcome->probabilities[2].action, 3);
  BOOST_CHECK_CLOSE(copied_to.outcome->probabilities[2].score, .25f, FLOAT_TOL);
  BOOST_CHECK_EQUAL(copied_to.type, CCB::example_type::decision);
}
