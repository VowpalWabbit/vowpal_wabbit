#!/usr/bin/env bash

DATASET_NAME="Amazon-670K"
FILES_PREFIX="amazon"
K=159
PARAMS="-l 0.0004 --power_t 0.2 --kary_tree 64 --passes 12 -b 33"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
