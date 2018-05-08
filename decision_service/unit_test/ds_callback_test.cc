#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "error_callback_fn.h"
#include "ds_api_status.h"

using namespace decision_service;
using namespace std;

int err = 10;
string err_msg = "This is an error message";

void error_handler(const api_status& s, void* user_context)
{
  BOOST_ASSERT(s.get_error_code() == err);
  BOOST_ASSERT(s.get_error_msg() == err_msg);
  *static_cast<int*>(user_context) = -1;
}

BOOST_AUTO_TEST_CASE(callback)
{
  auto i = 0;
  error_callback_fn fn(error_handler, &i);
  api_status s;
  s.set_error_code(err);
  s.set_error_msg(err_msg);
  fn.report_error(s);
  BOOST_ASSERT(i == -1);
}

BOOST_AUTO_TEST_CASE(null_callback)
{
  auto i = 0;
  error_callback_fn fn(nullptr, &i);
  api_status s;
  s.set_error_code(err);
  s.set_error_msg(err_msg);
  fn.report_error(s);  // should not crash
  BOOST_ASSERT(i == 0);
}

void ex_error_handler(const api_status& s, void* user_context)
{
  BOOST_ASSERT(s.get_error_code() == err);
  BOOST_ASSERT(s.get_error_msg() == err_msg);
  *static_cast<int*>(user_context) = -2;
  throw 5;
}

BOOST_AUTO_TEST_CASE(exception_in_callback)
{
  auto i = 0;
  error_callback_fn fn(ex_error_handler, &i);
  api_status s;
  s.set_error_code(err);
  s.set_error_msg(err_msg);
  fn.report_error(s);  // should not crash
  BOOST_ASSERT(i == -2);  
}

