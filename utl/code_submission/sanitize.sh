#! /bin/bash

# NOTES:  This script should be copied outside the repo root
#         Check sanitize.py for keywords to look for in comments to remove

rm -rf cats
git clone https://github.com/SoftwareBuildingBlocks/vowpal_wabbit.git cats
cd cats
git checkout icml_push2
rm -rf cs
rm -rf cluster
rm -rf demo
rm -rf logo_assets
rm -rf java
rm -rf test
rm -rf python
rm -rf R
mkdir cluster
mkdir test
mkdir python
touch cluster/CMakeLists.txt
touch test/CMakeLists.txt
touch python/CMakeLists.txt

# Sanitize sources
find . -name "*.cc" -exec sh -c 'python3 utl/code_submission/sanitize.py -s "$1" > "$1.clean"; mv "$1" "$1.old"; mv "$1.clean" "$1"' == {} \;
find . -name "*.h" -exec sh -c 'python3 utl/code_submission/sanitize.py -s "$1" > "$1.clean"; mv "$1" "$1.old"; mv "$1.clean" "$1"' == {} \;
find . -name "*.old" -exec sh -c 'rm "$1"' == {} \;

# Get all dependencies
git submodule update --init --recursive

# Create zip file
rm -rf .git
cd ..
cp utl
tar -cvf cats.source.tar cats
zip source.zip cats.source.tar