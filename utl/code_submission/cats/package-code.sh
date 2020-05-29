#! /bin/bash

# NOTES:  This script should be copied outside the repo root
#         Check sanitize.py for keywords to look for in comments to remove

rm -rf cats
git clone https://github.com/SoftwareBuildingBlocks/vowpal_wabbit.git cats
cd cats
git checkout icml_push
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
cp cats/utl/code_submission/run-me.sh .
cp cats/utl/code_submission/function-declaration.sh .
cp cats/utl/code_submission/README.txt .
rm -rf cats/utl/code_submission
tar -cvf cats.source.tar cats
zip source.zip cats.source.tar run-me.sh function-declaration.sh README.txt

# Clean up temporaries
rm run-me.sh
rm function-declaration.sh
rm cats.source.tar
rm README.txt