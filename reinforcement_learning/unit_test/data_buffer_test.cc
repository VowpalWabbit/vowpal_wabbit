#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "utility/data_buffer.h"

using namespace reinforcement_learning;
using namespace utility;
using namespace std;

BOOST_AUTO_TEST_CASE(new_data_buffer_is_empty) {
  data_buffer buffer;

  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(single_output_to_data_buffer) {
  data_buffer buffer;

  buffer << "test";

  BOOST_CHECK_EQUAL(buffer.str(), "test");
}

BOOST_AUTO_TEST_CASE(multiple_outputs_to_data_buffer) {
  data_buffer buffer;

  const string value_string = "test";
  const size_t value_size_t = 2;

  buffer << value_string << value_size_t << value_string.c_str();

  BOOST_CHECK_EQUAL(buffer.str(), "test2test");
}

BOOST_AUTO_TEST_CASE(remove_last_from_nonempty_data_buffer) {
    data_buffer buffer;
    
    buffer << "test1";
    buffer.remove_last();

    BOOST_CHECK_EQUAL(buffer.str(), "test");
}

BOOST_AUTO_TEST_CASE(empty_data_buffer_reset) {
  data_buffer buffer;

  buffer << "test";

  buffer.reset();

  BOOST_CHECK_EQUAL(buffer.size(), 0);
}

BOOST_AUTO_TEST_CASE(nonempty_data_buffer_reset) {
  data_buffer buffer;

  unsigned char data = '\11';
  buffer.append(&data, 1);
  buffer.reset();

  BOOST_CHECK_EQUAL(buffer.size(), 0);
}
