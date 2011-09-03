mapper=`printenv mapred_task_id | cut -d "_" -f 5`
echo $mapper > /dev/stderr
rm -f temp.cache
echo 'Starting training' > /dev/stderr
echo $1 > /dev/stderr
gdcmd="./vw -b 24 --total $mapred_map_tasks --node $mapper --unique_id 0$mapred_job_id --cache_file temp.cache --passes 1 --regularization=1 --adaptive --exact_adaptive_norm -d /dev/stdin -f tempmodel --master_location $mapreduce_job_submithost --loss_function=logistic" 
bfgscmd="./vw -b 24 --total $mapred_map_tasks --node $mapper --unique_id 1$mapred_job_id --cache_file temp.cache --bfgs --mem 5 --passes 20 --regularization=1 --master_location $mapreduce_job_submithost -f model -i tempmodel --loss_function=logistic"
if [ "$mapper" == '000000' ]
then
    $gdcmd > mapperout 2>&1
    if [ $? -ne 0 ] 
    then
       exit 1
    fi 
    $bfgscmd >> mapperout 2>&1
    outfile=$mapred_output_dir/model
    mapperfile=$mapred_output_dir/mapperout
    found=`hadoop fs -lsr | grep $mapred_output_dir | grep mapperout`
    if [ "$found" != "" ]
    then
	hadoop fs -rmr $mapperfile
    fi
    found=`hadoop fs -lsr | grep $mapred_output_dir | grep model`
    if [ "$found" != "" ]
    then
	hadoop fs -rmr $outfile
    fi
    echo $outfile > /dev/stderr
    hadoop fs -put model $outfile
    hadoop fs -put mapperout $mapperfile
else
    $gdcmd
    if [ $? -ne 0 ]    
    then
       exit 1
    fi
    $bfgscmd
fi
