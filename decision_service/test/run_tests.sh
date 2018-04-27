#!/bin/bash

set -x

./ds_event_test
./ds_async_batcher_test
./ds_concurrent_queue_test
./ds_eventhub_test
