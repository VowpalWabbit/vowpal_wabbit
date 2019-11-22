#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <vector>
#include "conditional_contextual_bandit.h"
#include "parser.h"

void parse_label(label_parser& lp, parser* p, std::string label, CCB::label& l)
{
  tokenize(' ', { const_cast<char*>(label.c_str()), const_cast<char*>(label.c_str()) + strlen(label.c_str()) }, p->words);
  lp.default_label(&l);
  lp.parse_label(p, nullptr, &l, p->words);
}

BOOST_AUTO_TEST_CASE(ccb_parse_label)
{
  auto lp = CCB::ccb_label_parser;
  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();

  {
    CCB::label label;
    // Memset is used because in practice these labels are always Calloced.
    // This is dangerous though and we should REALLY let this object have a constructor.
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb shared", label);
    BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
    BOOST_CHECK(label.outcome == nullptr);
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::shared);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb action", label);
    BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
    BOOST_CHECK(label.outcome == nullptr);
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::action);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb slot", label);
    BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 0);
    BOOST_CHECK(label.outcome == nullptr);
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::slot);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb slot 1,3,4", label);
    BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 3);
    BOOST_CHECK_EQUAL(label.explicit_included_actions[0], 1);
    BOOST_CHECK_EQUAL(label.explicit_included_actions[1], 3);
    BOOST_CHECK_EQUAL(label.explicit_included_actions[2], 4);
    BOOST_CHECK(label.outcome == nullptr);
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::slot);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb slot 1:1.0:0.5 3", label);
    BOOST_CHECK_EQUAL(label.explicit_included_actions.size(), 1);
    BOOST_CHECK_EQUAL(label.explicit_included_actions[0], 3);
    BOOST_CHECK_CLOSE(label.outcome->cost, 1.0f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label.outcome->probabilities.size(), 1);
    BOOST_CHECK_EQUAL(label.outcome->probabilities[0].action, 1);
    BOOST_CHECK_CLOSE(label.outcome->probabilities[0].score, .5f, FLOAT_TOL);
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::slot);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", label);
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
    BOOST_CHECK_EQUAL(label.type, CCB::example_type::slot);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "shared", label), VW::vw_exception);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "other shared", label), VW::vw_exception);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "other", label), VW::vw_exception);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "ccb unknown", label), VW::vw_exception);
    lp.delete_label(&label);
  }
  {
    CCB::label label;
    memset(&label, 0, sizeof(label));
    BOOST_REQUIRE_THROW(parse_label(lp, &p, "ccb slot 1:1.0:0.5,4:0.7", label), VW::vw_exception);
    lp.delete_label(&label);
  }
  p.words.delete_v();
  p.parse_name.delete_v();
}

BOOST_AUTO_TEST_CASE(ccb_cache_label)
{
  io_buf io;
  //io.init();      TODO: figure out and fix leak caused by double init()

  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();

  auto lp = CCB::ccb_label_parser;
  CCB::label label;
  memset(&label, 0, sizeof(label));
  parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", label);
  lp.cache_label(&label, io);
  io.space.end() = io.head;
  io.head = io.space.begin();

  CCB::label uncached_label;
  memset(&uncached_label, 0, sizeof(uncached_label));
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
  BOOST_CHECK_EQUAL(uncached_label.type, CCB::example_type::slot);
  lp.delete_label(&label);
  lp.delete_label(&uncached_label);
  p.words.delete_v();
  p.parse_name.delete_v();
}

BOOST_AUTO_TEST_CASE(ccb_copy_label)
{
  parser p{8 /*ring_size*/, false /*strict parse*/};
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();
  auto lp = CCB::ccb_label_parser;
  
  CCB::label label;
  memset(&label, 0, sizeof(label));
  parse_label(lp, &p, "ccb slot 1:-2.0:0.5,2:0.25,3:0.25 3,4", label);

  CCB::label copied_to;
  memset(&copied_to, 0, sizeof(copied_to));
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
  BOOST_CHECK_EQUAL(copied_to.type, CCB::example_type::slot);
  lp.delete_label(&label);
  lp.delete_label(&copied_to);
  p.words.delete_v();
  p.parse_name.delete_v();
}
