#!/bin/bash

# A test for model consistency cross all nodes on cluster
TEST_NAME="same_model_test"

TRAIN_SET_0=train-sets/${TEST_NAME}.0.dat
TRAIN_SET_1=train-sets/${TEST_NAME}.1.dat
MODEL_0=models/${TEST_NAME}.0.txt
MODEL_1=models/${TEST_NAME}.1.txt

# train on cluster mode
../cluster/spanning_tree

../vowpalwabbit/vw -d ${TRAIN_SET_0} --readable_model ${MODEL_0} \
--span_server localhost --total 2 --node 0 --unique_id 2333 \
-q ab --passes 1 --holdout_off &

../vowpalwabbit/vw -d ${TRAIN_SET_1} --readable_model ${MODEL_1} \
--span_server localhost --total 2 --node 1 --unique_id 2333 \
-q ab --passes 1 --holdout_off

killall spanning_tree

# compare output model
DIFF=$(diff ${MODEL_0} ${MODEL_1})
if [ -z "$DIFF" ]; then
    echo "$TEST_NAME: OK"
    exit 0
else
    echo "$TEST_NAME: FAILED: $MODEL_0 and $MODEL_1 are not same"
    exit 1
fi
