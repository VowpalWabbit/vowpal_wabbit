#include <boost/python.hpp>
#include "vw/core/vw.h"

// Simple function that uses VW library
const char* test_vw() {
    // Create a minimal VW instance to ensure we're linking to vw_core
    auto vw = VW::initialize("--quiet", nullptr, false, nullptr, nullptr);
    VW::finish(*vw);
    return "VW test module works!";
}

BOOST_PYTHON_MODULE(vw_test_module) {
    using namespace boost::python;
    def("test_vw", test_vw);
}
