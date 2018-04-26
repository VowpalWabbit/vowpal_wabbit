#include "../src/ds_async_batch.h"

#define BOOST_TEST_MODULE ds_async_batch_test
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>
#include <iostream>

using namespace decision_service;

class sender {
    public:
        std::vector<std::string> items;
        void send(const std::string& item) {
            items.push_back(item);
        };
};

BOOST_AUTO_TEST_CASE(flush_after_deletion)
{
    sender s;
	async_batch<sender>* batcher = new async_batch<sender>(s);

    batcher->append("foo");
    batcher->append("bar");

    delete batcher;//delete triggers a last flush

    std::string expected = "foo\nbar";

    BOOST_CHECK_EQUAL(s.items.size(), 1);
    BOOST_CHECK_EQUAL(s.items.front().c_str(), expected);
}

BOOST_AUTO_TEST_CASE(flush_timeout)
{
    sender s;
	async_batch<sender>* batcher = new async_batch<sender>(s);

    batcher->append("foo");
    batcher->append("bar");

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    
    std::string expected = "foo\nbar";

    BOOST_CHECK_EQUAL(s.items.size(), 1);
    BOOST_CHECK_EQUAL(s.items.front().c_str(), expected);

    delete batcher;//delete triggers a last flush
    BOOST_CHECK_EQUAL(s.items.size(), 1);
}
