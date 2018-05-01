#!/bin/bash

set -x

# compile manually boost tests
g++ -std=c++11 -o ds_event_test ds_event_test.cc ../src/ds_event.cc -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
g++ -std=c++11 -o ds_concurrent_queue_test ds_concurrent_queue_test.cc -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
g++ -std=c++11 -o ds_async_batcher_test ds_async_batcher_test.cc -pthread -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
g++ -std=c++11 -o ds_eventhub_client_test ds_eventhub_client_test.cc ../src/ds_eventhub_client.cc http_server/http_server.cc http_server/stdafx.cc -lboost_system -lcrypto -lssl -lcpprest -lboost_unit_test_framework -DBOOST_TEST_DYN_LINK
