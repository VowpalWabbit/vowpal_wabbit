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

  std::vector<unsigned char> data;
  data.push_back('\11');

  buffer.append(&data.at(0), data.size());

  BOOST_CHECK_EQUAL(buffer.buffer().at(0), '\11');
}

BOOST_AUTO_TEST_CASE(multiple_outputs_to_data_buffer) {
  data_buffer buffer;

  std::vector<unsigned char> data;
  data.push_back('\11');
  data.push_back('\12');

  buffer.append(&data.at(0), data.size());

  BOOST_CHECK_EQUAL(buffer.buffer().at(0), '\11');
  BOOST_CHECK_EQUAL(buffer.buffer().at(1), '\12');
}

BOOST_AUTO_TEST_CASE(empty_data_buffer_reset) {
  data_buffer buffer;

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


