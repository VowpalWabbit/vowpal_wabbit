mapper=`printenv mapred_task_id | cut -d "_" -f 5`
echo $mapper > /dev/stderr
./vw --cache_file temp.cache -b 24 --conjugate_gradient --passes 4 -q up -q ua -q us -q pa -q ps -q as --loss_function=logistic --regularization=1 --master_location $2 -d /dev/stdin -f model 
if [ "$mapper" == '000000' ]
then
    outfile=$1/model
    echo $outfile > /dev/stderr
    hadoop fs -rmr $outfile
    hadoop fs -put model $outfile
fi