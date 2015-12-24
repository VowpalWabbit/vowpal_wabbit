#!/usr/bin/env perl

####################################################################################################
# Author:  I. Dan Melamed
# Purpose:	for each pair of floating point numbers (x,y), and +ive tolerance t, check if (x / y - 1 < t)
# Streams:	2 files of numbers; the numbers that we want to be smaller should come first
# N.B.:		tolerance is evaluated in only one direction, unless one of the numbers is zero
####################################################################################################

#check for correct usage
if ($#ARGV < 0) {
    print "usage: $0 <tolerance> <file 1> [<file 2>]\n";
    exit; 
};

$tolerance = shift;
open(F, $ARGV[0]) || die "\nCouldn't open $ARGV[0]: $!\n";
shift;
open(G, $ARGV[0]) || die "\nCouldn't open $ARGV[0]: $!\n";
shift;

LINE: while (<F>) {
	@ftok = split;
	if (eof(G)) {
		print "1st file has more lines than 2nd.\n";
		last;
	};
	$_ = <G>;
	@gtok = split;
	while (@ftok) {
		if (! @gtok) {
			print "Different number of tokens on line $.\n";
			next LINE;
		};
		$ftok = shift @ftok;
		$gtok = shift @gtok;
		if ($ftok == 0 && $gtok == 0) {
			next;
		};
		if (
			($ftok != 0 && $gtok == 0)
			|| 
			($ftok == 0 && $gtok != 0)
			) {
			print "Difference in zeros on line $.: $ftok vs. $gtok .\n";
			next LINE;
		};
		$diff = $ftok / $gtok - 1.0;
		if ($diff > $tolerance) {
			print "Difference exceeds tolerance on line $.: $ftok vs. $gtok .\n";
			next LINE;
		};
	};
	if (@gtok) {
		print "Different number of tokens on line $.\n";
		next LINE;
	};
};

if (not eof(G)) {
	print "2nd file has more lines than 1st.\n";
};
