#! /bin/bash

# NOTES:  This script should be copied outside the repo root
#         Check sanitize.py for keywords to look for in comments to remove

export PAPER=cats
export BRANCH=icml_push
rm -rf ${PAPER}
git clone https://github.com/VowpalWabbit/vowpal_wabbit.git ${PAPER}
cd ${PAPER}
git checkout ${BRANCH}
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
find . -name "*.cc" -exec sh -c 'python3 papers/${PAPER}/code_submission/sanitize.py -s "$1" > "$1.clean"; mv "$1" "$1.old"; mv "$1.clean" "$1"' == {} \;
find . -name "*.h" -exec sh -c 'python3 papers/${PAPER}/code_submission/sanitize.py -s "$1" > "$1.clean"; mv "$1" "$1.old"; mv "$1.clean" "$1"' == {} \;
find . -name "*.old" -exec sh -c 'rm "$1"' == {} \;

# Get all dependencies
git submodule update --init --recursive

# Create zip file
rm -rf .git
cd ..
cp ${PAPER}/papers/${PAPER}/code_submission/run-me.sh .
cp ${PAPER}/papers/${PAPER}/code_submission/run-manual.sh .
cp ${PAPER}/papers/${PAPER}/code_submission/function-declaration.sh .
cp ${PAPER}/papers/${PAPER}/code_submission/README.txt .
rm -rf ${PAPER}/papers/${PAPER}/code_submission
tar -cvf ${PAPER}.source.tar ${PAPER}
zip source-code.zip ${PAPER}.source.tar run-me.sh run-manual.sh function-declaration.sh README.txt

# Clean up temporaries
rm run-me.sh
rm run-manual.sh
rm function-declaration.sh
rm ${PAPER}.source.tar
rm README.txt