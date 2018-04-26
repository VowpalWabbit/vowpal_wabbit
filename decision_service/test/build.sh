#!/bin/bash

set -x

# compile manually boost unit tests
g++ -std=c++11 -o ds_event_test ds_event_test.cc ../src/ds_event.cc -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
g++ -std=c++11 -o ds_concurrent_queue_test ds_concurrent_queue_test.cc -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
g++ -std=c++11 -o ds_async_batch_test ds_async_batch_test.cc -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK