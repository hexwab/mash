#!/usr/bin/perl -w
use Math::Random::OO::Normal;
my $r=Math::Random::OO::Normal->new(1,0.02);  # mean, stdev

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
$t=0; $tt=0;
$/=\1;
$that=0;
$count=100000;
$inverted=shift;
($vmin,$vmax)=($vmax,$vmin) if $inverted;
while ($q=<>) {
    $this=!!unpack"C",$q;
    if ($this!=$that) {
	line($tt+$rise, $this);
    }
    $tt=$t+$period*$r->next; # jittered
    $t+=$period;
    line($tt, $this);
    $that=$this;
    exit if !--$count;
}
