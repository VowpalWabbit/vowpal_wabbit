#!/usr/bin/env bash

DATASET_NAME="AmazonCat-13K"
FILES_PREFIX="amazonCat"
K=13330
PARAMS="-l 0.003 --power_t 0.3 --kary_tree 16 --passes 6 -b 33"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
