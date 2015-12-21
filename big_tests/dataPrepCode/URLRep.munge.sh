#! /bin/bash

gunzip -c $1 \
| tail -n +2 \
| $mungeCodeDir/svml2vw.pl
