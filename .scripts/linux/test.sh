#!/bin/bash
set -e
set -x

cd $1
cd build
make test_with_output
