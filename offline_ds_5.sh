#!/bin/bash

i=14
datatrain=ds_5_train.dat
datatest=ds_5_test.dat
sdata=ds
min=0.61
max=9.96
pass=1
timee=5000
pp=10000
bb=18
ll=1
filename=results/$sdata\_offline\_srm.txt
filename2=results/$sdata\_offline\_test.txt

# online training and saving logged data
nn=4
hh=1

time -p timeout $timee build/vowpalwabbit/vw --cbify $nn --cbify_reg --min_value=$min --max_value=$max --bandwidth $hh \
-d test/train-sets/regression/$datatrain --passes $pass -b $bb --coin --loss_option $ll -p results/$sdata\_$ll.acp

python3 utl/continous_action/acp_regression_data_join.py --p results/$sdata\_$ll.acp --d test/train-sets/regression/$datatrain > results/$sdata\_$ll.acpx

# offline training and saving srm and test results

printf "." > $filename
printf "." > $filename2

for n in 4 #8 16 32 64 128 256 512 1024 2048 4096 8192 
do
for h in 1 #2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
printf "\ntime -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap \n" >> $filename
time -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap
printf "\n($n, $h)\n" >> $filename
python3 utl/continous_action/srm.py --p results/$sdata\_$ll\_$n\_$h.ap --d results/$sdata\_$ll.acp -m $max -i $min -k $n --bandwidth $h >> $filename
printf "\ntime -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t \n" >> $filename2
time -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
-d test/train-sets/regression/$datatest --passes $pass -b $bb --coin --loss_option $ll -i results/$sdata.m -p results/$sdata.ap -t
printf "\n($n, $h)\n" >> $filename2
python3 utl/continous_action/ap_regression_data_join.py --p results/$sdata.ap --d test/train-sets/regression/$datatest -m $max -i $min >> $filename2

fi;
done
done
