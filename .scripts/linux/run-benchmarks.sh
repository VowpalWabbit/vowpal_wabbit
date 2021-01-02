#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

./build/test/benchmarks/vw-benchmarks.out --benchmark_repetitions=10 --benchmark_min_time=2 \
    --benchmark_report_aggregates_only=true --benchmark_format=console \
    --benchmark_out_format=json --benchmark_out="$1" 
