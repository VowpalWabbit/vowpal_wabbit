out_directory=$1
master=$2
mapper=`printenv mapred_task_id | cut -d "_" -f 5`
echo $mapper > /dev/stderr
rm -f temp.cache
echo 'Starting training' > /dev/stderr
echo $1 > /dev/stderr
#./vw -b 24 --cache_file temp.cache --passes 20 --regularization=1 -d /dev/stdin -f tempmodel --master_location $2 --bfgs --mem 5 
#./vw -b 24 --cache_file temp.cache --passes 10 --regularization=1 --loss_function=logistic -d /dev/stdin -f tempmodel --master_location $2 --bfgs --mem 5 
#./vw -b 24 --cache_file temp.cache --passes 1 -d /dev/stdin -i tempmodel -t
gdcmd="./vw -q aq -b 20 --cache_file temp.cache --passes 1 --regularization=1 --adaptive --exact_adaptive_norm -d /dev/stdin -f tempmodel --master_location $master --loss_function=logistic" 
bfgscmd="./vw -q aq -b 20 --cache_file temp.cache --bfgs --mem 5 --passes 20 --regularization=1 --master_location $master -f model -i tempmodel --loss_function=logistic"
if [ "$mapper" == '000000' ]
then
    $gdcmd > mapperout 2>&1
    #$bfgscmd >> mapperout 2>&1
    #outfile=$out_directory/model
    outfile=$out_directory/tempmodel
    mapperfile=$out_directory/mapperout
    found=`hadoop fs -lsr | grep $out_directory | grep mapperout`
    if [ "$found" != "" ]
    then
	hadoop fs -rmr $mapperfile
    fi
    found=`hadoop fs -lsr | grep $out_directory | grep model`
    if [ "$found" != "" ]
    then
	hadoop fs -rmr $outfile
    fi
    echo $outfile > /dev/stderr
    hadoop fs -put model $outfile
    hadoop fs -put mapperout $mapperfile
else
    $gdcmd
    #$bfgscmd
fi