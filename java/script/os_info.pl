#!/usr/bin/env perl

use strict;
use warnings;

if ($#ARGV != 0) {
  die "invalid arguments, exiting"
}

my $cmd = $ARGV[0];

if ($cmd eq "osfamily") {
  my $f = get_os_family($^O);
  print "$f." . get_os_arch($f);
} else {
  print "unknown command\n";
}

sub get_lsb_release {
  my $opt = shift;
  my $res=(split(':', `lsb_release $opt`))[1];
  $res =~ s/\s+//g;
  return $res;
}

sub get_os_arch {
  my $os_family = shift;
  my $arch = `uname -m`;
  if ($os_family =~ /ubuntu/i) {
    $arch = `dpkg --print-architecture`;
  }
  chomp($arch);
  return $arch;
}

sub get_os_family {
  my $os = shift;
  if ($os =~ /darwin/i) {
    return "Darwin";
  } elsif ($os =~ /linux/i) {
    my $dist = get_lsb_release("-i");
    $dist =~ s/ /_/g;
    my $fullVersion = get_lsb_release("-r");
    my $majorVersion = (split('\.', $fullVersion))[0];
    return "$dist.$majorVersion";
  } else {
    return "unknown";
  }
}
