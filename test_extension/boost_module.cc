#include <boost/python.hpp>

const char* greet() {
    return "Hello from Boost.Python extension!";
}

BOOST_PYTHON_MODULE(boost_module) {
    using namespace boost::python;
    def("greet", greet);
}
