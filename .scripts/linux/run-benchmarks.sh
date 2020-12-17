#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

./build/test/benchmarks/vw-benchmarks.out --benchmark_format=json --benchmark_repetitions=10 --benchmark_min_time=2 --benchmark_display_aggregates_only=false --benchmark_report_aggregates_only=true | tee "$1"
