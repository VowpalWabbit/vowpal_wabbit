#! /usr/bin/env perl

use warnings;
use strict;
use IO::File;

my $freqfile = shift @ARGV or die;
my $numlabels = shift @ARGV or die;
my $shufbufsize = shift @ARGV or die;

my $freqfh = new IO::File $freqfile, "r" or die "$freqfile: $!";

my %dict;

my $curlabels = $numlabels;
while (defined ($_ = <$freqfh>))
  {
    chomp;
    my ($key, undef) = split /\s+/, $_;

    $dict{$key} = $curlabels;
    --$curlabels;
    last unless $curlabels > 0;
  }

srand 69;

my $context = 6;
my @shufbuf;
my @charpos = split //, "abcdefghijklmnop";

$\="\n";
while (defined ($_ = <STDIN>))
  {
    chomp;
    s/\|/_/g; s/:/_/g;                  # VW special characters
    my @tokens = split /\s+/, $_;

    foreach my $pos ($context ... $#tokens)
      {
        my $label = $dict{$tokens[$pos]} || $numlabels+1;

        my $index = int (rand ($shufbufsize));
        print $shufbuf[$index] if length ($shufbuf[$index]);
        $shufbuf[$index] = join " ", $label, map { join "", " |", $charpos[$_], " ", $tokens[$pos - $_] } (1 .. $context);
      }
  }

foreach my $index (0 .. $shufbufsize)
  {
    print $shufbuf[$index] if length ($shufbuf[$index]);
  }
