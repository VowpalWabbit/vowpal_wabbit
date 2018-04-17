#!/usr/bin/env bash

DATASET_NAME="Bibtex"
FILES_PREFIX="bibtex"
K=159
PARAMS="-l 0.2 --power_t 0.4 --kary_tree 32 --passes 20 -b 30"

bash run_xml.sh $DATASET_NAME $FILES_PREFIX $K "$PARAMS"
