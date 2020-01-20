#!/usr/bin/perl -w
open A, ">/tmp/out1" or die $!;
open B, ">/tmp/out2" or die $!;
open C, ">/tmp/out3" or die $!;
$/=\3;
while ($q=<>) {
    print A substr($q,0,1);
    print B substr($q,1,1);
    print C substr($q,2,1);
}
