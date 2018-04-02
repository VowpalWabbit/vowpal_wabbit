#!/bin/bash

suffix=".gz"

for filename in ./*.vw.gz; do
	vw_name=$(echo "$filename" | sed -e "s/$suffix$//")
	echo $vw_name
	zcat $filename | shuf > ../vwshuffled/$vw_name
	gzip ../vwshuffled/$vw_name
done
