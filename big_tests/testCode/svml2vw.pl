#!/usr/bin/env perl

# convert data format from SVM-Light to VW

while (<>) {
    s/^\-1/0/;
    s/^\+1/1/;
    s/ / | /;
    print;
}
