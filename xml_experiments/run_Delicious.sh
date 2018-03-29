#!/usr/bin/env bash

DATASET_NAME="Delicious"
FILES_PREFIX="delicious"
K=983
PARAMS="-l 1.0 --power_t 0.4 --kary_tree 12 --passes 12 -b 30"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
