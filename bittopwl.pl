#!/usr/bin/perl -w
use Math::Random::OO::Normal;
my $r=Math::Random::OO::Normal->new(1,1);  # mean 1, stdev 1

sub line {
    my ($t,$b)=@_;
    printf "%.13f %.3f\n", $t, $b?$vmax:$vmin;
}
$freq=5e8; # 500MHz
$period=1/$freq;
$rise=100e-12; # 100ps
#$vmin=1.025;
#$vmax=1.375;
$vmin=0;
$vmax=3.3;
$t=0;
$/=\1;
$that=0;
$count=100000;
$inverted=shift;
($vmin,$vmax)=($vmax,$vmin) if $inverted;
while ($q=<>) {
    $this=!!unpack"C",$q;
    if ($this!=$that) {
	line($t+$rise*$r->next, $this);
    }
    $t+=$period;
    line($t*$r->next, $this);
    $that=$this;
    exit if !--$count;
}
