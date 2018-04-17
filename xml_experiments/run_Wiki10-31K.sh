#!/usr/bin/env bash

DATASET_NAME="Wiki10-31K"
FILES_PREFIX="wiki10"
K=30938
PARAMS="-l 0.0001 --power_t 0.4 --kary_tree 12 --passes 12 -b 33"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
