#!/usr/bin/env bash

DATASET_NAME="WikiLSHTC-325K"
FILES_PREFIX="wikiLSHTC"
K=325056
PARAMS="-l 0.4 --power_t 0.3 --kary_tree 128 --passes 20 -b 35"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
