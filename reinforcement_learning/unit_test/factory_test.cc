#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "object_factory.h"
#include "err_constants.h"
#include "constants.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(factory_tempate_usage) {
  struct an_interface { virtual void do_something() = 0; virtual ~an_interface() {} };
  struct impl_A : public an_interface { void do_something() override {} };
  struct impl_B : public an_interface { explicit impl_B(int b) {} void do_something() override {} };

  auto b = 5;  // arbitrary variable to illustrate a point
  u::object_factory<an_interface, const u::config_collection&> factory;

  auto create_A_fn = [](an_interface** pret, const u::config_collection&, r::api_status*) -> int {
    *pret = new impl_A();
    return r::error_code::success;
  };

  auto create_B_fn = [b](an_interface** pret, const u::config_collection&, r::api_status*) -> int {
    *pret = new impl_B(b);
    return r::error_code::success;
  };

  factory.register_type("A", create_A_fn);
  factory.register_type("B", create_B_fn);

  u::config_collection cc;
  cc.set(r::name::MODEL_SRC, r::value::AZURE_STORAGE_BLOB);

  an_interface* p_impl;
  auto scode = factory.create(&p_impl, std::string("A"), cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  p_impl->do_something();
  delete p_impl;
}
