#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$(realpath $SCRIPT_DIR/../../)
cd $REPO_DIR

# Assumes cmake has been configured to build directory

cat $REPO_DIR/build/compile_commands.json | \
    jq -r ".[] | select(.file | startswith(\"$REPO_DIR/vowpalwabbit\")) | .file" | \
    xargs -n 1 -I {} clang-tidy -header-filter=$REPO_DIR/vowpalwabbit/* -p=build {}
