#!/bin/bash
mapper=`printenv mapred_task_id | cut -d "_" -f 5`
rm -f temp.cache
date +"%F %T Start training mapper=$mapper" > /dev/stderr
vwcmd="./vw -b 24 --total $mapred_map_tasks --node $mapper --cache_file temp.cache --span_server $mapreduce_job_submithost --loss_function=logistic"
mapred_job_id=`echo $mapred_job_id | tr -d 'job_'`
gdcmd="$vwcmd --unique_id $mapred_job_id --passes 1 --adaptive --exact_adaptive_norm -d /dev/stdin -f tempmodel"
mapred_job_id=`expr $mapred_job_id \* 2` #create new nonce
bfgscmd="$vwcmd --unique_id $mapred_job_id --bfgs --mem 5 --passes 20 -f model -i tempmodel"
if [ "$mapper" == '000000' ]; then
    $gdcmd > mapperout 2>&1
    if [ $? -ne 0 ]; then
      date +"%F %T Failed mapper=$mapper cmd=$gdcmd" > /dev/stderr
      exit 1
    fi
    $bfgscmd >> mapperout 2>&1
    outfile=$mapred_output_dir/model
    mapperfile=$mapred_output_dir/mapperout
    found=`hadoop fs -lsr | grep $mapred_output_dir | grep mapperout`
    if [ "$found" != "" ]; then
      hadoop fs -rm -r $mapperfile
    fi
    found=`hadoop fs -lsr | grep $mapred_output_dir | grep model`
    if [ "$found" != "" ]; then
      hadoop fs -rm -r $outfile
    fi
    date +"%F %T outfile=$outfile" > /dev/stderr
    hadoop fs -put model $outfile
    hadoop fs -put mapperout $mapperfile
else
    $gdcmd
    if [ $? -ne 0 ]; then
      date +"%F %T Failed mapper=$mapper cmd=$gdcmd" > /dev/stderr
      exit 1
    fi
    $bfgscmd
fi
date +"%F %T Done mapper=$mapper" > /dev/stderr
