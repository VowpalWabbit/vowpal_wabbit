#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "model_mgmt/data_callback_fn.h"

using namespace reinforcement_learning;
using namespace model_management;
using namespace std;

char const * const err_msg = "This is an error message";

void data_handler(const model_data& md, int* user_context) {
  BOOST_ASSERT(md.data() != nullptr);
  BOOST_ASSERT(md.data_sz() != 0);
  BOOST_ASSERT(md.refresh_count() > 0);
  BOOST_CHECK_EQUAL(*static_cast<int*>(user_context), -1);
}

BOOST_AUTO_TEST_CASE(data_callback) {
  auto i = -1;
  data_callback_fn fn(data_handler, &i);
  const auto str = "model data";
  
  model_data md;
  md.alloc(strlen(str)+1);
  md.increment_refresh_count();
  memcpy(md.data(), str, strlen(str)+1);
  
  fn.report_data(md, nullptr);
  md.free();
  BOOST_ASSERT(i == -1);
}

BOOST_AUTO_TEST_CASE(null_data_callback) {
  auto i = -1;
  using int_cb = data_callback_fn::data_fn_t<int>;
  data_callback_fn fn((int_cb) nullptr, &i);
  const auto str = "model data";

  model_data md;
  md.alloc(strlen(str)+1);
  md.increment_refresh_count();
  memcpy(md.data(), str, strlen(str)+1);

  fn.report_data(md, nullptr); // should not crash
  md.free();
  BOOST_ASSERT(i == -1);
}

void ex_data_handler(const model_data& md, int* user_context) {
  throw 5;
}

BOOST_AUTO_TEST_CASE(exception_in_data_callback) {
  auto i = -2;
  data_callback_fn fn(ex_data_handler, &i);
  const auto str = "model data";

  model_data md;
  md.alloc(strlen(str)+1);
  md.increment_refresh_count();
  memcpy(md.data(), str, strlen(str)+1);

  fn.report_data(md, nullptr); // should not crash
  md.free();
  BOOST_ASSERT(i == -2);
}

