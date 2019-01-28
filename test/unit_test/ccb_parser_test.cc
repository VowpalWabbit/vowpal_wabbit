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

BOOST_AUTO_TEST_CASE(parse_ccb_sample)
{
  parser p;
  p.words = v_init<substring>();
  p.parse_name = v_init<substring>();

  auto label = parse_label(&p, "shared");
  BOOST_CHECK_EQUAL(label.excluded_actions.size(), 0);
  BOOST_CHECK_EQUAL(label.outcomes.size(), 0);
  BOOST_CHECK_EQUAL(label.type, CCB::example_type::shared);
}
