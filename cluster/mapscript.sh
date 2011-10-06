out_directory=$1
in_directory=$2
nmappers=$3;
hadoop fs -rmr $out_directory > /dev/null 2>&1; 
total=`hadoop fs -ls $in_directory | cut -d " " -f 7 | awk 'BEGIN{sum = 0} {if(NF > 0) sum += $1;} END{print sum;}'`
echo $total
mapsize=`expr $total / $nmappers`
maprem=`expr $total % $nmappers`
mapsize=`expr $mapsize + $maprem`
mapsize=`expr $mapsize + 100`
echo $mapsize
./spanning_tree
hadoop jar $HADOOP_HOME/hadoop-streaming.jar -Dmapred.job.queue.name=search -Dmapred.min.split.size=$mapsize -Dmapred.map.tasks.speculative.execution=true -Dmapred.reduce.tasks=0 -Dmapred.job.map.memory.mb=3000 -Dmapred.child.java.opts="-Xmx100m" -Dmapred.task.timeout=600000000 -input $in_directory -output $out_directory -file ../vw -file runvw.sh -mapper runvw.sh -reducer NONE
