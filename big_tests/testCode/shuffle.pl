#!/usr/bin/env perl

########################################################################
# shuffle input lines
# same as random sample (without replacement) of all of them
########################################################################

srand(13);

while (<>) {
    $line[$. - 1] = $_;
};

$size = $count = $.;

for(;$size > 0; $size-- && $count--) {
	$ind = int rand $count;
	print splice(@line, $ind, 1);
};
