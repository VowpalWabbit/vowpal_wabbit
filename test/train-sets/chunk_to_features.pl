#!/usr/bin/perl -w
use strict;

my %cdict = (); my $cdictNum = 1;
while (1) {
    my $cdictFile = shift or last;
    open F, $cdictFile or die;
    while (<F>) {
        chomp;
        my ($c, $num) = split;
        $cdict{$c} = $num;
        if ($num+1 > $cdictNum) { $cdictNum = $num + 1; }
    }
    close F or die;
}

my @w = (); my @t = (); my @c = ();
while (<>) {
    chomp;
    if (/^[\s]*$/) { dumpit(); print "\n"; @w = (); @t = (); @c = (); next; }

    my ($w,$t,$c) = split;
    #if ($c =~ /-NP/) { push @c, "1"; } else { push @c, "-1"; }
    if (not exists $cdict{$c}) { 
        $cdict{$c} = $cdictNum;
        $cdictNum++;
        print STDERR "$c\t$cdict{$c}\n";
    }

    push @c, $cdict{$c};
    push @t, $t;
    push @w, $w;
}

sub dumpit {
    for (my $n=0; $n<@c; $n++) {
        my %f = ();
        for (my $m=-2; $m<=+2; $m++) {
            computef(\%f, '_'.$m, $n+$m);
        }
        print $c[$n] . ' |';
        foreach my $f (keys %f) { 
            $f =~ s/:/-COL-/g;
            $f =~ s/\|/-PIP-/g;
            print ' ' . $f; 
        }
        print "\n";
    }
}

sub computef {
    my ($f, $s0, $i) = @_;

    if ($i <   0) { $f->{"w".$s0."=<s>" } = 1; return; }
    if ($i >= @c) { $f->{"w".$s0."=</s>"} = 1; return; }

    my $w = $w[$i]; my $p = $t[$i]; my $l = lc($w[$i]);

    $f->{"w".$s0."=".$w} = 1;
#    $f->"p:=".$p} = 1;
    $f->{"l".$s0."=".$l} = 1;

    my $c = $w;
    $c =~ s/[A-Z]+/A/g;
    $c =~ s/[a-z]+/a/g;
    $c =~ s/[0-9]+/0/g;
    $c =~ s/[^\.Aa0]+/\#/g;
    $f->{"c".$s0."=".$c} = 1;
    $f->{"c".$s0."=".$c."_fw=".(($i==0) ? "y" : "n")} = 1;

    my $N = length($l);
    $f->{"pre1".$s0."=".substr($l,0,1)} = 1;
    $f->{"pre2".$s0."=".substr($l,0,2)} = 1;
    $f->{"pre3".$s0."=".substr($l,0,3)} = 1;
    $f->{"suf1".$s0."=".substr($l,$N-1,1)} = 1;
    $f->{"suf2".$s0."=".substr($l,$N-2,2)} = 1;
    $f->{"suf3".$s0."=".substr($l,$N-3,3)} = 1;
}
