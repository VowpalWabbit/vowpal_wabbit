#!/bin/bash
set -e
set -x

cd $1
cd test
export PATH=../build/vowpalwabbit/:$PATH && ./RunTests -d -fe -E 0.001
