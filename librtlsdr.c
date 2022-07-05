/*
 * rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
 * Copyright (C) 2012-2014 by Steve Markgraf <steve@steve-m.de>
 * Copyright (C) 2012 by Dimitri Stolnikov <horiz0n@gmx.net>
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
 */






#include <u.h>
#include <libc.h>
#include <thread.h>
#include "../lib/usb.h"
#include "dat.h"
#include "fns.h"


/* generic tuner interface functions, shall be moved to the tuner implementations */
int
e4000_init(void *dev) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	devt->e4k_s.i2c_addr = E4K_I2C_ADDR;
	rtlsdr_get_xtal_freq(devt, NULL, &devt->e4k_s.vco.fosc);
	devt->e4k_s.rtl_dev = dev;
	return e4k_init(&devt->e4k_s);
}
int
e4000_exit(void *dev) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return e4k_standby(&devt->e4k_s, 1);
}
int
e4000_set_freq(void *dev, u32int freq) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return e4k_tune_freq(&devt->e4k_s, freq);
}

int
e4000_set_bw(void *dev, int bw) {
	int r = 0;
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;

	r |= e4k_if_filter_bw_set(&devt->e4k_s, E4K_IF_FILTER_MIX, bw);
	r |= e4k_if_filter_bw_set(&devt->e4k_s, E4K_IF_FILTER_RC, bw);
	r |= e4k_if_filter_bw_set(&devt->e4k_s, E4K_IF_FILTER_CHAN, bw);

	return r;
}

int
e4000_set_gain(void *dev, int gain) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	int mixgain = (gain > 340) ? 12 : 4;


	int enhgain = (gain - 420);


	if(e4k_set_lna_gain(&devt->e4k_s, min(300, gain - mixgain * 10)) == -1)
		return -1;
	if(e4k_mixer_gain_set(&devt->e4k_s, mixgain) == -1)
		return -1;

 /* enhanced mixer gain seems to have no effect */
	if(enhgain >= 0)
		if(e4k_set_enh_gain(&devt->e4k_s, enhgain) == -1)
			return -1;


	return 0;
}

int
e4000_set_if_gain(void *dev, int stage, int gain) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return e4k_if_gain_set(&devt->e4k_s, (u8int)stage, (s8int)(gain / 10));
}

int
e4000_set_gain_mode(void *dev, int manual) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return e4k_enable_manual_gain(&devt->e4k_s, manual);
}

int _fc0012_init(void *dev) { return fc0012_init(dev); }

int fc0012_exit(void *dev) { return 0; }

int
fc0012_set_freq(void *dev, u32int freq) {
	/* select V-band/U-band filter */
	rtlsdr_set_gpio_bit(dev, 6, (freq > 300000000) ? 1 : 0);
	return fc0012_set_params(dev, freq, 6000000);
}

int fc0012_set_bw(void *dev, int bw) { return 0; }

int _fc0012_set_gain(void *dev, int gain) { return fc0012_set_gain(dev, gain); }

int fc0012_set_gain_mode(void *dev, int manual) { return 0; }

int _fc0013_init(void *dev) { return fc0013_init(dev); }

int fc0013_exit(void *dev) { return 0; }

int
fc0013_set_freq(void *dev, u32int freq) {
	return fc0013_set_params(dev, freq, 6000000);
}

int fc0013_set_bw(void *dev, int bw) { return 0; }

int _fc0013_set_gain(void *dev, int gain) { return fc0013_set_lna_gain(dev, gain); }

int fc2580_init(void *dev) { return fc2580_Initialize(dev); }

int fc2580_exit(void *dev) { return 0; }

int
_fc2580_set_freq(void *dev, u32int freq) {
	return fc2580_SetRfFreqHz(dev, freq);
}

int fc2580_set_bw(void *dev, int bw) { return fc2580_SetBandwidthMode(dev, 1); }

int fc2580_set_gain(void *dev, int gain) { return 0; }

int fc2580_set_gain_mode(void *dev, int manual) { return 0; }

int
r820t_init(void *dev) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	devt->r82xx_p.rtl_dev = dev;

	if (devt->tuner_type == RTLSDR_TUNER_R828D) {
		devt->r82xx_c.i2c_addr = R828D_I2C_ADDR;
		devt->r82xx_c.rafael_chip = CHIP_R828D;
	} else {
		devt->r82xx_c.i2c_addr = R820T_I2C_ADDR;
		devt->r82xx_c.rafael_chip = CHIP_R820T;
	}

	rtlsdr_get_xtal_freq(devt, NULL, &devt->r82xx_c.xtal);

	devt->r82xx_c.max_i2c_msg_len = 8;
	devt->r82xx_c.use_predetect = 0;
	devt->r82xx_p.cfg = &devt->r82xx_c;

	return r82xx_init(&devt->r82xx_p);
}

int
r820t_exit(void *dev) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return r82xx_standby(&devt->r82xx_p);
}

int
r820t_set_freq(void *dev, u32int freq) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return r82xx_set_freq(&devt->r82xx_p, freq);
}

int
r820t_set_bw(void *dev, int bw) {
	int r;
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;

	r = r82xx_set_bandwidth(&devt->r82xx_p, bw, devt->rate);
	if(r < 0)
		return r;
	r = rtlsdr_set_if_freq(devt, r);
	if (r)
		return r;
	return rtlsdr_set_center_freq(devt, devt->freq);
}

int
r820t_set_gain(void *dev, int gain) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return r82xx_set_gain(&devt->r82xx_p, 1, gain);
}

int
r820t_set_gain_mode(void *dev, int manual) {
	rtlsdr_dev_t* devt = (rtlsdr_dev_t*)dev;
	return r82xx_set_gain(&devt->r82xx_p, manual, 0);
}



/* definition order must match enum rtlsdr_tuner */
static rtlsdr_tuner_iface_t tuners[] = {
	{
		NULL, NULL, NULL, NULL, NULL, NULL, NULL /* dummy for unknown tuners */
	},
	{
		e4000_init, e4000_exit,
		e4000_set_freq, e4000_set_bw, e4000_set_gain, e4000_set_if_gain,
		e4000_set_gain_mode
	},
	{
		_fc0012_init, fc0012_exit,
		fc0012_set_freq, fc0012_set_bw, _fc0012_set_gain, NULL,
		fc0012_set_gain_mode
	},
	{
		_fc0013_init, fc0013_exit,
		fc0013_set_freq, fc0013_set_bw, _fc0013_set_gain, NULL,
		fc0013_set_gain_mode
	},
	{
		fc2580_init, fc2580_exit,
		_fc2580_set_freq, fc2580_set_bw, fc2580_set_gain, NULL,
		fc2580_set_gain_mode
	},
	{
		r820t_init, r820t_exit,
		r820t_set_freq, r820t_set_bw, r820t_set_gain, NULL,
		r820t_set_gain_mode
	},
	{
		r820t_init, r820t_exit,
		r820t_set_freq, r820t_set_bw, r820t_set_gain, NULL,
		r820t_set_gain_mode
	},
};



int
rtlsdr_read_array(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int *array, u8int len)
{
	int r;
	u16int index = (block << 8);

//	r = libusb_control_transfer(dev->devh, CTRL_IN, 0, addr, index, array, len, CTRL_TIMEOUT);
    r = usbcmd(dev->usbd, CTRL_IN, 0, addr, index, array, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_read_array: %r\n", argv0);

	return r;
}


int
rtlsdr_write_array(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int *array, u8int len)
{
	int r;
	u16int index = (block << 8) | 0x10;

//	r = libusb_control_transfer(dev->devh, CTRL_OUT, 0, addr, index, array, len, CTRL_TIMEOUT);
    r = usbcmd(dev->usbd, CTRL_OUT, 0, addr, index, array, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_write_array: %r\n", argv0);

	return r;
}


int
rtlsdr_i2c_write_reg(rtlsdr_dev_t *dev, u8int i2c_addr, u8int reg, u8int val)
{
	u16int addr = i2c_addr;
	u8int data[2];

	data[0] = reg;
	data[1] = val;
	return rtlsdr_write_array(dev, IICB, addr, (u8int *)&data, 2);
}


u8int
rtlsdr_i2c_read_reg(rtlsdr_dev_t *dev, u8int i2c_addr, u8int reg)
{
	u16int addr = i2c_addr;
	u8int data = 0;

	rtlsdr_write_array(dev, IICB, addr, &reg, 1);
	rtlsdr_read_array(dev, IICB, addr, &data, 1);

	return data;
}


int
rtlsdr_i2c_write(rtlsdr_dev_t *dev, u8int i2c_addr, u8int *buffer, int len)
{
	u16int addr = i2c_addr;

	if (!dev)
		return -1;

	return rtlsdr_write_array(dev, IICB, addr, buffer, len);
}


int
rtlsdr_i2c_read(rtlsdr_dev_t *dev, u8int i2c_addr, u8int *buffer, int len)
{
	u16int addr = i2c_addr;

	if (!dev)
		return -1;

	return rtlsdr_read_array(dev, IICB, addr, buffer, len);
}


u16int
rtlsdr_read_reg(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int len)
{
	int r;
	unsigned char data[2];
	u16int index = (block << 8);
	u16int reg;

//	r = libusb_control_transfer(dev->devh, CTRL_IN, 0, addr, index, data, len, CTRL_TIMEOUT);
    r = usbcmd(dev->usbd, CTRL_IN, 0, addr, index, data, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_read_reg: %r\n", argv0);

	reg = (data[1] << 8) | data[0];

	return reg;
}


int
rtlsdr_write_reg(rtlsdr_dev_t *dev, u8int block, u16int addr, u16int val, u8int len)
{
	int r;
	unsigned char data[2];

	u16int index = (block << 8) | 0x10;

	if (len == 1)
		data[0] = val & 0xff;
	else
		data[0] = val >> 8;

	data[1] = val & 0xff;

//	r = libusb_control_transfer(dev->devh, CTRL_OUT, 0, addr, index, data, len, CTRL_TIMEOUT);
    r = usbcmd(dev->usbd, CTRL_OUT, 0, addr, index, data, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_write_reg; %r\n", argv0);

	return r;
}


u16int
rtlsdr_demod_read_reg(rtlsdr_dev_t *dev, u8int page, u16int addr, u8int len)
{
	int r;
	unsigned char data[2];

	u16int index = page;
	u16int reg;
	addr = (addr << 8) | 0x20;

//	r = libusb_control_transfer(dev->devh, CTRL_IN, 0, addr, index, data, len, CTRL_TIMEOUT);
    r =usbcmd(dev->usbd, CTRL_IN, 0, addr, index, data, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_demod_read_reg: %r\n", argv0);

	reg = (data[1] << 8) | data[0];

	return reg;
}


int
rtlsdr_demod_write_reg(rtlsdr_dev_t *dev, u8int page, u16int addr, u16int val, u8int len)
{
	int r;
	unsigned char data[2];
	u16int index = 0x10 | page;
	addr = (addr << 8) | 0x20;

	if (len == 1)
		data[0] = val & 0xff;
	else
		data[0] = val >> 8;

	data[1] = val & 0xff;

//	r = libusb_control_transfer(dev->devh, CTRL_OUT, 0, addr, index, data, len, CTRL_TIMEOUT);
    r =usbcmd(dev->usbd, CTRL_OUT, 0, addr, index, data, len);

	if (r < 0)
		fprint(2, "%s: rtlsdr_demod_write_reg: %r\n", argv0);

	rtlsdr_demod_read_reg(dev, 0x0a, 0x01, 1);

	return (r == len) ? 0 : -1;
}


void
rtlsdr_set_gpio_bit(rtlsdr_dev_t *dev, u8int gpio, int val)
{
	u16int r;

	gpio = 1 << gpio;
	r = rtlsdr_read_reg(dev, SYSB, GPO, 1);
	r = val ? (r | gpio) : (r & ~gpio);
	rtlsdr_write_reg(dev, SYSB, GPO, r, 1);
}


void
rtlsdr_set_gpio_output(rtlsdr_dev_t *dev, u8int gpio)
{
	int r;
	gpio = 1 << gpio;

	r = rtlsdr_read_reg(dev, SYSB, GPD, 1);
	rtlsdr_write_reg(dev, SYSB, GPD, r & ~gpio, 1);
	r = rtlsdr_read_reg(dev, SYSB, GPOE, 1);
	rtlsdr_write_reg(dev, SYSB, GPOE, r | gpio, 1);
}


void
rtlsdr_set_i2c_repeater(rtlsdr_dev_t *dev, int on)
{
	rtlsdr_demod_write_reg(dev, 1, 0x01, on ? 0x18 : 0x10, 1);
}


int
rtlsdr_set_fir(rtlsdr_dev_t *dev)
{
	u8int fir[20];

	int i;
	/* format: s8int[8] */
	for (i = 0; i < 8; ++i) {
		const int val = dev->fir[i];
		if (val < -128 || val > 127) {
			return -1;
		}
		fir[i] = val;
	}
	/* format: int12_t[8] */
	for (i = 0; i < 8; i += 2) {
		const int val0 = dev->fir[8+i];
		const int val1 = dev->fir[8+i+1];
		if (val0 < -2048 || val0 > 2047 || val1 < -2048 || val1 > 2047) {
			return -1;
		}
		fir[8+i*3/2] = val0 >> 4;
		fir[8+i*3/2+1] = (val0 << 4) | ((val1 >> 8) & 0x0f);
		fir[8+i*3/2+2] = val1;
	}

	for (i = 0; i < (int)sizeof(fir); i++) {
		if (rtlsdr_demod_write_reg(dev, 1, 0x1c + i, fir[i], 1))
				return -1;
	}

	return 0;
}


void
rtlsdr_init_baseband(rtlsdr_dev_t *dev)
{
	unsigned int i;

	/* initialize USB */
	rtlsdr_write_reg(dev, USBB, USB_SYSCTL, 0x09, 1);
	rtlsdr_write_reg(dev, USBB, USB_EPA_MAXPKT, 0x0002, 2);
	rtlsdr_write_reg(dev, USBB, USB_EPA_CTL, 0x1002, 2);

	/* poweron demod */
	rtlsdr_write_reg(dev, SYSB, DEMOD_CTL_1, 0x22, 1);
	rtlsdr_write_reg(dev, SYSB, DEMOD_CTL, 0xe8, 1);

	/* reset demod (bit 3, soft_rst) */
	rtlsdr_demod_write_reg(dev, 1, 0x01, 0x14, 1);
	rtlsdr_demod_write_reg(dev, 1, 0x01, 0x10, 1);

	/* disable spectrum inversion and adjacent channel rejection */
	rtlsdr_demod_write_reg(dev, 1, 0x15, 0x00, 1);
	rtlsdr_demod_write_reg(dev, 1, 0x16, 0x0000, 2);

	/* clear both DDC shift and IF frequency registers  */
	for (i = 0; i < 6; i++)
		rtlsdr_demod_write_reg(dev, 1, 0x16 + i, 0x00, 1);

	rtlsdr_set_fir(dev);

	/* enable SDR mode, disable DAGC (bit 5) */
	rtlsdr_demod_write_reg(dev, 0, 0x19, 0x05, 1);

	/* init FSM state-holding register */
	rtlsdr_demod_write_reg(dev, 1, 0x93, 0xf0, 1);
	rtlsdr_demod_write_reg(dev, 1, 0x94, 0x0f, 1);

	/* disable AGC (en_dagc, bit 0) (this seems to have no effect) */
	rtlsdr_demod_write_reg(dev, 1, 0x11, 0x00, 1);

	/* disable RF and IF AGC loop */
	rtlsdr_demod_write_reg(dev, 1, 0x04, 0x00, 1);

	/* disable PID filter (enable_PID = 0) */
	rtlsdr_demod_write_reg(dev, 0, 0x61, 0x60, 1);

	/* opt_adc_iq = 0, default ADC_I/ADC_Q datapath */
	rtlsdr_demod_write_reg(dev, 0, 0x06, 0x80, 1);

	/* Enable Zero-IF mode (en_bbin bit), DC cancellation (en_dc_est),
	 * IQ estimation/compensation (en_iq_comp, en_iq_est) */
	rtlsdr_demod_write_reg(dev, 1, 0xb1, 0x1b, 1);

	/* disable 4.096 MHz clock output on pin TP_CK0 */
	rtlsdr_demod_write_reg(dev, 0, 0x0d, 0x83, 1);
}


int
rtlsdr_deinit_baseband(rtlsdr_dev_t *dev)
{
	int r = 0;

	if (!dev)
		return -1;

	if (dev->tuner && dev->tuner->exit) {
		rtlsdr_set_i2c_repeater(dev, 1);
		r = dev->tuner->exit(dev); /* deinitialize tuner */
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	/* poweroff demodulator and ADCs */
	rtlsdr_write_reg(dev, SYSB, DEMOD_CTL, 0x20, 1);

	return r;
}


static int
rtlsdr_set_if_freq(rtlsdr_dev_t *dev, u32int freq)
{
	u32int rtl_xtal;
	s32int if_freq;
	u8int tmp;
	int r;

	if (!dev)
		return -1;

	/* read corrected clock value */
	if (rtlsdr_get_xtal_freq(dev, &rtl_xtal, NULL))
		return -2;

	if_freq = ((freq * TWO_POW(22)) / rtl_xtal) * (-1);

	tmp = (if_freq >> 16) & 0x3f;
	r = rtlsdr_demod_write_reg(dev, 1, 0x19, tmp, 1);
	tmp = (if_freq >> 8) & 0xff;
	r |= rtlsdr_demod_write_reg(dev, 1, 0x1a, tmp, 1);
	tmp = if_freq & 0xff;
	r |= rtlsdr_demod_write_reg(dev, 1, 0x1b, tmp, 1);

	return r;
}


int
rtlsdr_set_sample_freq_correction(rtlsdr_dev_t *dev, int ppm)
{
	int r = 0;
	u8int tmp;
	s16int offs = ppm * (-1) * TWO_POW(24) / 1000000;

	tmp = offs & 0xff;
	r |= rtlsdr_demod_write_reg(dev, 1, 0x3f, tmp, 1);
	tmp = (offs >> 8) & 0x3f;
	r |= rtlsdr_demod_write_reg(dev, 1, 0x3e, tmp, 1);

	return r;
}


int
rtlsdr_set_xtal_freq(rtlsdr_dev_t *dev, u32int rtl_freq, u32int tuner_freq)
{
	int r = 0;

	if (!dev)
		return -1;

	if (rtl_freq > 0 &&
		(rtl_freq < MIN_RTL_XTAL_FREQ || rtl_freq > MAX_RTL_XTAL_FREQ))
		return -2;

	if (rtl_freq > 0 && dev->rtl_xtal != rtl_freq) {
		dev->rtl_xtal = rtl_freq;

		/* update xtal-dependent settings */
		if (dev->rate)
			r = rtlsdr_set_sample_rate(dev, dev->rate);
	}

	if (dev->tun_xtal != tuner_freq) {
		if (0 == tuner_freq)
			dev->tun_xtal = dev->rtl_xtal;
		else
			dev->tun_xtal = tuner_freq;

		/* read corrected clock value into e4k and r82xx structure */
		if (rtlsdr_get_xtal_freq(dev, NULL, &dev->e4k_s.vco.fosc) ||
		    rtlsdr_get_xtal_freq(dev, NULL, &dev->r82xx_c.xtal))
			return -3;

		/* update xtal-dependent settings */
		if (dev->freq)
			r = rtlsdr_set_center_freq(dev, dev->freq);
	}

	return r;
}


int
rtlsdr_get_xtal_freq(rtlsdr_dev_t *dev, u32int *rtl_freq, u32int *tuner_freq)
{
	if (!dev)
		return -1;

	if (rtl_freq)
		*rtl_freq = (u32int) APPLY_PPM_CORR(dev->rtl_xtal, dev->corr);

	if (tuner_freq)
		*tuner_freq = (u32int) APPLY_PPM_CORR(dev->tun_xtal, dev->corr);

	return 0;
}


int
rtlsdr_write_eeprom(rtlsdr_dev_t *dev, u8int *data, u8int offset, u16int len)
{
	int r = 0;
	int i;
	u8int cmd[2];

	if (!dev)
		return -1;

	if ((len + offset) > 256)
		return -2;

	for (i = 0; i < len; i++) {
		cmd[0] = i + offset;
		r = rtlsdr_write_array(dev, IICB, EEPROM_ADDR, cmd, 1);
		r = rtlsdr_read_array(dev, IICB, EEPROM_ADDR, &cmd[1], 1);

		/* only write the byte if it differs */
		if (cmd[1] == data[i])
			continue;

		cmd[1] = data[i];
		r = rtlsdr_write_array(dev, IICB, EEPROM_ADDR, cmd, 2);
		if (r != sizeof(cmd))
			return -3;

		/* for some EEPROMs (e.g. ATC 240LC02) we need a delay
		 * between write operations, otherwise they will fail */

		sleep(5);

	}

	return 0;
}


int
rtlsdr_read_eeprom(rtlsdr_dev_t *dev, u8int *data, u8int offset, u16int len)
{
	int r = 0;
	int i;

	if (!dev)
		return -1;

	if ((len + offset) > 256)
		return -2;

	r = rtlsdr_write_array(dev, IICB, EEPROM_ADDR, &offset, 1);
	if (r < 0)
		return -3;

	for (i = 0; i < len; i++) {
		r = rtlsdr_read_array(dev, IICB, EEPROM_ADDR, data + i, 1);

		if (r < 0)
			return -3;
	}

	return r;
}


int
rtlsdr_set_center_freq(rtlsdr_dev_t *dev, u32int freq)
{
	int r = -1;

	if (!dev || !dev->tuner)
		return -1;

	if (dev->direct_sampling) {
		r = rtlsdr_set_if_freq(dev, freq);
	} else if (dev->tuner && dev->tuner->set_freq) {
		rtlsdr_set_i2c_repeater(dev, 1);
		r = dev->tuner->set_freq(dev, freq - dev->offs_freq);
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	if (!r)
		dev->freq = freq;
	else
		dev->freq = 0;

	return r;
}


u32int
rtlsdr_get_center_freq(rtlsdr_dev_t *dev)
{
	if (!dev)
		return 0;

	return dev->freq;
}


int
rtlsdr_set_freq_correction(rtlsdr_dev_t *dev, int ppm)
{
	int r = 0;

	if (!dev)
		return -1;

	if (dev->corr == ppm)
		return -2;

	dev->corr = ppm;

	r |= rtlsdr_set_sample_freq_correction(dev, ppm);

	/* read corrected clock value into e4k and r82xx structure */
	if (rtlsdr_get_xtal_freq(dev, NULL, &dev->e4k_s.vco.fosc) ||
	    rtlsdr_get_xtal_freq(dev, NULL, &dev->r82xx_c.xtal))
		return -3;

	if (dev->freq) /* retune to apply new correction value */
		r |= rtlsdr_set_center_freq(dev, dev->freq);

	return r;
}


int
rtlsdr_get_freq_correction(rtlsdr_dev_t *dev)
{
	if (!dev)
		return 0;

	return dev->corr;
}


int
rtlsdr_get_tuner_gains(rtlsdr_dev_t *dev, int *gains)
{
	/* all gain values are expressed in tenths of a dB */
	const int e4k_gains[] = { -10, 15, 40, 65, 90, 115, 140, 165, 190, 215,
				  240, 290, 340, 420 };
	const int fc0012_gains[] = { -99, -40, 71, 179, 192 };
	const int fc0013_gains[] = { -99, -73, -65, -63, -60, -58, -54, 58, 61,
				       63, 65, 67, 68, 70, 71, 179, 181, 182,
				       184, 186, 188, 191, 197 };
	const int fc2580_gains[] = { 0 /* no gain values */ };
	const int r82xx_gains[] = { 0, 9, 14, 27, 37, 77, 87, 125, 144, 157,
				     166, 197, 207, 229, 254, 280, 297, 328,
				     338, 364, 372, 386, 402, 421, 434, 439,
				     445, 480, 496 };
	const int unknown_gains[] = { 0 /* no gain values */ };

	const int *ptr = NULL;
	int len = 0;

	if (!dev)
		return -1;

	switch (dev->tuner_type) {
	case RTLSDR_TUNER_E4000:
		ptr = e4k_gains; len = sizeof(e4k_gains);
		break;
	case RTLSDR_TUNER_FC0012:
		ptr = fc0012_gains; len = sizeof(fc0012_gains);
		break;
	case RTLSDR_TUNER_FC0013:
		ptr = fc0013_gains; len = sizeof(fc0013_gains);
		break;
	case RTLSDR_TUNER_FC2580:
		ptr = fc2580_gains; len = sizeof(fc2580_gains);
		break;
	case RTLSDR_TUNER_R820T:
	case RTLSDR_TUNER_R828D:
		ptr = r82xx_gains; len = sizeof(r82xx_gains);
		break;
	default:
		ptr = unknown_gains; len = sizeof(unknown_gains);
		break;
	}

	if (!gains) { /* no buffer provided, just return the count */
		return len / sizeof(int);
	} else {
		if (len)
			memcpy(gains, ptr, len);

		return len / sizeof(int);
	}
}


int
rtlsdr_set_tuner_bandwidth(rtlsdr_dev_t *dev, u32int bw)
{
	int r = 0;

	if (!dev || !dev->tuner)
		return -1;

	if (dev->tuner->set_bw) {
		rtlsdr_set_i2c_repeater(dev, 1);
		r = dev->tuner->set_bw(dev, bw > 0 ? bw : dev->rate);
		rtlsdr_set_i2c_repeater(dev, 0);
		if (r)
			return r;
		dev->bw = bw;
	}
	return r;
}


int
rtlsdr_set_tuner_gain(rtlsdr_dev_t *dev, int gain)
{
	int r = 0;

	if (!dev || !dev->tuner)
		return -1;

	if (dev->tuner->set_gain) {
		rtlsdr_set_i2c_repeater(dev, 1);
		r = dev->tuner->set_gain((void *)dev, gain);
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	if (!r)
		dev->gain = gain;
	else
		dev->gain = 0;

	return r;
}


int
rtlsdr_get_tuner_gain(rtlsdr_dev_t *dev)
{
	if (!dev)
		return 0;

	return dev->gain;
}


int
rtlsdr_set_tuner_gain_mode(rtlsdr_dev_t *dev, int mode)
{
	int r = 0;

	if (!dev || !dev->tuner)
		return -1;

	if (dev->tuner->set_gain_mode) {
		rtlsdr_set_i2c_repeater(dev, 1);
		r = dev->tuner->set_gain_mode((void *)dev, mode);
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	return r;
}


int
rtlsdr_set_sample_rate(rtlsdr_dev_t *dev, u32int samp_rate)
{
	int r = 0;
	u16int tmp;
	u32int rsamp_ratio, real_rsamp_ratio;
	double real_rate;

	if (!dev)
		return -1;

	/* check if the rate is supported by the resampler */
	if ((samp_rate <= 225000) || (samp_rate > 3200000) ||
	   ((samp_rate > 300000) && (samp_rate <= 900000))) {
		fprint(2, "Invalid sample rate: %u Hz\n", samp_rate);
	//	return -EINVAL;
		return -1;
	}

	rsamp_ratio = (dev->rtl_xtal * TWO_POW(22)) / samp_rate;
	rsamp_ratio &= 0x0ffffffc;

	real_rsamp_ratio = rsamp_ratio | ((rsamp_ratio & 0x08000000) << 1);
	real_rate = (dev->rtl_xtal * TWO_POW(22)) / real_rsamp_ratio;

	if ( ((double)samp_rate) != real_rate )
		fprint(2, "Exact sample rate is: %f Hz\n", real_rate);

	dev->rate = (u32int)real_rate;

	if (dev->tuner && dev->tuner->set_bw) {
		rtlsdr_set_i2c_repeater(dev, 1);
		dev->tuner->set_bw(dev, dev->bw > 0 ? dev->bw : dev->rate);
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	tmp = (rsamp_ratio >> 16);
	r |= rtlsdr_demod_write_reg(dev, 1, 0x9f, tmp, 2);
	tmp = rsamp_ratio & 0xffff;
	r |= rtlsdr_demod_write_reg(dev, 1, 0xa1, tmp, 2);

	r |= rtlsdr_set_sample_freq_correction(dev, dev->corr);

	/* reset demod (bit 3, soft_rst) */
	r |= rtlsdr_demod_write_reg(dev, 1, 0x01, 0x14, 1);
	r |= rtlsdr_demod_write_reg(dev, 1, 0x01, 0x10, 1);

	/* recalculate offset frequency if offset tuning is enabled */
	if (dev->offs_freq)
		rtlsdr_set_offset_tuning(dev, 1);

	return r;
}


u32int
rtlsdr_get_sample_rate(rtlsdr_dev_t *dev)
{
	if (!dev)
		return 0;

	return dev->rate;
}


int
rtlsdr_set_testmode(rtlsdr_dev_t *dev, int on)
{
	if (!dev)
		return -1;

	return rtlsdr_demod_write_reg(dev, 0, 0x19, on ? 0x03 : 0x05, 1);
}


int
rtlsdr_set_agc_mode(rtlsdr_dev_t *dev, int on)
{
	if (!dev)
		return -1;

	return rtlsdr_demod_write_reg(dev, 0, 0x19, on ? 0x25 : 0x05, 1);
}


int
rtlsdr_set_direct_sampling(rtlsdr_dev_t *dev, int on)
{
	int r = 0;

	if (!dev)
		return -1;

	if (on) {
		if (dev->tuner && dev->tuner->exit) {
			rtlsdr_set_i2c_repeater(dev, 1);
			r = dev->tuner->exit(dev);
			rtlsdr_set_i2c_repeater(dev, 0);
		}

		/* disable Zero-IF mode */
		r |= rtlsdr_demod_write_reg(dev, 1, 0xb1, 0x1a, 1);

		/* disable spectrum inversion */
		r |= rtlsdr_demod_write_reg(dev, 1, 0x15, 0x00, 1);

		/* only enable In-phase ADC input */
		r |= rtlsdr_demod_write_reg(dev, 0, 0x08, 0x4d, 1);

		/* swap I and Q ADC, this allows to select between two inputs */
		r |= rtlsdr_demod_write_reg(dev, 0, 0x06, (on > 1) ? 0x90 : 0x80, 1);

		fprint(2, "Enabled direct sampling mode, input %i\n", on);
		dev->direct_sampling = on;
	} else {
		if (dev->tuner && dev->tuner->init) {
			rtlsdr_set_i2c_repeater(dev, 1);
			r |= dev->tuner->init(dev);
			rtlsdr_set_i2c_repeater(dev, 0);
		}

		if ((dev->tuner_type == RTLSDR_TUNER_R820T) ||
		    (dev->tuner_type == RTLSDR_TUNER_R828D)) {
			r |= rtlsdr_set_if_freq(dev, R82XX_IF_FREQ);

			/* enable spectrum inversion */
			r |= rtlsdr_demod_write_reg(dev, 1, 0x15, 0x01, 1);
		} else {
			r |= rtlsdr_set_if_freq(dev, 0);

			/* enable In-phase + Quadrature ADC input */
			r |= rtlsdr_demod_write_reg(dev, 0, 0x08, 0xcd, 1);

			/* Enable Zero-IF mode */
			r |= rtlsdr_demod_write_reg(dev, 1, 0xb1, 0x1b, 1);
		}

		/* opt_adc_iq = 0, default ADC_I/ADC_Q datapath */
		r |= rtlsdr_demod_write_reg(dev, 0, 0x06, 0x80, 1);

		fprint(2, "Disabled direct sampling mode\n");
		dev->direct_sampling = 0;
	}

	r |= rtlsdr_set_center_freq(dev, dev->freq);

	return r;
}


int
rtlsdr_get_direct_sampling(rtlsdr_dev_t *dev)
{
	if (!dev)
		return -1;

	return dev->direct_sampling;
}


int
rtlsdr_set_offset_tuning(rtlsdr_dev_t *dev, int on)
{
	int r = 0;
	int bw;

	if (!dev)
		return -1;

	if ((dev->tuner_type == RTLSDR_TUNER_R820T) ||
	    (dev->tuner_type == RTLSDR_TUNER_R828D))
		return -2;

	if (dev->direct_sampling)
		return -3;

	/* based on keenerds 1/f noise measurements */
	dev->offs_freq = on ? ((dev->rate / 2) * 170 / 100) : 0;
	r |= rtlsdr_set_if_freq(dev, dev->offs_freq);

	if (dev->tuner && dev->tuner->set_bw) {
		rtlsdr_set_i2c_repeater(dev, 1);
		if (on) {
			bw = 2 * dev->offs_freq;
		} else if (dev->bw > 0) {
			bw = dev->bw;
		} else {
			bw = dev->rate;
		}
		dev->tuner->set_bw(dev, bw);
		rtlsdr_set_i2c_repeater(dev, 0);
	}

	if (dev->freq > dev->offs_freq)
		r |= rtlsdr_set_center_freq(dev, dev->freq);

	return r;
}


int
rtlsdr_get_offset_tuning(rtlsdr_dev_t *dev)
{
	if (!dev)
		return -1;

	return (dev->offs_freq) ? 1 : 0;
}


int
rtlsdr_reset_buffer(rtlsdr_dev_t *dev)
{
	if (!dev)
		return -1;

	rtlsdr_write_reg(dev, USBB, USB_EPA_CTL, 0x1002, 2);
	rtlsdr_write_reg(dev, USBB, USB_EPA_CTL, 0x0000, 2);

	return 0;
}


u32int
rtlsdr_get_tuner_clock(void *dev)
{
	u32int tuner_freq;

	if (!dev)
		return 0;

	/* read corrected clock value */
	if (rtlsdr_get_xtal_freq((rtlsdr_dev_t *)dev, NULL, &tuner_freq))
		return 0;

	return tuner_freq;
}


int
rtlsdr_i2c_write_fn(void *dev, u8int addr, u8int *buf, int len)
{
	if (dev)
		return rtlsdr_i2c_write(((rtlsdr_dev_t *)dev), addr, buf, len);

	return -1;
}


int
rtlsdr_i2c_read_fn(void *dev, u8int addr, u8int *buf, int len)
{
	if (dev)
		return rtlsdr_i2c_read(((rtlsdr_dev_t *)dev), addr, buf, len);

	return -1;
}


int
rtlsdr_set_bias_tee_gpio(rtlsdr_dev_t *dev, int gpio, int on)
{
	if (!dev)
		return -1;

	rtlsdr_set_gpio_output(dev, gpio);
	rtlsdr_set_gpio_bit(dev, gpio, on);

	return 0;
}


int
rtlsdr_set_bias_tee(rtlsdr_dev_t *dev, int on)
{
	return rtlsdr_set_bias_tee_gpio(dev, 0, on);
}



int
rtlsdr_open(rtlsdr_dev_t **out_dev, char *endpoint)
{
	int r;
	int i;
	u8int reg;

	Usbdev *ud;
	Ep *ep;


	rtlsdr_dev_t *dev = nil;

	dev = malloc(sizeof(rtlsdr_dev_t));
	if (dev == nil)
		sysfatal("no mem for malloc, rtlsdr_open");

	memset(dev, 0, sizeof(rtlsdr_dev_t));
	memcpy(dev->fir, fir_default, sizeof(fir_default));

	dev->dev_lost = 1; 


/* 9Front, set the USB Dev struct in the rtlsdr_dev struct */
	dev->usbd = getdev(endpoint);
	if(dev->usbd == nil)
		sysfatal("getdev in rtlsdr_open failed");

/* find the bulk read endpoint */
//	ep = nil;
	ud = dev->usbd->usb;

	for(i = 0; i < nelem(ud->ep); i++){
		if((ep = ud->ep[i]) == nil)
			continue;
		if(ep->type == Ebulk && ep->dir == Ein)
			break;
	}

	print("found ep at %d\n", ep->id);


	dev->readradio = openep(dev->usbd, ep);

	if(dev->readradio == nil)
		sysfatal("openep failed: %r");

/* set the bulk read endpoints data file to be readable */
	if(opendevdata(dev->readradio, OREAD) < 0)
		sysfatal("opendevdata: %r");

	dev->rtl_xtal = DEF_RTL_XTAL_FREQ;

	/* perform a dummy write, if it fails, reset the device */
	if (rtlsdr_write_reg(dev, USBB, USB_SYSCTL, 0x09, 1) < 0)
		sysfatal("dummy write failed");

	rtlsdr_init_baseband(dev);
	dev->dev_lost = 0;

	/* Probe tuners */
	rtlsdr_set_i2c_repeater(dev, 1);

	reg = rtlsdr_i2c_read_reg(dev, E4K_I2C_ADDR, E4K_CHECK_ADDR);
	if (reg == E4K_CHECK_VAL) {
		fprint(2, "Found Elonics E4000 tuner\n");
		dev->tuner_type = RTLSDR_TUNER_E4000;
		goto found;
	}

	reg = rtlsdr_i2c_read_reg(dev, FC0013_I2C_ADDR, FC0013_CHECK_ADDR);
	if (reg == FC0013_CHECK_VAL) {
		fprint(2, "Found Fitipower FC0013 tuner\n");
		dev->tuner_type = RTLSDR_TUNER_FC0013;
		goto found;
	}

	reg = rtlsdr_i2c_read_reg(dev, R820T_I2C_ADDR, R82XX_CHECK_ADDR);
	if (reg == R82XX_CHECK_VAL) {
		fprint(2, "Found Rafael Micro R820T tuner\n");
		dev->tuner_type = RTLSDR_TUNER_R820T;
		goto found;
	}

	reg = rtlsdr_i2c_read_reg(dev, R828D_I2C_ADDR, R82XX_CHECK_ADDR);
	if (reg == R82XX_CHECK_VAL) {
		fprint(2, "Found Rafael Micro R828D tuner\n");
		dev->tuner_type = RTLSDR_TUNER_R828D;
		goto found;
	}

	/* initialise GPIOs */
	rtlsdr_set_gpio_output(dev, 4);

	/* reset tuner before probing */
	rtlsdr_set_gpio_bit(dev, 4, 1);
	rtlsdr_set_gpio_bit(dev, 4, 0);

	reg = rtlsdr_i2c_read_reg(dev, FC2580_I2C_ADDR, FC2580_CHECK_ADDR);
	if ((reg & 0x7f) == FC2580_CHECK_VAL) {
		fprint(2, "Found FCI 2580 tuner\n");
		dev->tuner_type = RTLSDR_TUNER_FC2580;
		goto found;
	}

	reg = rtlsdr_i2c_read_reg(dev, FC0012_I2C_ADDR, FC0012_CHECK_ADDR);
	if (reg == FC0012_CHECK_VAL) {
		fprint(2, "Found Fitipower FC0012 tuner\n");
		rtlsdr_set_gpio_output(dev, 6);
		dev->tuner_type = RTLSDR_TUNER_FC0012;
		goto found;
	}

found:
	/* use the rtl clock value by default */
	dev->tun_xtal = dev->rtl_xtal;
	dev->tuner = &tuners[dev->tuner_type];

	switch (dev->tuner_type) {
	case RTLSDR_TUNER_R828D:
		dev->tun_xtal = R828D_XTAL_FREQ;
		/* fall-through */
	case RTLSDR_TUNER_R820T:
		/* disable Zero-IF mode */
		rtlsdr_demod_write_reg(dev, 1, 0xb1, 0x1a, 1);

		/* only enable In-phase ADC input */
		rtlsdr_demod_write_reg(dev, 0, 0x08, 0x4d, 1);

		/* the R82XX use 3.57 MHz IF for the DVB-T 6 MHz mode, and
		 * 4.57 MHz for the 8 MHz mode */
		rtlsdr_set_if_freq(dev, R82XX_IF_FREQ);

		/* enable spectrum inversion */
		rtlsdr_demod_write_reg(dev, 1, 0x15, 0x01, 1);
		break;
	case RTLSDR_TUNER_UNKNOWN:
		fprint(2, "No supported tuner found\n");
		rtlsdr_set_direct_sampling(dev, 1);
		break;
	default:
		break;
	}

	if (dev->tuner->init)
		r = dev->tuner->init(dev);

	rtlsdr_set_i2c_repeater(dev, 0);

	*out_dev = dev;

	return 0;
err:
	if (dev) {
		if (dev->usbd)
			closedev(dev->usbd);

		free(dev);
	}

	return r;
}


int
rtlsdr_close(rtlsdr_dev_t *dev)
{
	rtlsdr_deinit_baseband(dev);

	closedev(dev->usbd);

	free(dev);

	return 0;
}


