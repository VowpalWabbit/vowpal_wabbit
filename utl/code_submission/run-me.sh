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

# should be repeated for all data sets

# Run experiments

# parameteres for all the experiments
pass=1
timee=5000
pp=10000
bb=18
ll=1
nn=4
hh=1


# parametrs for each data set
name=black_friday
min=185
max=23959

# name=BNG_auto_price
# min=379.36
# max=43392.42

# name=BNG_cpu_act
# min=-64.9
# max=187.54

# name=BNG_wisconsin
# min=-10.87
# max=168.12

# name=ds_5
# min=0.61
# max=9.96

# name=zurich
# min=-121830
# max=7190

datatrain=$name\_train.dat
datatest=$name\_test.dat
data=$name.dat
sdata=$name
filename1=results/$sdata\_offline\_srm.txt
filename2=results/$sdata\_offline\_test.txt
filename3=../results/$sdata\_online_validation.txt

# running the CATS offline algorithm and saving the loss estimation in SRM and test error results

# 1) saving logged data from training the initial model

time -p timeout $timee build/vowpalwabbit/vw --cbify $nn --cbify_reg --min_value=$min --max_value=$max --bandwidth $hh \
-d test/train-sets/regression/$datatrain --passes $pass -b $bb --coin --loss_option $ll -p results/$sdata\_$ll.acp

python3 utl/continous_action/acp_regression_data_join.py --p results/$sdata\_$ll.acp --d test/train-sets/regression/$datatrain > results/$sdata\_$ll.acpx

# 2) offline training a new model and saving the loss estimation in SRM and test error results

printf "." > $filename1
printf "." > $filename2

for n in 4 #8 16 32 64 128 256 512 1024 2048 4096 8192
do
for h in 1 #2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
printf "\ntime -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap \n" >> $filename1
time -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap
printf "\n($n, $h)\n" >> $filename1
python3 utl/continous_action/srm.py --p results/$sdata\_$ll\_$n\_$h.ap --d results/$sdata\_$ll.acp -m $max -i $min -k $n --bandwidth $h >> $filename1
printf "\ntime -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t \n" >> $filename2
time -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t
printf "\n($n, $h)\n" >> $filename2
python3 utl/continous_action/ap_regression_data_join.py --p results/$sdata.ap --d test/train-sets/regression/$datatest -m $max -i $min >> $filename2

fi;
done
done


# running the online algorithms for CATS as well as the comparators and saving the progressive validation results

echo "CATS online:" > $filename3

for n in 4 #8 16 32 64 128 256 512 1024 2048 4096 8192
do
for h in 1 #2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
fi;
done
done


printf "\n\nDiscretized Tree online:-----------------------------------------------------------------------------" >> $filename3
for n in 2 #4 8 16 32 64 128 256 512 1024 2048 4096 8192
do
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --otc $n --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --otc $n --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
done


printf "\n\nDiscretized Linear online:----------------------------------------------------------------------------" >> $filename3
for n in 2 #4 8 16 32 64 128 256 512 1024 2048 4096 8192
do
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename3
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename3 2>&1 ) 2>> $filename3
done