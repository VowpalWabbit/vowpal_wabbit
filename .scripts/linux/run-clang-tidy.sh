#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# -DCMAKE_EXPORT_COMPILE_COMMANDS=On is manually set because in CMake 3.16, just setting it in the CMakeLists.txt does not work.
cmake -S . -B build -DFMT_SYS_DEP=On -DSPDLOG_SYS_DEP=On -DVW_BOOST_MATH_SYS_DEP=On -DBUILD_TESTING=Off -DVW_BUILD_VW_C_WRAPPER=Off -DCMAKE_EXPORT_COMPILE_COMMANDS=On
# remove grep to view warnings and not only errors
run-clang-tidy -p build -quiet -header-filter=vw/* 1>tidy_out.txt 2>tidy_error.txt || true
grep -A 3 ": error" -f tidy_out.txt > tidy_onlyerrors.txt

if [ -s tidy_onlyerrors.txt ]; then
        # The file is not-empty.
        cat tidy_onlyerrors.txt
        exit 11
else
        # The file is empty.
        cat tidy_error.txt
fi
