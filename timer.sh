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

# echo "text million"
# for i in {1..8}; do 
# echo $i
# avg_time 10 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i
# sleep 1
# done;

# echo "text without learner million"
# for i in {1..8}; do 
# echo $i
# avg_time 10 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i --no_learner
# sleep 1
# done;

# echo "creating cache million"
# build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=2 -c -k --quiet

# echo "cache million"
# for i in {1..8}; do 
# echo $i
# avg_time 10 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i -c
# sleep 1
# done;

# echo "cache without learner million"
# for i in {1..8}; do 
# echo $i
# avg_time 10 build/vowpalwabbit/vw ../0002_million.dat --num_parse_threads=$i --no_learner -c
# sleep 1
# done;

echo "text 4million"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0002_4million.dat --num_parse_threads=$i
sleep 1
done;

echo "text without learner 4million"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0002_4million.dat --num_parse_threads=$i --no_learner
sleep 1
done;

echo "creating cache 4million"
build/vowpalwabbit/vw ../0002_4million.dat --num_parse_threads=2 -c -k --quiet

echo "cache 4million"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0002_4million.dat --num_parse_threads=$i -c
sleep 1
done;

echo "cache without learner 4million"
for i in {1..8}; do 
echo $i
avg_time 10 build/vowpalwabbit/vw ../0002_4million.dat --num_parse_threads=$i --no_learner -c
sleep 1
done;