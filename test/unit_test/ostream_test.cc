#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "error_reporting.h"
#include "memory.h"

BOOST_AUTO_TEST_CASE(test_custom_ostream)
{
  auto output_func = [](void* context, const std::string& input) {
    BOOST_CHECK(context == nullptr);
    BOOST_CHECK_EQUAL(input, "This is the test input, 123\n");
  };

  owning_ostream stream{VW::make_unique<custom_output_stream_buf>(nullptr, output_func)};

  stream << "This is the test input, " << 123 << std ::endl;
}
