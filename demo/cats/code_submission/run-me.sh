#! /bin/bash
# set -o xtrace

export PAPER=cats

. function-declaration.sh

# Untar file
tar -xvf cats.source.tar cats

# Get vw dependencies
# Build vw
cd cats
mkdir build
cd build
cmake ..
make -j vw_cli_bin

# Download data files
cd ..
mkdir test/train-sets
mkdir test/train-sets/regression
curl -Ss -o test/train-sets/regression/BNG_wisconsin.csv https://www.openml.org/data/get_csv/150677/BNG_wisconsin.arff &
curl -Ss -o test/train-sets/regression/BNG_cpu_act.csv https://www.openml.org/data/get_csv/150680/BNG_cpu_act.arff &
curl -Ss -o test/train-sets/regression/BNG_auto_price.csv https://www.openml.org/data/get_csv/150679/BNG_auto_price.arff &
curl -Ss -o test/train-sets/regression/black_friday.csv https://www.openml.org/data/get_csv/21230845/file639340bd9ca9.arff &
curl -Ss -o test/train-sets/regression/zurich.csv https://www.openml.org/data/get_csv/5698591/file62a9329beed2.arff &

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
python3 papers/${PAPER}/utility/preprocess_data.py -c ./test/train-sets/regression/BNG_wisconsin.csv &
python3 papers/${PAPER}/utility/preprocess_data.py -c ./test/train-sets/regression/BNG_cpu_act.csv &
python3 papers/${PAPER}/utility/preprocess_data.py -c ./test/train-sets/regression/BNG_auto_price.csv &
python3 papers/${PAPER}/utility/preprocess_data.py -c ./test/train-sets/regression/black_friday.csv &
python3 papers/${PAPER}/utility/preprocess_data.py -c ./test/train-sets/regression/zurich.csv &

# Create additional synthetic datasets
python3 papers/${PAPER}/utility/create_synthetic_data.py &

# Wait for all background jobs to finish
wait

ds_min=$(cat ./test/train-sets/regression/ds_1000000.min)
ds_max=$(cat ./test/train-sets/regression/ds_1000000.max)

# Run experiments
mkdir results

# Online:
run_online "black_friday" 185 23959
run_online "BNG_auto_price" 379.36 43392.42
run_online "BNG_cpu_act" -64.9 187.54
run_online "BNG_wisconsin" -10.87 168.12
run_online "zurich" -121830 7190
run_online "ds_1000000" $ds_min $ds_max

# Offline:
run_offline "black_friday" 185 23959
run_offline "BNG_auto_price" 379.36 43392.42
run_offline "BNG_cpu_act" -64.9 187.54
run_offline "BNG_wisconsin" -10.87 168.12
run_offline "zurich" -121830 7190
run_offline "ds_1000000" $ds_min $ds_max
