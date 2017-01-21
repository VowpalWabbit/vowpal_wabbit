#!/usr/bin/env bash

set -e

dryrun=$1

# MR1 sets $mapred_map_tasks
# MR2/YARN sets $mapreduce_job_maps
nmappers=$mapreduce_job_maps

# MR1 sets $mapreduce_job_submithost
# MR2/YARN sets $mapreduce_job_submithostname
submit_host=$mapreduce_job_submithostname

# MR1 sets $mapred_output_dir
# MR2/YARN sets $mapreduce_output_fileoutputformat_outputdir
output_dir=$mapreduce_output_fileoutputformat_outputdir

set -u

# This works on both MR1 and MR2/YARN
mapper=`printenv mapred_task_id | cut -d "_" -f 5`
mapred_job_id=`echo "$mapred_job_id" | awk -F "_" '{print $NF}'`

# debug
echo $mapper > /dev/stderr
echo $nmappers > /dev/stderr
echo $output_dir > /dev/stderr
echo $submit_host > /dev/stderr

rm -f temp.cache || true

echo 'Starting training' > /dev/stderr
# SGD step
gdcmd="./vw -b 20
        --total $nmappers
        --node $mapper
        --unique_id $mapred_job_id
        --passes 2
        --save_per_pass
        --readable_model sgd.rmodel
        -d /dev/stdin
        -f sgd.vwmodel
        --cache_file temp.cache
        --span_server $submit_host
        --loss_function=logistic"

# BFGS step
mapred_job_id=`expr $mapred_job_id \* 2` #create new nonce
bfgscmd="./vw
        --total $nmappers
        --node $mapper
        --unique_id $mapred_job_id
        --cache_file temp.cache
        --bfgs
        --mem 5
        --passes 2
        --save_per_pass
        --readable_model bfgs.rmodel
        --span_server $submit_host
        -f bfgs.vwmodel
        -i sgd.vwmodel
        --loss_function=logistic"

if [ "$mapper" == '000000' ]
then
    if [ -z ${dryrun:-} ]
    then
        echo "SGD ..." > /dev/stderr
        $gdcmd > >(tee vw.out) 2> >(tee vw.err >&2)
        echo "BFGS ..." > /dev/stderr
        $bfgscmd > >(tee -a vw.out) 2> >(tee -a vw.err >&2)
    else
        echo "Dryrrun"
        echo $gdcmd
        set
        cat > /dev/null
    fi

    if [ $? -ne 0 ]
    then
       exit 5
    fi

    # store models and output in hdfs
    hadoop fs -put -f sgd.vwmodel* $output_dir || true
    hadoop fs -put -f sgd.rmodel* $output_dir || true
    hadoop fs -put -f bfgs.vwmodel* $output_dir || true
    hadoop fs -put -f bfgs.rmodel* $output_dir || true
    hadoop fs -put -f vw.* $output_dir || true

else
    if [ -z ${dryrun:-} ]
    then
        echo "SGD ..."
        $gdcmd
        echo "BFGS ..."
        $bfgscmd
    else
        echo "Dryrrun"
        echo $gdcmd
        echo $bfgscmd
        cat > /dev/null
    fi

    if [ $? -ne 0 ]
    then
       exit 6
    fi
fi
