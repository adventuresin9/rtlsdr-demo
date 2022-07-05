/*
 * rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
 * Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
 * Copyright (C) 2012 by Hoernchen <la@tfc-server.de>
 * Copyright (C) 2012 by Kyle Keen <keenerd@gmail.com>
 * Copyright (C) 2013 by Elias Oenal <EliasOenal@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * modified to work with 9Front 
 * by adventuresin9@gmail.com
 * May 2022
 *
 * the code to demodulate the FM radio was lifted from
 * rtl_fm.c
 */




#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "../lib/usb.h"
#include "dat.h"
#include "fns.h"

#define round(x) (x > 0.0 ? floor(x + 0.5): ceil(x - 0.5))

//#define DEFAULT_SAMPLE_RATE		24000
//#define DEFAULT_BUF_LENGTH		(1 * 16384)
//#define MAXIMUM_OVERSAMPLE		16
//#define MAXIMUM_BUF_LENGTH		(MAXIMUM_OVERSAMPLE * DEFAULT_BUF_LENGTH)
//#define AUTO_GAIN			-100
//#define BUFFER_DUMP			4096

//#define FREQUENCIES_LIMIT		1000

static volatile int do_exit = 0;
static int lcm_post[17] = {1,1,1,3,1,5,3,7,1,9,5,11,3,13,7,15,1};
static int ACTUAL_BUF_LENGTH;

static int *atan_lut = NULL;
static int atan_lut_size = 131072; /* 512 KB */
static int atan_lut_coef = 8;


// multiple of these, eventually
dongle_state dongle;
demod_state demod;
output_state output;
controller_state controller;


/* {length, coef, coef, coef}  and scaled by 2^15
   for now, only length 9, optimal way to get +85% bandwidth */
#define CIC_TABLE_MAX 10
int cic_9_tables[][10] = {
	{0,},
	{9, -156,  -97, 2798, -15489, 61019, -15489, 2798,  -97, -156},
	{9, -128, -568, 5593, -24125, 74126, -24125, 5593, -568, -128},
	{9, -129, -639, 6187, -26281, 77511, -26281, 6187, -639, -129},
	{9, -122, -612, 6082, -26353, 77818, -26353, 6082, -612, -122},
	{9, -120, -602, 6015, -26269, 77757, -26269, 6015, -602, -120},
	{9, -120, -582, 5951, -26128, 77542, -26128, 5951, -582, -120},
	{9, -119, -580, 5931, -26094, 77505, -26094, 5931, -580, -119},
	{9, -119, -578, 5921, -26077, 77484, -26077, 5921, -578, -119},
	{9, -119, -577, 5917, -26067, 77473, -26067, 5917, -577, -119},
	{9, -199, -362, 5303, -25505, 77489, -25505, 5303, -362, -199},
};


double log2(double n)
{
	return log(n) / log(2.0);
}


void rotate_90(unsigned char *buf, u32int len)
/* 90 rotation is 1+0j, 0+1j, -1+0j, 0-1j
   or [0, 1, -3, 2, -4, -5, 7, -6] */
{
	u32int i;
	unsigned char tmp;
	for (i=0; i<len; i+=8) {
		/* uint8_t negation = 255 - x */
		tmp = 255 - buf[i+3];
		buf[i+3] = buf[i+2];
		buf[i+2] = tmp;

		buf[i+4] = 255 - buf[i+4];
		buf[i+5] = 255 - buf[i+5];

		tmp = 255 - buf[i+6];
		buf[i+6] = buf[i+7];
		buf[i+7] = tmp;
	}
}


void low_pass(struct demod_state *d)
/* simple square window FIR */
{
	int i=0, i2=0;
	while (i < d->lp_len) {
		d->now_r += d->lowpassed[i];
		d->now_j += d->lowpassed[i+1];
		i += 2;
		d->prev_index++;
		if (d->prev_index < d->downsample) {
			continue;
		}
		d->lowpassed[i2]   = d->now_r; // * d->output_scale;
		d->lowpassed[i2+1] = d->now_j; // * d->output_scale;
		d->prev_index = 0;
		d->now_r = 0;
		d->now_j = 0;
		i2 += 2;
	}
	d->lp_len = i2;
}


int low_pass_simple(s16int *signal2, int len, int step)
// no wrap around, length must be multiple of step
{
	int i, i2, sum;
	for(i=0; i < len; i+=step) {
		sum = 0;
		for(i2=0; i2<step; i2++) {
			sum += (int)signal2[i + i2];
		}
		//signal2[i/step] = (s16int)(sum / step);
		signal2[i/step] = (s16int)(sum);
	}
	signal2[i/step + 1] = signal2[i/step];
	return len / step;
}


void low_pass_real(struct demod_state *s)
/* simple square window FIR */
// add support for upsampling?
{
	int i=0, i2=0;
	int fast = (int)s->rate_out;
	int slow = s->rate_out2;
	while (i < s->result_len) {
		s->now_lpr += s->result[i];
		i++;
		s->prev_lpr_index += slow;
		if (s->prev_lpr_index < fast) {
			continue;
		}
		s->result[i2] = (s16int)(s->now_lpr / (fast/slow));
		s->prev_lpr_index -= fast;
		s->now_lpr = 0;
		i2 += 1;
	}
	s->result_len = i2;
}


void fifth_order(s16int *data, int length, s16int *hist)
/* for half of interleaved data */
{
	int i;
	s16int a, b, c, d, e, f;
	a = hist[1];
	b = hist[2];
	c = hist[3];
	d = hist[4];
	e = hist[5];
	f = data[0];
	/* a downsample should improve resolution, so don't fully shift */
	data[0] = (a + (b+e)*5 + (c+d)*10 + f) >> 4;
	for (i=4; i<length; i+=4) {
		a = c;
		b = d;
		c = e;
		d = f;
		e = data[i-2];
		f = data[i];
		data[i/2] = (a + (b+e)*5 + (c+d)*10 + f) >> 4;
	}
	/* archive */
	hist[0] = a;
	hist[1] = b;
	hist[2] = c;
	hist[3] = d;
	hist[4] = e;
	hist[5] = f;
}


void generic_fir(s16int *data, int length, int *fir, s16int *hist)
/* Okay, not at all generic.  Assumes length 9, fix that eventually. */
{
	int d, temp, sum;
	for (d=0; d<length; d+=2) {
		temp = data[d];
		sum = 0;
		sum += (hist[0] + hist[8]) * fir[1];
		sum += (hist[1] + hist[7]) * fir[2];
		sum += (hist[2] + hist[6]) * fir[3];
		sum += (hist[3] + hist[5]) * fir[4];
		sum +=            hist[4]  * fir[5];
		data[d] = sum >> 15 ;
		hist[0] = hist[1];
		hist[1] = hist[2];
		hist[2] = hist[3];
		hist[3] = hist[4];
		hist[4] = hist[5];
		hist[5] = hist[6];
		hist[6] = hist[7];
		hist[7] = hist[8];
		hist[8] = temp;
	}
}


/* define our own complex math ops
   because ARMv5 has no hardware float */

void multiply(int ar, int aj, int br, int bj, int *cr, int *cj)
{
	*cr = ar*br - aj*bj;
	*cj = aj*br + ar*bj;
}

int polar_discriminant(int ar, int aj, int br, int bj)
{
	int cr, cj;
	double angle;
	multiply(ar, aj, br, -bj, &cr, &cj);
	angle = atan2((double)cj, (double)cr);
	return (int)(angle / 3.14159 * (1<<14));
}


int fast_atan2(int y, int x)
/* pre scaled for int16 */
{
	int yabs, angle;
	int pi4=(1<<12), pi34=3*(1<<12);  // note pi = 1<<14
	if (x==0 && y==0) {
		return 0;
	}
	yabs = y;
	if (yabs < 0) {
		yabs = -yabs;
	}
	if (x >= 0) {
		angle = pi4  - pi4 * (x-yabs) / (x+yabs);
	} else {
		angle = pi34 - pi4 * (x+yabs) / (yabs-x);
	}
	if (y < 0) {
		return -angle;
	}
	return angle;
}

int polar_disc_fast(int ar, int aj, int br, int bj)
{
	int cr, cj;
	multiply(ar, aj, br, -bj, &cr, &cj);
	return fast_atan2(cj, cr);
}

int atan_lut_init(void)
{
	int i = 0;

	atan_lut = malloc(atan_lut_size * sizeof(int));

	for (i = 0; i < atan_lut_size; i++) {
		atan_lut[i] = (int) (atan((double) i / (1<<atan_lut_coef)) / 3.14159 * (1<<14));
	}

	return 0;
}

int polar_disc_lut(int ar, int aj, int br, int bj)
{
	int cr, cj, x, x_abs;

	multiply(ar, aj, br, -bj, &cr, &cj);

	/* special cases */
	if (cr == 0 || cj == 0) {
		if (cr == 0 && cj == 0)
			{return 0;}
		if (cr == 0 && cj > 0)
			{return 1 << 13;}
		if (cr == 0 && cj < 0)
			{return -(1 << 13);}
		if (cj == 0 && cr > 0)
			{return 0;}
		if (cj == 0 && cr < 0)
			{return 1 << 14;}
	}

	/* real range -32768 - 32768 use 64x range -> absolute maximum: 2097152 */
	x = (cj << atan_lut_coef) / cr;
	x_abs = abs(x);

	if (x_abs >= atan_lut_size) {
		/* we can use linear range, but it is not necessary */
		return (cj > 0) ? 1<<13 : -(1<<13);
	}

	if (x > 0) {
		return (cj > 0) ? atan_lut[x] : atan_lut[x] - (1<<14);
	} else {
		return (cj > 0) ? (1<<14) - atan_lut[-x] : -atan_lut[-x];
	}

	return 0;
}



/* done with the math */

void fm_demod(struct demod_state *fm)
{
	int i, pcm;
	s16int *lp = fm->lowpassed;
	pcm = polar_discriminant(lp[0], lp[1],
		fm->pre_r, fm->pre_j);
	fm->result[0] = (s16int)pcm;
	for (i = 2; i < (fm->lp_len-1); i += 2) {
		switch (fm->custom_atan) {
		case 0:
			pcm = polar_discriminant(lp[i], lp[i+1],
				lp[i-2], lp[i-1]);
			break;
		case 1:
			pcm = polar_disc_fast(lp[i], lp[i+1],
				lp[i-2], lp[i-1]);
			break;
		case 2:
			pcm = polar_disc_lut(lp[i], lp[i+1],
				lp[i-2], lp[i-1]);
			break;
		}
		fm->result[i/2] = (s16int)pcm;
	}
	fm->pre_r = lp[fm->lp_len - 2];
	fm->pre_j = lp[fm->lp_len - 1];
	fm->result_len = fm->lp_len/2;
}


void deemph_filter(struct demod_state *fm)
{
	static int avg;  // cheating...
	int i, d;
	// de-emph IIR
	// avg = avg * (1 - alpha) + sample * alpha;
	for (i = 0; i < fm->result_len; i++) {
		d = fm->result[i] - avg;
		if (d > 0) {
			avg += (d + fm->deemph_a/2) / fm->deemph_a;
		} else {
			avg += (d - fm->deemph_a/2) / fm->deemph_a;
		}
		fm->result[i] = (s16int)avg;
	}
}


int rms(s16int *samples, int len, int step)
/* largely lifted from rtl_power */
{
	int i;
	long p, t, s;
	double dc, err;

	p = t = 0L;
	for (i=0; i<len; i+=step) {
		s = (long)samples[i];
		t += s;
		p += s * s;
	}
	/* correct for dc offset in squares */
	dc = (double)(t*step) / (double)len;
	err = t * 2 * dc - dc * dc * len;

	return (int)sqrt((p-err) / len);
}


void dc_block_filter(struct demod_state *fm)
{
	int i, avg;
	s64int sum = 0;
	for (i=0; i < fm->result_len; i++) {
		sum += fm->result[i];
	}
	avg = sum / fm->result_len;
	avg = (avg + fm->dc_avg * 9) / 10;
	for (i=0; i < fm->result_len; i++) {
		fm->result[i] -= avg;
	}
	fm->dc_avg = avg;
}


void full_demod(struct demod_state *d)
{
	int i, ds_p;
	int sr = 0;
	ds_p = d->downsample_passes;
	if (ds_p) {
		for (i=0; i < ds_p; i++) {
			fifth_order(d->lowpassed,   (d->lp_len >> i), d->lp_i_hist[i]);
			fifth_order(d->lowpassed+1, (d->lp_len >> i) - 1, d->lp_q_hist[i]);
		}
		d->lp_len = d->lp_len >> ds_p;
		/* droop compensation */
		if (d->comp_fir_size == 9 && ds_p <= CIC_TABLE_MAX) {
			generic_fir(d->lowpassed, d->lp_len,
				cic_9_tables[ds_p], d->droop_i_hist);
			generic_fir(d->lowpassed+1, d->lp_len-1,
				cic_9_tables[ds_p], d->droop_q_hist);
		}
	} else {
		low_pass(d);
	}
	/* power squelch */
	if (d->squelch_level) {
		sr = rms(d->lowpassed, d->lp_len, 1);
		if (sr < d->squelch_level) {
			d->squelch_hits++;
			for (i=0; i<d->lp_len; i++) {
				d->lowpassed[i] = 0;
			}
		} else {
			d->squelch_hits = 0;}
	}
	d->mode_demod(d);  /* lowpassed -> result */
//	if (d->mode_demod == &raw_demod) {
//		return;
//	}

	/* todo, fm noise squelch */
	// use nicer filter here too?
	if (d->post_downsample > 1) {
		d->result_len = low_pass_simple(d->result, d->result_len, d->post_downsample);}
	if (d->deemph) {
		deemph_filter(d);}
	if (d->dc_block) {
		dc_block_filter(d);}
	if (d->rate_out2 > 0) {
		low_pass_real(d);
		//arbitrary_resample(d->result, d->result, d->result_len, d->result_len * d->rate_out2 / d->rate_out);
	}
}


void dongle_init(struct dongle_state *s)
{
	s->rate = DEFAULT_SAMPLE_RATE;
	s->gain = AUTO_GAIN; // tenths of a dB
	s->mute = 0;
	s->direct_sampling = 0;
	s->offset_tuning = 0;
	s->demod_target = &demod;
}


void demod_init(struct demod_state *s)
{
	s->rate_in = DEFAULT_SAMPLE_RATE;
	s->rate_out = DEFAULT_SAMPLE_RATE;
	s->squelch_level = 0;
	s->conseq_squelch = 10;
	s->terminate_on_squelch = 0;
	s->squelch_hits = 11;
	s->downsample_passes = 0;
	s->comp_fir_size = 0;
	s->prev_index = 0;
	s->post_downsample = 1;  // once this works, default = 4
	s->custom_atan = 0;
	s->deemph = 0;
	s->rate_out2 = -1;  // flag for disabled
	s->mode_demod = &fm_demod;
	s->pre_j = s->pre_r = s->now_r = s->now_j = 0;
	s->prev_lpr_index = 0;
	s->deemph_a = 0;
	s->now_lpr = 0;
	s->dc_block = 0;
	s->dc_avg = 0;
//	pthread_rwlock_init(&s->rw, NULL);
//	pthread_cond_init(&s->ready, NULL);
//	pthread_mutex_init(&s->ready_m, NULL);
	s->output_target = &output;
}


void output_init(struct output_state *s)
{
	s->rate = DEFAULT_SAMPLE_RATE;
//	pthread_rwlock_init(&s->rw, NULL);
//	pthread_cond_init(&s->ready, NULL);
//	pthread_mutex_init(&s->ready_m, NULL);
}


void controller_init(struct controller_state *s)
{
//	s->freqs[0] = 100000000;
	s->freqs[0] = 101100000;
	s->freq_len = 0;
	s->edge = 0;
	s->wb_mode = 0;
//	pthread_cond_init(&s->hop, NULL);
//	pthread_mutex_init(&s->hop_m, NULL);
}



/*
 * below replaces the main function for the command line 
 * program to provide a file system interface that is 
 * typical for Plan 9 systems.
 */








void
radioread(Req *r)
{
	char *radioout;
	char *rerror;
	float station;

	char *buf;

	station = atof(r->fid->file->name);
	station *= 1000000;

	controller.freqs[0] = (u32int)station;
	controller.freq_len = 1;

	
	snprint(buf, sizeof(buf), "station is %d\n", controller.freqs[0]);
	readstr(r, buf);
	respond(r, nil);

}


void
radiocreate(Req *r)
{
	File *f;
	int p;
	float station;


	f = r->fid->file;
	p = r->ifcall.perm;

/* don't allow creation of files outside FM radio range */
	station = atof(r->fid->file->name);
	if(station > 108.0 || station < 88.0){
		responderror(r);
		return;
	}

	if((p & DMDIR) != 0)
		p = (p & ~0777) | ((p & f->mode) & 0777);
	else
		p = (p & ~0666) | ((p & f->mode) & 0666);
	if((f = createfile(f, r->ifcall.name, r->fid->uid, p, nil)) == nil){
		responderror(r);
		return;
	}
	f->atime = f->mtime = time(0);
	f->aux = nil;
	r->fid->file = f;
	r->ofcall.qid = f->qid;
	respond(r, nil);
}


void
radiodelete(Fid *fid)
{
	File *f;

	f = fid->file;
	if(fid->omode != -1 && (fid->omode & ORCLOSE) != 0 && f != nil && f->parent != nil)
		removefile(f);
}


void
radioend(Srv*)
{
	rtlsdr_close(dongle.dev);

	postnote(PNGROUP, getpid(), "shutdown");
	threadexitsall(nil);
}


Srv fs = {
	.read = radioread,
//	.write = radiowrite,
	.create = radiocreate,
	.destroyfid = radiodelete,
//	.start = radiostart,
	.end = radioend,
};



/* for debug */



void
threadmain(int argc, char **argv)
{
	int r, i;
	char *epname;
	Dev *plandev;

	usbdebug = 0;

	int enable_biastee = 0;

	epname = argv[1];

	print("trying enpoint %s\n", epname);



	dongle_init(&dongle);
	demod_init(&demod);
	output_init(&output);
	controller_init(&controller);

/* these are the settings set with -M wbfm */
	controller.wb_mode = 1;
	demod.mode_demod = &fm_demod;
	demod.rate_in = 170000;
	demod.rate_out = 170000;
	demod.rate_out2 = 32000;
	demod.custom_atan = 1;
	//demod.post_downsample = 4;
	demod.deemph = 1;
	demod.squelch_level = 0;

/* hardcoding a frequency to tune to */
//	controller.freqs[controller.freq_len] = 91500000;
//	controller.freq_len++;

	/* quadruple sample_rate to limit to Δθ to ±π/2 */
	demod.rate_in *= demod.post_downsample;

	if (!output.rate)
		output.rate = demod.rate_out;

	if (controller.freq_len > 1)
		demod.terminate_on_squelch = 0;


/* changed to work with 9Front */
	r = rtlsdr_open(&dongle.dev, epname);
	if(r < 0)
		sysfatal("failed right away at rtlsdr_open: %r");




	if (demod.deemph) {
		demod.deemph_a = (int)round(1.0/((1.0-exp(-1.0/(demod.rate_out * 75e-6)))));
	}

	rtlsdr_set_tuner_gain_mode(dongle.dev, 0);

/* if adding gain is needed */
//	r = rtlsdr_set_tuner_gain_mode(dongle.dev, 1);
//	r = rtlsdr_set_tuner_gain(dev, gain);

//	rtlsdr_set_bias_tee(dongle.dev, enable_biastee);

	rtlsdr_set_freq_correction(dongle.dev, dongle.ppm_error);

/* hard setting for debug */
//	rtlsdr_set_center_freq(dongle.dev, 100000000);
//	rtlsdr_set_sample_rate(dongle.dev, 250000);


/* Reset endpoint before we start reading from it (mandatory) */
	rtlsdr_reset_buffer(dongle.dev);


//	int testfd;
//	testfd = open("test.iq", OWRITE);
	char testtest[512];
	print("\n reading from bulk endpoint \n");
	int looploop;
	for(looploop = 0; looploop < (32); looploop++){
		read(dongle.dev->readradio->dfd, testtest, sizeof(testtest));
//		write(testfd, testtest, sizeof(testtest));
		print(testtest);
	}
	print("\n done reading bulk \n");

	rtlsdr_close(dongle.dev);
	print("closing up, test done\n");

//	threadpostsharesrv(&fs, nil, "usb", "rtlsdr");

	threadexits(nil);
}