#!/usr/bin/perl -w
open A, ">/tmp/out1" or die $!;
open B, ">/tmp/out2" or die $!;
$/=\2;
while ($q=<>) {
    print A substr($q,0,1);
    print B substr($q,1,1);
}
