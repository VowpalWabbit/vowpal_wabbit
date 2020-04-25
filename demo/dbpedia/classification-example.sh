#!/usr/bin/env bash
#
# Adapted from fastText/classification-example.sh

myshuf() {
  perl -MList::Util=shuffle -e 'print shuffle(<>);' "$@";
}

normalize_text() {
  tr '[:upper:]' '[:lower:]' | sed -e 's/^\([0-9][0-9]*\),/\1 | /g' | \
    sed -e "s/'/ ' /g" -e 's/"//g' -e 's/\./ \. /g' -e 's/<br \/>/ /g' \
        -e 's/,/ , /g' -e 's/(/ ( /g' -e 's/)/ ) /g' -e 's/\!/ \! /g' \
        -e 's/\?/ \? /g' -e 's/\;/ /g' -e 's/\:/ /g' | tr -s " " | myshuf
}

RESULTDIR=result
DATADIR=data

mkdir -p "${RESULTDIR}"
mkdir -p "${DATADIR}"

if [ ! -f "${DATADIR}/dbpedia.train" ]
then
  wget -c "https://github.com/le-scientifique/torchDatasets/raw/master/dbpedia_csv.tar.gz" -O "${DATADIR}/dbpedia_csv.tar.gz"
  tar -xzvf "${DATADIR}/dbpedia_csv.tar.gz" -C "${DATADIR}"
  cat "${DATADIR}/dbpedia_csv/train.csv" | normalize_text > "${DATADIR}/dbpedia.train"
  cat "${DATADIR}/dbpedia_csv/test.csv" | normalize_text > "${DATADIR}/dbpedia.test"
fi

VW_EXEC=../../vowpalwabbit/vw
NUM_CLASSES=`cat ${DATADIR}/dbpedia_csv/classes.txt | wc -l`

${VW_EXEC} -d "${DATADIR}/dbpedia.train" \
  --cache_file "${RESULTDIR}/dbpedia.cache" -f "${RESULTDIR}/dbpedia.bin" \
  --oaa ${NUM_CLASSES} --passes 5 --ngram 2 --skips 2 \
  --loss_function hinge --bit_precision 25 --l2 1e-5 \
  -k --threads

${VW_EXEC} -t -d "${DATADIR}/dbpedia.test" \
  -i "${RESULTDIR}/dbpedia.bin"

${VW_EXEC} -t -d "${DATADIR}/dbpedia.test" \
  -i "${RESULTDIR}/dbpedia.bin" \
  -p "${RESULTDIR}/dbpedia.test.predict" \
  --quiet
