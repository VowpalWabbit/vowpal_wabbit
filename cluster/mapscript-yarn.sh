#!/usr/bin/env bash

set -e

usage="$0 out_dir in_dir"
if [ "$2" == "" ]
then
	echo $usage
	exit
fi

set -u

out_directory=$1
in_directory=$2

hadoop fs -rmr $out_directory > /dev/null 2>&1 || true

./spanning_tree || true

hadoop jar /usr/lib/hadoop-mapreduce/hadoop-streaming.jar \
        -Dmapred.job.name="vw allreduce $in_directory" \
	-Dmapred.map.tasks.speculative.execution=true \
	-Dmapred.reduce.tasks=0 \
	-Dmapred.child.java.opts="-Xmx100m" \
	-Dmapred.task.timeout=600000000 \
        -Dmapred.job.map.memory.mb=1000 \
	-input $in_directory \
	-output $out_directory \
	-file ../vowpalwabbit/vw \
	-file /usr/lib64/libboost_program_options.so.5 \
	-file /lib64/libz.so.1 \
	-file runvw-yarn.sh \
	-mapper runvw-yarn.sh \
	-reducer NONE 

