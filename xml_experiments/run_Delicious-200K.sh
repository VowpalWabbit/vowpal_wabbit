#!/usr/bin/env bash

DATASET_NAME="Delicious-200K"
FILES_PREFIX="deliciousLarge"
K=205443
PARAMS="-l 0.000002 --power_t 0.2 --kary_tree 2 --passes 12 -b 34"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
