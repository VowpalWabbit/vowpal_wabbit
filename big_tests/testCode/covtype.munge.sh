#! /bin/bash

gunzip -c $1 \
| perl -pe 's/(.*),(.*)/$2 | $1/; s/,/ /g;' 
