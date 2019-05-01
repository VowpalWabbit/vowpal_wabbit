#!/bin/bash
set -e
set -x

# Check if any clang-formatting necessary
cd $1
./utl/clang-format check