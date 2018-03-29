#!/usr/bin/env bash

DATASET_NAME=$1
FILES_PREFIX=$2
K=$3
PARAMS=$4

SCRIPT_DIR=$( dirname "${BASH_SOURCE[0]}" )
BIN=${SCRIPT_DIR}/../vowpalwabbit/vw

if [ ! -e datasets4vw ]; then
    git clone https://github.com/mwydmuch/datasets4vw.git
fi

if [ ! -e datasets4vw/xml_repo/${FILES_PREFIX} ]; then
    bash datasets4vw/xml_repo/get_${DATASET_NAME}.sh
fi

if [ ! -e ${BIN} ]; then
    cd ${SCRIPT_DIR}/..
    make
    cd ${SCRIPT_DIR}
fi

TRAIN=${FILES_PREFIX}/${FILES_PREFIX}_train
TEST=${FILES_PREFIX}/${FILES_PREFIX}_test

if [ ! -e $TRAIN ]; then
    TRAIN=${FILES_PREFIX}/${FILES_PREFIX}_train0
    TEST=${FILES_PREFIX}/${FILES_PREFIX}_test0
fi

mkdir -p models
MODEL="models/${FILES_PREFIX}_$(echo $PARAMS | tr ' ' '_')"

time ${BIN} --plt $K $TRAIN -f $MODEL --sgd $PARAMS -c
time ${BIN} -t -i $MODEL $TEST --top_k 5
