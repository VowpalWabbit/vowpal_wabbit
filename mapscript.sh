hadoop fs -rmr $1; 
nmappers=$2;
total=`hadoop fs -ls ngd10 | cut -d " " -f 7 | awk 'BEGIN{sum = 0} {if(NF > 0) sum += $1;} END{print sum;}'`
echo $total
mapsize=`expr $total / $nmappers`
maprem=`expr $total % $nmappers`
mapsize=`expr $mapsize + $maprem`
mapsize=`expr $mapsize + 100`
echo $mapsize
killall allreduce_master
./allreduce_master $3 2
master=`hostname`
mapcommand="runvw.sh $1 $master"
echo $mapcommand
hadoop jar $HADOOP_HOME/hadoop-streaming.jar -Dmapred.job.queue.name=unfunded -Dmapred.min.split.size=$mapsize -Dmapred.reduce.tasks=0 -Dmapred.job.map.memory.mb=3000 -Dmapred.child.java.opts="-Xmx400m" -Dmapred.task.timeout=600000000 -input ngd10 -output $1 -file vw -file runvw.sh -mapper "$mapcommand" -reducer NONE