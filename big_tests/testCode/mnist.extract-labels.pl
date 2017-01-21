#! /usr/bin/env perl

use warnings;
use strict;

use IO::File;

local $SIG{__WARN__} = sub {};

#my $fname = shift @ARGV or die;
#my $fh = new IO::File $fname, "r" or die "$fname: $!";
my $fh = \*STDIN;
binmode ($fh, ':raw');

$/ = \4;
my $magic = unpack ("N", <$fh>);
die "wtf $magic" unless $magic == 2049;

my $n_items = unpack ("N", <$fh>);
warn "n_items = $n_items";

$/ = \1;

while (defined ($_ = <$fh>))
  {
    die "wtf @{[length($_)]}" unless length ($_) == 1;
    my ($label) = unpack ("C", $_);

    ++$label;

    print "$label 1 $label|\n";
  }
