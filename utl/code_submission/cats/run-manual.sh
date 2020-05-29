#! /bin/bash

. function-declaration.sh

cd cats

ds_min=$(cat ./test/train-sets/regression/ds_1000000.min)
ds_max=$(cat ./test/train-sets/regression/ds_1000000.max)

if [ $1 = 'friday' ] && [ $2 = 'off' ] 
then
    run_offline "black_friday" 185 23959
elif [ $1 = 'friday' ] && [ $2 = 'on' ] 
then
    run_online "black_friday" 185 23959
elif [ $1 = 'price' ] && [ $2 = 'off' ] 
then
    run_offline "BNG_auto_price" 379.36 43392.42
elif [ $1 = 'price' ] && [ $2 = 'on' ] 
then
    run_online "BNG_auto_price" 379.36 43392.42
elif [ $1 = 'cpu' ] && [ $2 = 'off' ] 
then
    run_offline "BNG_cpu_act" -64.9 187.54
elif [ $1 = 'cpu' ] && [ $2 = 'on' ] 
then
    run_online "BNG_cpu_act" -64.9 187.54
elif [ $1 = 'wis' ] && [ $2 = 'off' ] 
then
    run_offline "BNG_wisconsin" -10.87 168.12
elif [ $1 = 'wis' ] && [ $2 = 'on' ] 
then
    run_online "BNG_wisconsin" -10.87 168.12
elif [ $1 = 'zurich' ] && [ $2 = 'off' ] 
then
    run_offline "zurich" -121830 7190
elif [ $1 = 'zurich' ] && [ $2 = 'on' ] 
then
    run_online "zurich" -121830 7190
elif [ $1 = 'ds' ] && [ $2 = 'off' ] 
then
    run_offline "ds_1000000" $ds_min $ds_max
elif [ $1 = 'ds' ] && [ $2 = 'on' ] 
then
    run_online "ds_1000000" $ds_min $ds_max
fi
