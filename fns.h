
/* librtlsdr.c */
int		rtlsdr_read_array(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int *array, u8int len);
int		rtlsdr_write_array(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int *array, u8int len);

int		rtlsdr_i2c_write_reg(rtlsdr_dev_t *dev, u8int i2c_addr, u8int reg, u8int val);
u8int	rtlsdr_i2c_read_reg(rtlsdr_dev_t *dev, u8int i2c_addr, u8int reg);

int		rtlsdr_i2c_write(rtlsdr_dev_t *dev, u8int i2c_addr, u8int *buffer, int len);
int		rtlsdr_i2c_read(rtlsdr_dev_t *dev, u8int i2c_addr, u8int *buffer, int len);

u16int	rtlsdr_read_reg(rtlsdr_dev_t *dev, u8int block, u16int addr, u8int len);
int		rtlsdr_write_reg(rtlsdr_dev_t *dev, u8int block, u16int addr, u16int val, u8int len);

u16int	rtlsdr_demod_read_reg(rtlsdr_dev_t *dev, u8int page, u16int addr, u8int len);
int		rtlsdr_demod_write_reg(rtlsdr_dev_t *dev, u8int page, u16int addr, u16int val, u8int len);

void	rtlsdr_set_gpio_bit(rtlsdr_dev_t *dev, u8int gpio, int val);
void	rtlsdr_set_gpio_output(rtlsdr_dev_t *dev, u8int gpio);
void	rtlsdr_set_i2c_repeater(rtlsdr_dev_t *dev, int on);

int		rtlsdr_set_fir(rtlsdr_dev_t *dev);
void	rtlsdr_init_baseband(rtlsdr_dev_t *dev);
int		rtlsdr_deinit_baseband(rtlsdr_dev_t *dev);

static int	rtlsdr_set_if_freq(rtlsdr_dev_t *dev, u32int freq);

int		rtlsdr_set_sample_freq_correction(rtlsdr_dev_t *dev, int ppm);
int		rtlsdr_set_xtal_freq(rtlsdr_dev_t *dev, u32int rtl_freq, u32int tuner_freq);
int		rtlsdr_get_xtal_freq(rtlsdr_dev_t *dev, u32int *rtl_freq, u32int *tuner_freq);

int		rtlsdr_write_eeprom(rtlsdr_dev_t *dev, u8int *data, u8int offset, u16int len);
int		rtlsdr_read_eeprom(rtlsdr_dev_t *dev, u8int *data, u8int offset, u16int len);

int		rtlsdr_set_center_freq(rtlsdr_dev_t *dev, u32int freq);
u32int	rtlsdr_get_center_freq(rtlsdr_dev_t *dev);

int		rtlsdr_set_freq_correction(rtlsdr_dev_t *dev, int ppm);
int		rtlsdr_get_freq_correction(rtlsdr_dev_t *dev);

int		rtlsdr_get_tuner_gains(rtlsdr_dev_t *dev, int *gains);

int		rtlsdr_set_tuner_bandwidth(rtlsdr_dev_t *dev, u32int bw);

int		rtlsdr_set_tuner_gain(rtlsdr_dev_t *dev, int gain);
int		rtlsdr_get_tuner_gain(rtlsdr_dev_t *dev);

int		rtlsdr_set_tuner_gain_mode(rtlsdr_dev_t *dev, int mode);
int		rtlsdr_set_sample_rate(rtlsdr_dev_t *dev, u32int samp_rate);
u32int	rtlsdr_get_sample_rate(rtlsdr_dev_t *dev);
int		rtlsdr_set_testmode(rtlsdr_dev_t *dev, int on);
int		rtlsdr_set_agc_mode(rtlsdr_dev_t *dev, int on);
int		rtlsdr_set_direct_sampling(rtlsdr_dev_t *dev, int on);
int		rtlsdr_get_direct_sampling(rtlsdr_dev_t *dev);
int		rtlsdr_set_offset_tuning(rtlsdr_dev_t *dev, int on);
int		rtlsdr_get_offset_tuning(rtlsdr_dev_t *dev);

int		rtlsdr_reset_buffer(rtlsdr_dev_t *dev);

u32int	rtlsdr_get_tuner_clock(void *dev);

int		rtlsdr_i2c_write_fn(void *dev, u8int addr, u8int *buf, int len);
int		rtlsdr_i2c_read_fn(void *dev, u8int addr, u8int *buf, int len);
int		rtlsdr_set_bias_tee_gpio(rtlsdr_dev_t *dev, int gpio, int on);
int		rtlsdr_set_bias_tee(rtlsdr_dev_t *dev, int on);

int		rtlsdr_open(rtlsdr_dev_t **out_dev, char *endpoint);
int		rtlsdr_close(rtlsdr_dev_t *dev);




/* tuner_r82xx.c */

int r82xx_standby(struct r82xx_priv *priv);
int r82xx_init(struct r82xx_priv *priv);
int r82xx_set_freq(struct r82xx_priv *priv, u32int freq);
int r82xx_set_gain(struct r82xx_priv *priv, int set_manual_gain, int gain);
int r82xx_set_bandwidth(struct r82xx_priv *priv, int bandwidth,  u32int rate);



/* tuner_e4k.c */

int e4k_init(struct e4k_state *e4k);
int e4k_standby(struct e4k_state *e4k, int enable);
int e4k_if_gain_set(struct e4k_state *e4k, u8int stage, s8int value);
int e4k_mixer_gain_set(struct e4k_state *e4k, s8int value);
int e4k_commonmode_set(struct e4k_state *e4k, s8int value);
int e4k_tune_freq(struct e4k_state *e4k, u32int freq);
int e4k_tune_params(struct e4k_state *e4k, struct e4k_pll_params *p);
u32int e4k_compute_pll_params(struct e4k_pll_params *oscp, u32int fosc, u32int intended_flo);
int e4k_if_filter_bw_get(struct e4k_state *e4k, enum e4k_if_filter filter);
int e4k_if_filter_bw_set(struct e4k_state *e4k, enum e4k_if_filter filter,
		         u32int bandwidth);
int e4k_if_filter_chan_enable(struct e4k_state *e4k, int on);
int e4k_rf_filter_set(struct e4k_state *e4k);

int e4k_manual_dc_offset(struct e4k_state *e4k, s8int iofs, s8int irange, s8int qofs, s8int qrange);
int e4k_dc_offset_calibrate(struct e4k_state *e4k);
int e4k_dc_offset_gen_table(struct e4k_state *e4k);

int e4k_set_lna_gain(struct e4k_state *e4k, s32int gain);
int e4k_enable_manual_gain(struct e4k_state *e4k, u8int manual);
int e4k_set_enh_gain(struct e4k_state *e4k, s32int gain);


/* tuner fc0012 */

int fc0012_init(void *dev);
int fc0012_set_params(void *dev, u32int freq, u32int bandwidth);
int fc0012_set_gain(void *dev, int gain);


/* tuner fc0013 */

int fc0013_init(void *dev);
int fc0013_set_params(void *dev, u32int freq, u32int bandwidth);
int fc0013_set_gain_mode(void *dev, int manual);
int fc0013_set_lna_gain(void *dev, int gain);


/* fc2580 */

extern void fc2580_wait_msec(void *pTuner, int a);

fc2580_fci_result_type fc2580_i2c_write(void *pTuner, unsigned char reg, unsigned char val);
fc2580_fci_result_type fc2580_i2c_read(void *pTuner, unsigned char reg, unsigned char *read_data);

/*==============================================================================
       fc2580 initial setting

  This function is a generic function which gets called to initialize

  fc2580 in DVB-H mode or L-Band TDMB mode

  <input parameter>

  ifagc_mode
    type : integer
	1 : Internal AGC
	2 : Voltage Control Mode

==============================================================================*/
fc2580_fci_result_type fc2580_set_init(void *pTuner, int ifagc_mode, unsigned int freq_xtal );

/*==============================================================================
       fc2580 frequency setting

  This function is a generic function which gets called to change LO Frequency

  of fc2580 in DVB-H mode or L-Band TDMB mode

  <input parameter>

  f_lo
	Value of target LO Frequency in 'kHz' unit
	ex) 2.6GHz = 2600000

==============================================================================*/
fc2580_fci_result_type fc2580_set_freq(void *pTuner, unsigned int f_lo, unsigned int freq_xtal );


/*==============================================================================
       fc2580 filter BW setting

  This function is a generic function which gets called to change Bandwidth

  frequency of fc2580's channel selection filter

  <input parameter>

  filter_bw
    1 : 1.53MHz(TDMB)
	6 : 6MHz
	7 : 7MHz
	8 : 7.8MHz


==============================================================================*/
fc2580_fci_result_type fc2580_set_filter( void *pTuner, unsigned char filter_bw, unsigned int freq_xtal );

// Manipulaing functions
int
fc2580_Initialize(
	void *pTuner
	);

int
fc2580_SetRfFreqHz(
	void *pTuner,
	unsigned long RfFreqHz
	);

// Extra manipulaing functions
int
fc2580_SetBandwidthMode(
	void *pTuner,
	int BandwidthMode
	);



/* reg field */

u32int reg_field_read(struct reg_field_ops *ops, struct reg_field *field);
int reg_field_write(struct reg_field_ops *ops, struct reg_field *field, u32int val);
int reg_field_cmd(struct cmd_state *cs, enum cmd_op op, const char *cmd, int argc, char **argv, struct reg_field_ops *ops);




/* rtlfm.c */

void rotate_90(unsigned char *buf, u32int len);
void low_pass(struct demod_state *d);
int low_pass_simple(s16int *signal2, int len, int step);
void low_pass_real(struct demod_state *s);
void fifth_order(s16int *data, int length, s16int *hist);
void generic_fir(s16int *data, int length, int *fir, s16int *hist);
void multiply(int ar, int aj, int br, int bj, int *cr, int *cj);
int polar_discriminant(int ar, int aj, int br, int bj);
int fast_atan2(int y, int x);
int polar_disc_fast(int ar, int aj, int br, int bj);
int atan_lut_init(void);
int polar_disc_lut(int ar, int aj, int br, int bj);
void fm_demod(struct demod_state *fm);
void deemph_filter(struct demod_state *fm);
int rms(s16int *samples, int len, int step);
void full_demod(struct demod_state *d);

