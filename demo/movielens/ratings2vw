#! /usr/bin/env perl

use IO::File;

use warnings;
use strict;

srand 69;

sub output_user ($$@)
{
  my ($trainfh, $testfh, @rows) = @_;

  return () unless @rows > 1;

  my @permrows = map { $_->[1] }
                 sort { $a->[0] <=> $b->[0] }
                 map { [ rand (), $_ ] }
                 @rows;

  my @testrows = splice @permrows, -1;

  print $trainfh @permrows;
  return @testrows;
}

my $trainfile = shift @ARGV or die;
my $testfile = shift @ARGV or die;

my $trainfh = new IO::File $trainfile, "w" or die;
my $testfh = new IO::File $testfile, "w" or die;

my $olduser;
my @rows;
my @save;

my %seen;

while (defined ($_ = <>))
  {
    chomp;
    my ($user, $movie, $rating, undef) = split /::/, $_;

    if (defined ($olduser) && $user != $olduser)
      {
        push @save, output_user ($trainfh, $testfh, @rows);
        undef @rows;

        die "input file not collated" if $seen{$olduser}++;
      }

    push @rows, "$rating $rating|user $user |movie $movie\n";
    $olduser = $user;
  }

push @save, output_user ($trainfh, $testfh, @rows);

my @permsave = map { $_->[1] }
               sort { $a->[0] <=> $b->[0] }
               map { [ rand (), $_ ] }
               @save;

my @test = splice @permsave, 0, 5000;

print $trainfh @permsave;
print $testfh @test;
