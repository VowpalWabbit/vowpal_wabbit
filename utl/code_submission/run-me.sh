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
curl -o test/train-sets/regression/BNG_wisconsin.csv https://www.openml.org/data/get_csv/150677/BNG_wisconsin.arff &
curl -o test/train-sets/regression/BNG_cpu_act.csv https://www.openml.org/data/get_csv/150680/BNG_cpu_act.arff &
curl -o test/train-sets/regression/BNG_auto_price.csv https://www.openml.org/data/get_csv/150679/BNG_auto_price.arff &
curl -o test/train-sets/regression/black_friday.csv https://www.openml.org/data/get_csv/21230845/file639340bd9ca9.arff &
curl -o test/train-sets/regression/zurich.csv https://www.openml.org/data/get_csv/5698591/file62a9329beed2.arff &

# Wait for all background jobs to finish
wait

# Remove empty lines from data
sed '/^$/d' -i ./test/train-sets/regression/BNG_wisconsin.csv &
sed '/^$/d' -i ./test/train-sets/regression/BNG_cpu_act.csv &
sed '/^$/d' -i ./test/train-sets/regression/BNG_auto_price.csv &
sed '/^$/d' -i ./test/train-sets/regression/black_friday.csv &
sed '/^$/d' -i ./test/train-sets/regression/zurich.csv &

# Wait for all background jobs to finish
wait

# Transform data files
python3 utl/continous_action/preprocess_data.py -c ./test/train-sets/regression/BNG_wisconsin.csv &
python3 utl/continous_action/preprocess_data.py -c ./test/train-sets/regression/BNG_cpu_act.csv &
python3 utl/continous_action/preprocess_data.py -c ./test/train-sets/regression/BNG_auto_price.csv &
python3 utl/continous_action/preprocess_data.py -c ./test/train-sets/regression/black_friday.csv &
python3 utl/continous_action/preprocess_data.py -c ./test/train-sets/regression/zurich.csv &

# Wait for all background jobs to finish
wait

# Run experiments
echo "Run experiments"
