#!/usr/bin/python

import sys
from subprocess import call, Popen, PIPE
from math import log, exp
from os import devnull

# The learning algorithm is vowpal wabbit, available at https://github.com/JohnLangford/vowpal_wabbit/wiki
vw_train_cmd = '../../vowpalwabbit/vw  -c -f model --bfgs --passes 30 -b 22 --loss_function logistic --l2 14 --termination 0.00001 --holdout_off'
vw_test_cmd = '../../vowpalwabbit/vw  -t -i model -p /dev/stdout'

def get_features(line):
    feat = line[2:]
    # Bucketizing the integer features on a logarithmic scale
    for i in range(8):
        if feat[i]: 
            v = int(feat[i])
            if v>0:
                feat[i] = str(int(log(v+0.5)/log(1.5)))
    return ' '.join(['%d_%s' % (i,v) for i,v in enumerate(feat) if v])

def train_test_oneday(day):
    ts_beginning_test = 86400*(day-1)

    with open('data.txt') as f:
        line = f.readline()

        # Beginning of the training set: 3 weeks before the test period
        while int(line.split()[0]) < ts_beginning_test - 86400*21:
            line = f.readline()

        call('rm -f .cache', shell=True)
        vw = Popen(vw_train_cmd, shell=True, stdin=PIPE)

        print '---------- Training on days %d to %d ----------------' % (day-21, day-1) 
        print

        while int(line.split()[0]) < ts_beginning_test:
            line = line[:-1].split('\t')

            label = -1
            if line[1]:
                conv_ts = int(line[1])
                if conv_ts < ts_beginning_test: 
                    label = 1 # Positive label iff conversion and the conversion occured before the test period

            out = '%d | %s' % (label, get_features(line))
            print >>vw.stdin, out
            line = f.readline()

        vw.stdin.close()
        vw.wait()

        print
        print '---------- Testing on day %d ----------------' % (day-21)
 
        vw = Popen(vw_test_cmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=open(devnull, 'w'))
        ll = 0
        n = 0

        # Test is one day long
        while int(line.split()[0]) < ts_beginning_test + 86400:
            line = line[:-1].split('\t')

            print >>vw.stdin, '| '+get_features(line)
            dotproduct = float(vw.stdout.readline())
    
            # Test log likelihood
            if line[1]: # Positive example
                ll += log(1+exp(-dotproduct))
            else: # Negative sample
                ll += log(1+exp(dotproduct))
            n += 1

            line = f.readline()

        return (ll, n)

def main():
    ll = 0
    n = 0
    # Iterating over the 7 test days
    for day in range(54,61):
        ll_day, n_day = train_test_oneday(day)
        ll += ll_day
        n += n_day
        print ll_day, n_day
    print
    print 'Average test log likelihood: %f' % (ll/n)
    
if __name__ == "__main__":
    main()
