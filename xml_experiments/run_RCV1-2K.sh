#!/usr/bin/env bash

DATASET_NAME="RCV1-2K"
FILES_PREFIX="rcv1x"
K=2456
PARAMS="-l 4.0 --power_t 0.3 --kary_tree 24 --passes 12 -b 30"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
