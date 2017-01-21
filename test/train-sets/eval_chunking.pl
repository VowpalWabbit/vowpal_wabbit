#!/usr/bin/perl -w
use strict;

my $rdictFile = shift or die;
my $truthFile = shift or die;

my %rdict = (); my $rdictNum = 1;
open F, $rdictFile or die;
while (<F>) {
    chomp;
    my ($c, $num) = split;
    if ($c =~ /^[BI]-O$/) { $c = 'O'; }
    $rdict{$num} = $c;
    if ($num+1 > $rdictNum) { $rdictNum = $num + 1; }
}
close F or die;

my $np = 0;
my $nt = 0;
my $ni = 0;
my $nil = 0;

my $nc = 0;
my $ncl = 0;
my $na = 0;

my @truth = ();
if ($truthFile =~ /.gz$/ ) { open T, "gunzip -c $truthFile |" or die; }
elsif ($truthFile =~ /.bz2$/) { open T, "bzcat $truthFile |" or die; }
else { open T, $truthFile or die; }
while (<T>) {
    chomp;
    if (/^[\s]*$/) { runit(); @truth = (); next; }
    my ($c) = split;
    if (not defined $rdict{$c}) { die $c; }
    push @truth, $rdict{$c};
}
close T;

my $p  = $ni  / (($np > 0) ? $np : 1);
my $r  = $ni  / (($nt > 0) ? $nt : 1);
my $f  = 2 * $p * $r / ($p + $r);
my $a  = $nc  / (($na > 0) ? $na : 1);
my $pl = $nil / (($np > 0) ? $np : 1);
my $rl = $nil / (($nt > 0) ? $nt : 1);
my $fl = 2 * $pl * $rl / ($pl + $rl);
my $al = $ncl / (($na > 0) ? $na : 1);

$p  = int($p  * 1000)/10; $r  = int($r  * 1000)/10; $f  = int($f  * 1000)/10; $a  = int ($a  * 1000)/10;
$pl = int($pl * 1000)/10; $rl = int($rl * 1000)/10; $fl = int($fl * 1000)/10; $al = int ($al * 1000)/10;

print "unlabeled: p=$p\tr=$r\tf=$f\tacc=$a\n";
print "  labeled: p=$pl\tr=$rl\tf=$fl\tacc=$al\n";


sub runit {
    my $N = scalar @truth;
    my @pred = ();
    for (my $n=0; $n<$N; $n++) {
        $_ = <>;
        chomp;
        $_ = int($_);
        if (not defined $rdict{$_}) { die $_; }
        push @pred, $rdict{$_};
    }
    $_ = <>; chomp;
    if (not /^\s*$/) { die; }

    $na += $N;
    for (my $n=0; $n<$N; $n++) {
        if ($pred[$n] eq $truth[$n]) { $ncl++; }
        if (substr($pred[$n],0,1) eq substr($truth[$n],0,1)) { $nc++; }
    }

    my %c1 = chunksof(@truth);
    my %c2 = chunksof(@pred);

    $np += scalar keys %c1;
    $nt += scalar keys %c2;
    foreach my $c (keys %c1) {
        if (exists $c2{$c}) { 
            $ni++;
            if ($c2{$c} eq $c1{$c}) {
                $nil++;
            }
        }
    }
}

sub chunksof {
    my @l = @_;
    my $i = 0;
    my %c = ();
    while ($i < @l) {
        if ($l[$i] =~ /^B-(.+)$/) {
            my $lab = $1;
            if ($lab eq 'O') { $i++; next; }
            my $j = $i+1;
            while ($j < @l) {
                if ($l[$j] eq "I-$lab") { $j++; }
                else { last; }
            }
            $c{"$i $j"} = $lab;
            $i = $j;
        } else {
            $i++;
        }
    }
    return (%c);
}
