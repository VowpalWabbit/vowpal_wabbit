#!/bin/bash

i=24
datatrain=BNG_wisconsin_train.dat
sdata=wisconsin
min=-10.87
max=168.12
pass=1
timee=5000
pp=10000
bb=18
ll=2
filename=results/$sdata\_$i.txt

# nn=4
# hh=1

# time -p timeout $timee build/vowpalwabbit/vw --cbify $nn --cbify_reg --min_value=$min --max_value=$max --bandwidth $hh \
# -d test/train-sets/regression/$datatrain --passes $pass -b $bb --coin --loss_option $ll -p results/$sdata\_$ll.acp

# python utl/continous_action/acp_regression_data_join.py --p results/$sdata\_$ll.acp --d test/train-sets/regression/$datatrain > results/$sdata\_$ll.acpx


printf "." > $filename

for n in 4 8 16 32 64 128 256 512 1024 2048 4096 8192 
do
for h in 1 2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
# printf "\ntime -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
# -d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap \n" >> $filename
# time -p timeout $timee build/vowpalwabbit/vw --cont_tbd $n --min_value=$min --max_value=$max --bandwidth $h \
# -d results/$sdata\_$ll.acpx --passes $pass -b $bb --coin --loss_option $ll -f results/$sdata.m -p results/$sdata\_$ll\_$n\_$h.ap
printf "\n($n, $h)\n" >> $filename
python utl/continous_action/srm.py --p results/$sdata\_$ll\_$n\_$h.ap --d results/$sdata\_$ll.acp -m $max -i $min -k $n --bandwidth $h >> $filename

fi;
done
done
