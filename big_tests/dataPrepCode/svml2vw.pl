#!/bin/perl -ap

# convert data format from SVM-Light to VW
s/^\-1/0/;
s/^\+1/1/;
s/ / | /;
