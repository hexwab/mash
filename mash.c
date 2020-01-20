// MASH Simulation
// http://www.aholme.co.uk/Frac2/Mash.htm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
//#include <minmax.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#undef NDEBUG
#include <assert.h>
#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)<(y))?(x):(y))

#ifndef CONFIG /* defines passed externally */
#define ORDER 3 /* 2 or 3 */
#define TWOCH false /* include reference signal for comparison */
#define BINARY true /* binary weighted output; mutually-exclusive with TWOCH */
#define NPLICATE false /* single bitstream output, mutually-exclusive with BINARY */
#define COUNT 1e6 /* number of samples to generate */
#endif

/* input function, 32-bit unsigned */
#define FUNC(i) ((uint32_t) ((sin(i*2*M_PI/100.)+1.)*(2147483647))) /* sine */
//#define FUNC(i) ((uint32_t) ((((uint32_t)(i/5000.))&1)?0x0:0x7fffffff)) /* square */
//#define FUNC(i) ((uint32_t) 1e9) /* constant DC */

uint64_t freq[8] = {0};

#define MODULO (1<<24)
struct mash {
    int q[4];       // Latch outputs
    int c[4][4];    // Carry flip flops
};

int8_t mash_clock(struct mash *m, uint32_t in) {
	int d[4]; // Adder output / latch data input
#define c m->c
#define q m->q
	
        // Carry flip-flops
        c[3][3] = c[3][2]; c[3][2] = c[3][1]; c[3][1] = c[3][0];
	c[2][2] = c[2][1]; c[2][1] = c[2][0];
	c[1][1] = c[1][0];

        // Adders
        d[0] = (q[0]+  in) % MODULO;
        d[1] = (q[1]+d[0]) % MODULO;
        d[2] = (q[2]+d[1]) % MODULO;
        d[3] = (q[3]+d[2]) % MODULO;

        // Carries
        c[0][0] = (q[0]+  in) / MODULO;
        c[1][0] = (q[1]+d[0]) / MODULO;
        c[2][0] = (q[2]+d[1]) / MODULO;
        c[3][0] = (q[3]+d[2]) / MODULO;

        // Latch
        for (int i=0; i<4; i++) q[i] = d[i];

	// 4: +8 to -7 = 16
	// 3: +4 to -3 = 8
	// 2: +2 to -1 = 4
        return  c[0][0]
#if ORDER>=2
	  +   c[1][0] -   c[1][1]
#endif
#if ORDER>=3
	  +   c[2][0] - 2*c[2][1] +   c[2][2]
#endif
#if ORDER>=4
	  +   c[3][0] - 3*c[3][1] + 3*c[3][2] - c[3][3]
#endif
	  ;
#undef c
#undef q
}

void mash_init(struct mash *m) {
        // Initial values
        memset(m->c, 0, sizeof m->c);
        m->q[0] = 1;
        m->q[1] = 0;
        m->q[2] = 0;
        m->q[3] = 0;

        // Fill flip-flops
        for (int i=0; i<4; i++) mash_clock(m, 0);
}

#if 1
const bool bittable7[8][7] = {
	{ 0,0,0,0,0,0,0 },
	{ 0,0,0,1,0,0,0 },
	{ 0,0,1,0,0,1,0 },
	{ 0,1,0,1,0,1,0 },
	{ 1,0,1,0,1,0,1 },
	{ 1,1,0,1,1,0,1 },
	{ 1,1,1,0,1,1,1 },
	{ 1,1,1,1,1,1,1 }
};
#else
const bool bittable7[8][7] = {
	{ 0,0,0,0,0,0,0 },
	{ 0,0,0,1,0,0,0 },
	{ 0,0,0,1,1,0,0 },
	{ 0,0,1,1,1,0,0 },
	{ 0,0,1,1,1,1,0 },
	{ 0,1,1,1,1,1,0 },
	{ 0,1,1,1,1,1,1 },
	{ 1,1,1,1,1,1,1 }
};
#endif
#if 0
const bool bittable3[4][3] = {
	{ 0,0,0 },
	{ 0,1,0 },
	{ 1,0,1 },
	{ 1,1,1 }
};
#else
const bool bittable3[4][3][3] = {
	{ { 0,0,0 }, { 0,0,0 }, { 0,0,0 } },
	{ { 1,0,0 }, { 0,1,0 }, { 0,0,1 } },
	{ { 1,1,0 }, { 1,0,1 }, { 0,1,1 } },
	{ { 1,1,1 }, { 1,1,1 }, { 1,1,1 } },
};
#endif
int main(int argc, char *argv[])
{
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	struct mash m;
	uint64_t i;
	mash_init(&m);
	float cal=1;
	char inbuf[256];
	uint64_t avg=0;
#if BINARY
#define MAXDEC 1
#else
#if ORDER==2
#define MAXDEC 3
#else
#if ORDER==3
#define MAXDEC 7
#else
#error bad ORDER
#endif
#endif
#endif
	while (1) {
		uint32_t in, out;
		int q;
		for (int dec=0; dec<MAXDEC; dec++) {
			++i;
			in = FUNC(i);
			//2
			//out = (mash_clock(&m, in>>8)+1);//*(0x55555555);
			//if (((out&1)) && (((double)rand()/RAND_MAX)>cal)) out^=1;
			//out = (out&1)*0x40000000 *1.02 + (out&2)*0x40000000;
			//3
			//out = (mash_clock(&m, in>>8)+3)*(0x24924924);
			if (!dec) {
				q = 1;//rand()%3;
				out = (mash_clock(&m, in>>8)+3);
				freq[out]++;
			}
#if TWOCH
			// ch1: reference signal
			// 32-bit unsigned
			putchar (in >> 0);
			putchar (in >> 8);
			putchar (in >> 16);
			putchar (in >> 24);
#endif			
			// ch2: output
#if BINARY
			// ORDER bytes of weighting coefficients
			putchar ((out&1)?255:0);
	#if ORDER>=2
			putchar ((out&2)?255:0);
	#endif
	#if ORDER>=3
			putchar ((out&4)?255:0);
	#endif
#else
#if NPLICATE
	#if ORDER==2
			bool bit = bittable3[out][q][dec];
	#endif
	#if ORDER==3
			bool bit = bittable7[out][dec];
	#endif
			// 32-bit unsigned bitstream
			putchar (bit*255);
			putchar (bit*255);
			putchar (bit*255);
			putchar (bit*255);
#else
			// 32-bit unsigned weighted output
			// FIXME: scale this
			putchar (out >> 0);
			putchar (out >> 8);
			putchar (out >> 16);
			putchar (out >> 24);
#endif
#endif
			avg += out;
			if (!(i % 100000)) {
				fprintf(stderr,"avg=%f\n", ((double)avg)/i);
				for (int j=0; j<8;j++) {
					fprintf(stderr,"%d:%llu ", j, freq[j]);
				}
				fprintf(stderr,"n");
			}
			if (read(0,inbuf,sizeof inbuf)>=0) {
				cal-=0.01;
				i=0;avg=0;
				fprintf(stderr,"cal=%f\n", cal);
			}
		}
		if (i >= COUNT)
		  break;
	}
    return 0;
}
