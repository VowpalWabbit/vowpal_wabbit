#!/bin/bash

i=00
data=BNG_cpu_act.dat
min=-64.9
max=187.54
pass=1
timee=5000
pp=10000
bb=24
ll=1
filename=../results/$data\_$i.txt

echo "CATS:" > $filename

for n in 4 #8 16 32 64 128 256 512 1024 2048 4096 8192 16384 
do
for h in 1 #2 4 8 16 32 64 128
do
if (( $n > 2*$h ));
then
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --min_value=$min --max_value=$max --bandwidth $h \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename 2>&1 ) 2>> $filename
fi;
done
done


printf "\n\nDiscretized Tree:----------------------------------------------------------------------------------------------------------------------------" >> $filename
for n in 2 #4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 
do
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --otc $n --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --otc $n --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename 2>&1 ) 2>> $filename
done


printf "\n\nDiscretized Linear:----------------------------------------------------------------------------------------------------------------------------" >> $filename
for n in 2 #4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 
do
printf "\n\ntime -p timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll \n\n" >> $filename
time -p ( timeout $timee ../build/vowpalwabbit/vw --cbify $n --cbify_reg --cb_discrete --min_value=$min --max_value=$max \
-d ../test/train-sets/regression/$data --passes $pass --cache_file=$data\_$bb.cache -b $bb --coin --loss_option $ll >> $filename 2>&1 ) 2>> $filename
done
