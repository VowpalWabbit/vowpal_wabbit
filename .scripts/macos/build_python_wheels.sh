#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

# This script is intended to be used locally to build arm64 wheels for MacOS.

if ! command -v conda &> /dev/null
then
    echo "conda not installed"
    exit
fi

eval "$(conda shell.bash hook)"

rm -rf wheel_output
rm -rf build

conda create -y --name macos_python_build_env_38 python=3.8 wheel zlib boost py-boost flatbuffers
conda activate macos_python_build_env_38
pip wheel . -w wheel_output/ --global-option \
    --cmake-options="-DSTATIC_LINK_VW_JAVA=On;-DPython_INCLUDE_DIR=\"$CONDA_PREFIX/include/python3.8/\"" --verbose

rm -rf build
conda create -y --name macos_python_build_env_39 python=3.9 wheel zlib boost py-boost flatbuffers
conda activate macos_python_build_env_39
pip wheel . -w wheel_output/ --global-option \
    --cmake-options="-DSTATIC_LINK_VW_JAVA=On;-DPython_INCLUDE_DIR=\"$CONDA_PREFIX/include/python3.9/\"" --verbose
