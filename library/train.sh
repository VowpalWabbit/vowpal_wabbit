#!/bin/bash
rm -f train.cache train.w
../vowpalwabbit/vw -c -k -d train -f train.w -q st --passes 100 --hash all --noconstant --csoaa_ldf m
../vowpalwabbit/vw -t    -d train -i train.w -q st              --hash all --noconstant --csoaa_ldf m -p train.pred


