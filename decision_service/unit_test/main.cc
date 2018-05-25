#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main
#include <boost/test/unit_test.hpp>

#include <openssl/ssl.h>

struct GlobalConfig {
  ~GlobalConfig()
  {
    // fix memory leak: https://rt.openssl.org/Ticket/Display.html?id=2561&user=guest&pass=guest
    SSL_COMP_free_compression_methods();
  }
};

BOOST_GLOBAL_FIXTURE(GlobalConfig);
