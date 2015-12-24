#! /bin/bash

gunzip -c $1 \
| tail -n +2 \
| $testCodeDir/svml2vw.pl
