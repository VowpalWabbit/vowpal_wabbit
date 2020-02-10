#!/bin/bash

i=24
datatrain=ds_5_train.dat
sdata=ds
min=0.61
max=9.96
pass=1
timee=5000
pp=10000
bb=18
ll=2
filename=results/$sdata\_$i.txt

printf "." > $filename

for n in 4 8 16 32 64 128 256 512 1024 2048 4096 8192
do
for h in 1 2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
printf "\n($n, $h)\n" >> $filename
python3 utl/continous_action/srm.py --p results/$sdata\_$ll\_$n\_$h.ap --d results/$sdata\_$ll.acp -m $max -i $min -k $n --bandwidth $h >> $filename
fi;
done
done
