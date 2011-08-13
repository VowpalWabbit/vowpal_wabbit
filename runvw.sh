mapper=`printenv mapred_task_id | cut -d "_" -f 5`
echo $mapper > /dev/stderr
rm temp.cache
#./to_vw.py | awk 'BEGIN {srand();} {printf("%g;%s\n",rand(),$0);}' | sort -t ';' -g -k 1,1 | cut -d ';' -f 2 > train
#awk 'BEGIN {srand();} {printf("%g;%s\n",rand(),$0);}' | sort -t ';' -g -k 1,1 | cut -d ';' -f 2 > train
echo 'Starting training' > /dev/stderr
echo $1 > /dev/stderr
#./vw -b 24 --cache_file temp.cache --passes 20 --regularization=1 -d /dev/stdin -f tempmodel --master_location $2 --bfgs --mem 5 
#./vw -b 24 --cache_file temp.cache --passes 10 --regularization=1 --loss_function=logistic -d /dev/stdin -f tempmodel --master_location $2 --bfgs --mem 5 
#./vw -b 24 --cache_file temp.cache --passes 1 -d /dev/stdin -i tempmodel -t
#cat train | ./vw -b 24 --cache_file temp.cache --passes 1 -q up -q ua -q us -q pa -q ps -q as --loss_function=logistic --regularization=1 -d /dev/stdin -f tempmodel --master_location $2
gdcmd="./vw -b 24 --cache_file temp.cache --passes 1 --regularization=1 --adaptive --exact_adaptive_norm -d /dev/stdin -f tempmodel --master_location $2 --loss_function=logistic" 
#rm temp.cache
hybridcmd="./vw -b 24 --cache_file temp.cache --hybrid --mem 5 --passes 10 --regularization=1 --master_location $2 -f model -i tempmodel --loss_function=logistic"
if [ "$mapper" == '000000' ]
then
    $gdcmd > mapperout 2>&1
    $hybridcmd >> mapperout 2>&1
    outfile=$1/model
    mapperfile=$1/mapperout
    found=`hadoop fs -lsr | awk -v outdir="$1",outfile="mapperout" 'BEGIN {found = 0;} {split($NF,arr,"/"); if(arr[4] == outdir && arr[5] == outfile) found = 1;} END {print found;}'`
    if [ "$found" == "1" ]
    then
	hadoop fs -rmr $mapperfile
    fi
    echo $outfile > /dev/stderr
    hadoop fs -put model $outfile
    hadoop fs -put mapperout $mapperfile
else
    $gdcmd
    $hybridcmd
fi