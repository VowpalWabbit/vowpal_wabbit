#! /bin/bash

# Untar file
tar -xvf cats.source.tar cats

# Get vw dependencies
# Build vw
cd cats
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/ranaras/s/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j vw-bin

# Download data files
cd ..
mkdir test/train-sets
mkdir test/train-sets/regression
curl -o test/train-sets/regression/BNG_wisconsin.csv https://www.openml.org/data/get_csv/150677/BNG_wisconsin.arff
curl -o test/train-sets/regression/BNG_cpu_act.csv https://www.openml.org/data/get_csv/150680/BNG_cpu_act.arff
curl -o test/train-sets/regression/BNG_auto_price.csv https://www.openml.org/data/get_csv/150679/BNG_auto_price.arff
curl -o test/train-sets/regression/black_friday.csv https://www.openml.org/data/get_csv/21230845/file639340bd9ca9.arff
curl -o test/train-sets/regression/zurich.csv https://www.openml.org/data/get_csv/5698591/file62a9329beed2.arff

# Transform data files


# Run experiments
