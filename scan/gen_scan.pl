#!/usr/bin/perl -w

use strict;

my $N = 300;
my $BIG = 1500;

for (my $i=0; $i<$N; ++$i) {
    my $fn = sprintf("scan%03d.b", $i);
    open my $OUTF, ">$fn" or die;
    for (my $j=0; $j<$BIG; ++$j) {
	if ($j != $i) {
	    print $OUTF "+";
	}
	print $OUTF ">";
    }
    for (my $j=0; $j<$BIG; ++$j) {
	print $OUTF "<";
    }
    print $OUTF "[>]";
    close $OUTF;
}
