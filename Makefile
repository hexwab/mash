mash: mash.c
	gcc -g -Wall mash.c -o $@ -lm

mash.3bit.binary: mash.c
	gcc -DCONFIG -DORDER=3 -DBINARY=true -DCOUNT=1e6 -g -Wall mash.c -o $@ -lm

pwl3bitweighted: mash.3bit.binary
	./mash.3bit.binary >/tmp/mash.out
	perl split3bit.pl /tmp/mash.out
	perl bittopwl.pl 0 /tmp/out1 >/tmp/out1.pwl
	perl bittopwl.pl 0 /tmp/out2 >/tmp/out2.pwl
	perl bittopwl.pl 0 /tmp/out3 >/tmp/out3.pwl
