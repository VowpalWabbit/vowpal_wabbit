#!/bin/bash

set -e

./rl_test.out -j client.json -t 1 -n 10000 -x 1 -a 1 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 10000 -x 1 -a 10 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 10000 -x 1 -a 100 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 1 -a 1000 -e linux_perf -f -p

./rl_test.out -j client.json -t 1 -n 10000 -x 10 -a 1 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 10000 -x 10 -a 10 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 10 -a 100 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 10 -a 1000 -e linux_perf -f -p

./rl_test.out -j client.json -t 1 -n 10000 -x 100 -a 1 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 100 -a 10 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 100 -a 100 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 100 -a 1000 -e linux_perf -f -p

./rl_test.out -j client.json -t 1 -n 1000 -x 1000 -a 1 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 1000 -a 10 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 1000 -x 1000 -a 100 -e linux_perf -f -p
./rl_test.out -j client.json -t 1 -n 100 -x 1000 -a 1000 -e linux_perf -f -p
