#! /bin/bash
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

## Create zip file
### Remove this script and sanitizing py script
## rm -rf utl
## rm -rf .git
## cd ..
## tar -cvf cats.source.tar cats
## zip cats.source.tar.zip cats.source.tar

# Build vw
git submodule update --init --recursive
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/ranaras/s/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j vw-bin


