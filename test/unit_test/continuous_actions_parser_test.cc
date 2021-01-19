// #ifndef STATIC_LINK_VW
// #  define BOOST_TEST_DYN_LINK
// #endif

// #include <boost/test/unit_test.hpp>
// #include <boost/test/test_tools.hpp>

// #include "test_common.h"

// #include <vector>
// #include "cb_continuous_label.h"
// #include "parser.h"
// #include <memory>

// void parse_label(label_parser& lp, parser* p, VW::string_view label, VW::cb_continuous::continuous_label& l,
//     reduction_features& red_fts)
// {
//   tokenize(' ', label, p->words);
//   lp.default_label(&l);
//   lp.parse_label(p, nullptr, &l, p->words, red_fts);
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_label)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "ca 185.121:0.657567:6.20426e-05", *label, red_features);
//     BOOST_CHECK_CLOSE(label->costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].cost, 0.657567, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].action, 185.121, FLOAT_TOL);

//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_and_pdf)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;

//     parse_label(lp, &p, "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *label,
//         red_features);
//     // check label
//     BOOST_CHECK_CLOSE(label->costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].cost, 0.657567, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].action, 185.121, FLOAT_TOL);

//     // check pdf
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_only_pdf_no_label)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "ca pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05", *label, red_features);
//     BOOST_CHECK_EQUAL(label->costs.size(), 0);

//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_malformed_pdf)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;

//     parse_label(lp, &p, "ca pdf 185:8109.67 8109.67:23959:6.20426e-05", *label, red_features);

//     // check pdf
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 0);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_and_chosen_action)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "ca 185.121:0.657567:6.20426e-05 chosen_action 8110.121", *label, red_features);

//     // check label
//     BOOST_CHECK_CLOSE(label->costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].cost, 0.657567, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].action, 185.121, FLOAT_TOL);

//     // check chosen action
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
//     BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_chosen_action_only_no_label)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "ca chosen_action 8110.121", *label, red_features);

//     BOOST_CHECK_EQUAL(label->costs.size(), 0);
//     // check chosen action
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
//     BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_label_pdf_and_chosen_action)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p,
//         "ca 185.121:0.657567:6.20426e-05 pdf 185:8109.67:2.10314e-06 8109.67:23959:6.20426e-05 chosen_action 8110.121",
//         *label, red_features);

//     // check label
//     BOOST_CHECK_CLOSE(label->costs[0].pdf_value, 6.20426e-05, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].cost, 0.657567, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(label->costs[0].action, 185.121, FLOAT_TOL);

//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();

//     // check chosen action
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), true);
//     BOOST_CHECK_CLOSE(cats_reduction_features.chosen_action, 8110.121, FLOAT_TOL);

//     // check pdf
//     BOOST_CHECK_EQUAL(cats_reduction_features.pdf.size(), 2);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].left, 185., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].right, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[0].pdf_value, 2.10314e-06, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].left, 8109.67, FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].right, 23959., FLOAT_TOL);
//     BOOST_CHECK_CLOSE(cats_reduction_features.pdf[1].pdf_value, 6.20426e-05, FLOAT_TOL);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_no_label)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "", *label, red_features);

//     BOOST_CHECK_EQUAL(label->costs.size(), 0);
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continuous_actions_parse_no_label_w_prefix)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     parse_label(lp, &p, "ca", *label, red_features);

//     BOOST_CHECK_EQUAL(label->costs.size(), 0);
//     const auto& cats_reduction_features = red_features.template get<VW::continuous_actions::reduction_features>();
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_pdf_set(), false);
//     BOOST_CHECK_EQUAL(cats_reduction_features.is_chosen_action_set(), false);

//     lp.delete_label(label.get());
//   }
// }

// BOOST_AUTO_TEST_CASE(continus_actions_check_label_for_prefix)
// {
//   auto lp = VW::cb_continuous::the_label_parser;
//   parser p{8 /*ring_size*/, false /*strict parse*/};

//   {
//     auto label = scoped_calloc_or_throw<VW::cb_continuous::continuous_label>();
//     reduction_features red_features;
//     BOOST_REQUIRE_THROW(parse_label(lp, &p, "185.121:0.657567:6.20426e-05", *label, red_features), VW::vw_exception);
//     lp.delete_label(label.get());
//   }
// }