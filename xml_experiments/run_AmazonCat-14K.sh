#!/usr/bin/env bash

DATASET_NAME="AmazonCat-14K"
FILES_PREFIX="amazonCat-14K"
K=14588
PARAMS="-l 0.01 --power_t 0.4 --kary_tree 16 --passes 6 -b 33"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
