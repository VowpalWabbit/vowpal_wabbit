#!/bin/bash
rm -f train.cache train.w
../vowpalwabbit/vw -c -d train -f train.w -q st --passes 100 --hash all --noconstant --csoaa_ldf m
../vowpalwabbit/vw -t -d train -i train.w -p train.pred --noconstant  --csoaa_ldf m

