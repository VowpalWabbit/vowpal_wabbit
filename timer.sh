#!/bin/bash

avg_time() {
    #
    # usage: avg_time n command ...
    #
    n=$1; shift
    (($# > 0)) || return                   # bail if no command given
    for ((i = 0; i < n; i++)); do
        { time -p "$@" &>/dev/null; } 2>&1 # ignore the output of the command
                                           # but collect time's output in stdout
    done | awk '
        /real/ { real = real + $2; nr++ }
        /user/ { user = user + $2; nu++ }
        /sys/  { sys  = sys  + $2; ns++}
        END    {
                 if (nr>0) printf("%f\n", real/nr);
                #  if (nu>0) printf("user %f\n", user/nu);
                #  if (ns>0) printf("sys %f\n",  sys/ns)
               }'
}

echo "creating cache"
build/vowpalwabbit/vw ../0001_million.dat -c -k --quiet
build/vowpalwabbit/vw ../0002_million.dat -c -k --quiet
echo ""

echo "CACHE"
echo "0001 dataset"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0001_million.dat --num_parse_threads=$i -c
sleep 1
done;
echo "0002 dataset"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i -c
sleep 1
done;
echo ""

echo "TEXT"
echo "0001 dataset"
for i in {1..8}; do 
echo $i
avg_time 5 build/vowpalwabbit/vw ../0001_million.dat --num_parse_threads=$i
sleep 1
done;
echo "0002 dataset"
for i in {1..8}; do 
echo $i
avg_time 5 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i
sleep 1
done;
echo ""

echo "json"
for i in {1..8}; do 
echo $i
avg_time 3 build/vowpalwabbit/vw ../0001_million.json --num_parse_threads=$i --json
sleep 1
done;
echo ""