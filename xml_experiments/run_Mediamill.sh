#!/usr/bin/env bash

DATASET_NAME="Mediamill"
FILES_PREFIX="mediamill"
K=101
PARAMS="-l 3.0 --power_t 0.3 --kary_tree 8 --passes 12 -b 30"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
