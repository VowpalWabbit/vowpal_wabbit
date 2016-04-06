#! /bin/bash

paste -d' ' \
 <(gunzip -c $1 | $testCodeDir/mnist.extract-labels.pl) \
 <(gunzip -c $2 | $testCodeDir/mnist.extractfeatures)
