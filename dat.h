
/* two raised to the power of n */
#define TWO_POW(n)		((double)(1ULL<<(n)))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MHZ(x)		((x)*1000*1000)
#define KHZ(x)		((x)*1000)

#define DEFAULT_BUF_NUMBER	15
#define DEFAULT_BUF_LENGTH	(16 * 32 * 512)

#define DEF_RTL_XTAL_FREQ	28800000
#define MIN_RTL_XTAL_FREQ	(DEF_RTL_XTAL_FREQ - 1000)
#define MAX_RTL_XTAL_FREQ	(DEF_RTL_XTAL_FREQ + 1000)

#define APPLY_PPM_CORR(val,ppm) (((val) * (1.0 + (ppm) / 1e6)))

#define CTRL_IN     (Rd2h|Rvendor)
#define CTRL_OUT    (Rh2d|Rvendor)
#define EEPROM_ADDR	0xa0


#define NULL nil

#define FIR_LEN 16

typedef struct rtlsdr_dev rtlsdr_dev_t;
typedef struct rtlsdr_dongle rtlsdr_dongle;


static const int fir_default[FIR_LEN] = {
	-54, -36, -41, -40, -32, -14, 14, 53,	/* 8 bit signed */
	101, 156, 215, 273, 327, 372, 404, 421	/* 12 bit signed */
};



enum rtlsdr_tuner {
	RTLSDR_TUNER_UNKNOWN = 0,
	RTLSDR_TUNER_E4000,
	RTLSDR_TUNER_FC0012,
	RTLSDR_TUNER_FC0013,
	RTLSDR_TUNER_FC2580,
	RTLSDR_TUNER_R820T,
	RTLSDR_TUNER_R828D
};

typedef struct rtlsdr_tuner_iface {
	/* tuner interface */
	int (*init)(void *);
	int (*exit)(void *);
	int (*set_freq)(void *, u32int freq /* Hz */);
	int (*set_bw)(void *, int bw /* Hz */);
	int (*set_gain)(void *, int gain /* tenth dB */);
	int (*set_if_gain)(void *, int stage, int gain /* tenth dB */);
	int (*set_gain_mode)(void *, int manual);
} rtlsdr_tuner_iface_t;






enum rtlsdr_async_status {
	RTLSDR_INACTIVE = 0,
	RTLSDR_CANCELING,
	RTLSDR_RUNNING
};






struct rtlsdr_dongle {
	u16int vid;
	u16int pid;
	const char *name;
};





enum usb_reg {
	USB_SYSCTL			= 0x2000,
	USB_CTRL			= 0x2010,
	USB_STAT			= 0x2014,
	USB_EPA_CFG			= 0x2144,
	USB_EPA_CTL			= 0x2148,
	USB_EPA_MAXPKT		= 0x2158,
	USB_EPA_MAXPKT_2	= 0x215a,
	USB_EPA_FIFO_CFG	= 0x2160,
};

enum sys_reg {
	DEMOD_CTL		= 0x3000,
	GPO				= 0x3001,
	GPI				= 0x3002,
	GPOE			= 0x3003,
	GPD				= 0x3004,
	SYSINTE			= 0x3005,
	SYSINTS			= 0x3006,
	GP_CFG0			= 0x3007,
	GP_CFG1			= 0x3008,
	SYSINTE_1		= 0x3009,
	SYSINTS_1		= 0x300a,
	DEMOD_CTL_1		= 0x300b,
	IR_SUSPEND		= 0x300c,
};

enum blocks {
	DEMODB			= 0,
	USBB			= 1,
	SYSB			= 2,
	TUNB			= 3,
	ROMB			= 4,
	IRB				= 5,
	IICB			= 6,
};







/* tuner r82xx */

#define R820T_I2C_ADDR		0x34
#define R828D_I2C_ADDR		0x74
#define R828D_XTAL_FREQ		16000000

#define R82XX_CHECK_ADDR	0x00
#define R82XX_CHECK_VAL		0x69

#define R82XX_IF_FREQ		3570000

#define REG_SHADOW_START	5
#define NUM_REGS		30
#define NUM_IMR			5
#define IMR_TRIAL		9

#define VER_NUM			49

enum r82xx_chip {
	CHIP_R820T,
	CHIP_R620D,
	CHIP_R828D,
	CHIP_R828,
	CHIP_R828S,
	CHIP_R820C,
};

enum r82xx_tuner_type {
	TUNER_RADIO = 1,
	TUNER_ANALOG_TV,
	TUNER_DIGITAL_TV
};

enum r82xx_xtal_cap_value {
	XTAL_LOW_CAP_30P = 0,
	XTAL_LOW_CAP_20P,
	XTAL_LOW_CAP_10P,
	XTAL_LOW_CAP_0P,
	XTAL_HIGH_CAP_0P
};

typedef struct r82xx_config r82xx_config;
typedef struct r82xx_priv r82xx_priv;
typedef struct r82xx_freq_range r82xx_freq_range;

struct r82xx_config {
	u8int i2c_addr;
	u32int xtal;
	enum r82xx_chip rafael_chip;
	unsigned int max_i2c_msg_len;
	int use_predetect;
};

struct r82xx_priv {
	struct r82xx_config		*cfg;

	u8int				regs[NUM_REGS];
	u8int				buf[NUM_REGS + 1];
	enum r82xx_xtal_cap_value	xtal_cap_sel;
	u16int			pll;	/* kHz */
	u32int			int_freq;
	u8int				fil_cal_code;
	u8int				input;
	int				has_lock;
	int				init_done;

	/* Store current mode */
	u32int			delsys;
	enum r82xx_tuner_type		type;

	u32int			bw;	/* in MHz */

	void *rtl_dev;
};

struct r82xx_freq_range {
	u32int	freq;
	u8int		open_d;
	u8int		rf_mux_ploy;
	u8int		tf_c;
	u8int		xtal_cap20p;
	u8int		xtal_cap10p;
	u8int		xtal_cap0p;
};

enum r82xx_delivery_system {
	SYS_UNDEFINED,
	SYS_DVBT,
	SYS_DVBT2,
	SYS_ISDBT,
};


/* Those initial values start from REG_SHADOW_START */
static const u8int r82xx_init_array[NUM_REGS] = {
	0x83, 0x32, 0x75,			/* 05 to 07 */
	0xc0, 0x40, 0xd6, 0x6c,			/* 08 to 0b */
	0xf5, 0x63, 0x75, 0x68,			/* 0c to 0f */
	0x6c, 0x83, 0x80, 0x00,			/* 10 to 13 */
	0x0f, 0x00, 0xc0, 0x30,			/* 14 to 17 */
	0x48, 0xcc, 0x60, 0x00,			/* 18 to 1b */
	0x54, 0xae, 0x4a, 0xc0			/* 1c to 1f */
};

/* Tuner frequency ranges */
static const struct r82xx_freq_range freq_ranges[] = {
	{
	/* .freq = */		0,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0xdf,	/* R27[7:0]  band2,band0 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		50,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0xbe,	/* R27[7:0]  band4,band1  */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		55,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x8b,	/* R27[7:0]  band7,band4 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		60,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x7b,	/* R27[7:0]  band8,band4 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		65,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x69,	/* R27[7:0]  band9,band6 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		70,	/* Start freq, in MHz */
	/* .open_d = */		0x08,	/* low */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x58,	/* R27[7:0]  band10,band7 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		75,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x44,	/* R27[7:0]  band11,band11 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		80,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x44,	/* R27[7:0]  band11,band11 */
	/* .xtal_cap20p = */	0x02,	/* R16[1:0]  20pF (10)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		90,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x34,	/* R27[7:0]  band12,band11 */
	/* .xtal_cap20p = */	0x01,	/* R16[1:0]  10pF (01)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		100,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x34,	/* R27[7:0]  band12,band11 */
	/* .xtal_cap20p = */	0x01,	/* R16[1:0]  10pF (01)    */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		110,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x24,	/* R27[7:0]  band13,band11 */
	/* .xtal_cap20p = */	0x01,	/* R16[1:0]  10pF (01)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		120,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x24,	/* R27[7:0]  band13,band11 */
	/* .xtal_cap20p = */	0x01,	/* R16[1:0]  10pF (01)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		140,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x14,	/* R27[7:0]  band14,band11 */
	/* .xtal_cap20p = */	0x01,	/* R16[1:0]  10pF (01)   */
	/* .xtal_cap10p = */	0x01,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		180,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x13,	/* R27[7:0]  band14,band12 */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		220,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x13,	/* R27[7:0]  band14,band12 */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		250,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x11,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		280,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x02,	/* R26[7:6]=0 (LPF)  R26[1:0]=2 (low) */
	/* .tf_c = */		0x00,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		310,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x41,	/* R26[7:6]=1 (bypass)  R26[1:0]=1 (middle) */
	/* .tf_c = */		0x00,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		450,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x41,	/* R26[7:6]=1 (bypass)  R26[1:0]=1 (middle) */
	/* .tf_c = */		0x00,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		588,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x40,	/* R26[7:6]=1 (bypass)  R26[1:0]=0 (highest) */
	/* .tf_c = */		0x00,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}, {
	/* .freq = */		650,	/* Start freq, in MHz */
	/* .open_d = */		0x00,	/* high */
	/* .rf_mux_ploy = */	0x40,	/* R26[7:6]=1 (bypass)  R26[1:0]=0 (highest) */
	/* .tf_c = */		0x00,	/* R27[7:0]  highest,highest */
	/* .xtal_cap20p = */	0x00,	/* R16[1:0]  0pF (00)   */
	/* .xtal_cap10p = */	0x00,
	/* .xtal_cap0p = */	0x00,
	}
};

static int r82xx_xtal_capacitor[][2] = {
	{ 0x0b, XTAL_LOW_CAP_30P },
	{ 0x02, XTAL_LOW_CAP_20P },
	{ 0x01, XTAL_LOW_CAP_10P },
	{ 0x00, XTAL_LOW_CAP_0P  },
	{ 0x10, XTAL_HIGH_CAP_0P },
};


/* reg field */


enum cmd_op {
	CMD_OP_GET	= (1 << 0),
	CMD_OP_SET	= (1 << 1),
	CMD_OP_EXEC	= (1 << 2),
};

enum pstate {
	ST_IN_CMD,
	ST_IN_ARG,
};

struct strbuf {
	u8int idx;
	char buf[32];
};

struct cmd_state {
	struct strbuf cmd;
	struct strbuf arg;
	enum pstate state;
	void (*out)(const char *format, va_list ap);
};

struct cmd {
	const char *cmd;
	u32int ops;
	int (*cb)(struct cmd_state *cs, enum cmd_op op, const char *cmd,
		  int argc, char **argv);
	const char *help;
};

/* structure describing a field in a register */
struct reg_field {
	u8int reg;
	u8int shift;
	u8int width;
};

struct reg_field_ops {
	const struct reg_field *fields;
	const char **field_names;
	u32int num_fields;
	void *data;
	int (*write_cb)(void *data, u32int reg, u32int val);
	u32int (*read_cb)(void *data, u32int reg);
};



/* tuner e4k */

#define E4K_I2C_ADDR	0xc8
#define E4K_CHECK_ADDR	0x02
#define E4K_CHECK_VAL	0x40

enum e4k_reg {
	E4K_REG_MASTER1		= 0x00,
	E4K_REG_MASTER2		= 0x01,
	E4K_REG_MASTER3		= 0x02,
	E4K_REG_MASTER4		= 0x03,
	E4K_REG_MASTER5		= 0x04,
	E4K_REG_CLK_INP		= 0x05,
	E4K_REG_REF_CLK		= 0x06,
	E4K_REG_SYNTH1		= 0x07,
	E4K_REG_SYNTH2		= 0x08,
	E4K_REG_SYNTH3		= 0x09,
	E4K_REG_SYNTH4		= 0x0a,
	E4K_REG_SYNTH5		= 0x0b,
	E4K_REG_SYNTH6		= 0x0c,
	E4K_REG_SYNTH7		= 0x0d,
	E4K_REG_SYNTH8		= 0x0e,
	E4K_REG_SYNTH9		= 0x0f,
	E4K_REG_FILT1		= 0x10,
	E4K_REG_FILT2		= 0x11,
	E4K_REG_FILT3		= 0x12,
	// gap
	E4K_REG_GAIN1		= 0x14,
	E4K_REG_GAIN2		= 0x15,
	E4K_REG_GAIN3		= 0x16,
	E4K_REG_GAIN4		= 0x17,
	// gap
	E4K_REG_AGC1		= 0x1a,
	E4K_REG_AGC2		= 0x1b,
	E4K_REG_AGC3		= 0x1c,
	E4K_REG_AGC4		= 0x1d,
	E4K_REG_AGC5		= 0x1e,
	E4K_REG_AGC6		= 0x1f,
	E4K_REG_AGC7		= 0x20,
	E4K_REG_AGC8		= 0x21,
	// gap
	E4K_REG_AGC11		= 0x24,
	E4K_REG_AGC12		= 0x25,
	// gap
	E4K_REG_DC1		= 0x29,
	E4K_REG_DC2		= 0x2a,
	E4K_REG_DC3		= 0x2b,
	E4K_REG_DC4		= 0x2c,
	E4K_REG_DC5		= 0x2d,
	E4K_REG_DC6		= 0x2e,
	E4K_REG_DC7		= 0x2f,
	E4K_REG_DC8		= 0x30,
	// gap
	E4K_REG_QLUT0		= 0x50,
	E4K_REG_QLUT1		= 0x51,
	E4K_REG_QLUT2		= 0x52,
	E4K_REG_QLUT3		= 0x53,
	// gap
	E4K_REG_ILUT0		= 0x60,
	E4K_REG_ILUT1		= 0x61,
	E4K_REG_ILUT2		= 0x62,
	E4K_REG_ILUT3		= 0x63,
	// gap
	E4K_REG_DCTIME1		= 0x70,
	E4K_REG_DCTIME2		= 0x71,
	E4K_REG_DCTIME3		= 0x72,
	E4K_REG_DCTIME4		= 0x73,
	E4K_REG_PWM1		= 0x74,
	E4K_REG_PWM2		= 0x75,
	E4K_REG_PWM3		= 0x76,
	E4K_REG_PWM4		= 0x77,
	E4K_REG_BIAS		= 0x78,
	E4K_REG_CLKOUT_PWDN	= 0x7a,
	E4K_REG_CHFILT_CALIB	= 0x7b,
	E4K_REG_I2C_REG_ADDR	= 0x7d,
	// FIXME
};

#define E4K_MASTER1_RESET	(1 << 0)
#define E4K_MASTER1_NORM_STBY	(1 << 1)
#define E4K_MASTER1_POR_DET	(1 << 2)

#define E4K_SYNTH1_PLL_LOCK	(1 << 0)
#define E4K_SYNTH1_BAND_SHIF	1

#define E4K_SYNTH7_3PHASE_EN	(1 << 3)

#define E4K_SYNTH8_VCOCAL_UPD	(1 << 2)

#define E4K_FILT3_DISABLE	(1 << 5)

#define E4K_AGC1_LIN_MODE	(1 << 4)
#define E4K_AGC1_LNA_UPDATE	(1 << 5)
#define E4K_AGC1_LNA_G_LOW	(1 << 6)
#define E4K_AGC1_LNA_G_HIGH	(1 << 7)

#define E4K_AGC6_LNA_CAL_REQ	(1 << 4)

#define E4K_AGC7_MIX_GAIN_AUTO	(1 << 0)
#define E4K_AGC7_GAIN_STEP_5dB	(1 << 5)

#define E4K_AGC8_SENS_LIN_AUTO	(1 << 0)

#define E4K_AGC11_LNA_GAIN_ENH	(1 << 0)

#define E4K_DC1_CAL_REQ		(1 << 0)

#define E4K_DC5_I_LUT_EN	(1 << 0)
#define E4K_DC5_Q_LUT_EN	(1 << 1)
#define E4K_DC5_RANGE_DET_EN	(1 << 2)
#define E4K_DC5_RANGE_EN	(1 << 3)
#define E4K_DC5_TIMEVAR_EN	(1 << 4)

#define E4K_CLKOUT_DISABLE	0x96

#define E4K_CHFCALIB_CMD	(1 << 0)

#define E4K_AGC1_MOD_MASK	0xF

enum e4k_agc_mode {
	E4K_AGC_MOD_SERIAL		= 0x0,
	E4K_AGC_MOD_IF_PWM_LNA_SERIAL	= 0x1,
	E4K_AGC_MOD_IF_PWM_LNA_AUTONL	= 0x2,
	E4K_AGC_MOD_IF_PWM_LNA_SUPERV	= 0x3,
	E4K_AGC_MOD_IF_SERIAL_LNA_PWM	= 0x4,
	E4K_AGC_MOD_IF_PWM_LNA_PWM	= 0x5,
	E4K_AGC_MOD_IF_DIG_LNA_SERIAL	= 0x6,
	E4K_AGC_MOD_IF_DIG_LNA_AUTON	= 0x7,
	E4K_AGC_MOD_IF_DIG_LNA_SUPERV	= 0x8,
	E4K_AGC_MOD_IF_SERIAL_LNA_AUTON	= 0x9,
	E4K_AGC_MOD_IF_SERIAL_LNA_SUPERV = 0xa,
};

enum e4k_band {
	E4K_BAND_VHF2	= 0,
	E4K_BAND_VHF3	= 1,
	E4K_BAND_UHF	= 2,
	E4K_BAND_L	= 3,
};

enum e4k_mixer_filter_bw {
	E4K_F_MIX_BW_27M	= 0,
	E4K_F_MIX_BW_4M6	= 8,
	E4K_F_MIX_BW_4M2	= 9,
	E4K_F_MIX_BW_3M8	= 10,
	E4K_F_MIX_BW_3M4	= 11,
	E4K_F_MIX_BW_3M		= 12,
	E4K_F_MIX_BW_2M7	= 13,
	E4K_F_MIX_BW_2M3	= 14,
	E4K_F_MIX_BW_1M9	= 15,
};

enum e4k_if_filter {
	E4K_IF_FILTER_MIX,
	E4K_IF_FILTER_CHAN,
	E4K_IF_FILTER_RC
};
struct e4k_pll_params {
	u32int fosc;
	u32int intended_flo;
	u32int flo;
	u16int x;
	u8int z;
	u8int r;
	u8int r_idx;
	u8int threephase;
};

struct e4k_state {
	void *i2c_dev;
	u8int i2c_addr;
	enum e4k_band band;
	struct e4k_pll_params vco;
	void *rtl_dev;
};


/* tuner fc001x common, swiped from linux source code */

enum fc001x_xtal_freq {
	FC_XTAL_27_MHZ,		/* 27000000 */
	FC_XTAL_28_8_MHZ,	/* 28800000 */
	FC_XTAL_36_MHZ,		/* 36000000 */
};



/* tuner fc0012 */

#define FC0012_I2C_ADDR		0xc6
#define FC0012_CHECK_ADDR	0x00
#define FC0012_CHECK_VAL	0xa1


/* tuner fc0013 */

#define FC0013_I2C_ADDR		0xc6
#define FC0013_CHECK_ADDR	0x00
#define FC0013_CHECK_VAL	0xa3


/* tuner fc2580 */

#define	BORDER_FREQ	2600000	//2.6GHz : The border frequency which determines whether Low VCO or High VCO is used
#define USE_EXT_CLK	0	//0 : Use internal XTAL Oscillator / 1 : Use External Clock input
#define OFS_RSSI 57

#define FC2580_I2C_ADDR		0xac
#define FC2580_CHECK_ADDR	0x01
#define FC2580_CHECK_VAL	0x56

typedef enum {
	FC2580_UHF_BAND,
	FC2580_L_BAND,
	FC2580_VHF_BAND,
	FC2580_NO_BAND
} fc2580_band_type;

typedef enum {
	FC2580_FCI_FAIL,
	FC2580_FCI_SUCCESS
} fc2580_fci_result_type;

enum FUNCTION_STATUS
{
	FUNCTION_SUCCESS,
	FUNCTION_ERROR,
};

// The following context is FC2580 tuner API source code
// Definitions

// AGC mode
enum FC2580_AGC_MODE
{
	FC2580_AGC_INTERNAL = 1,
	FC2580_AGC_EXTERNAL = 2,
};


// Bandwidth mode
enum FC2580_BANDWIDTH_MODE
{
	FC2580_BANDWIDTH_1530000HZ = 1,
	FC2580_BANDWIDTH_6000000HZ = 6,
	FC2580_BANDWIDTH_7000000HZ = 7,
	FC2580_BANDWIDTH_8000000HZ = 8,
};




/* the big rtlsdr struct */


struct rtlsdr_dev {
//	libusb_context *ctx;
//	struct libusb_device_handle *devh;
	Dev *usbd;
	Dev *readradio;
	u32int xfer_buf_num;
	u32int xfer_buf_len;
//	struct libusb_transfer **xfer;
	unsigned char **xfer_buf;
//	rtlsdr_read_async_cb_t cb;
	void *cb_ctx;
//	enum rtlsdr_async_status async_status;
	int async_cancel;
	int use_zerocopy;
	/* rtl demod context */
	u32int rate; /* Hz */
	u32int rtl_xtal; /* Hz */
	int fir[FIR_LEN];
	int direct_sampling;
	/* tuner context */
	enum rtlsdr_tuner tuner_type;
	rtlsdr_tuner_iface_t *tuner;
	u32int tun_xtal; /* Hz */
	u32int freq; /* Hz */
	u32int bw;
	u32int offs_freq; /* Hz */
	int corr; /* ppm */
	int gain; /* tenth dB */
	struct e4k_state e4k_s;
	struct r82xx_config r82xx_c;
	struct r82xx_priv r82xx_p;
	/* status */
	int dev_lost;
	int driver_active;
	unsigned int xfer_errors;
};


/* rtlfm.c */

#define DEFAULT_SAMPLE_RATE		24000
//#define DEFAULT_BUF_LENGTH		(1 * 16384)
#define MAXIMUM_OVERSAMPLE		16
#define MAXIMUM_BUF_LENGTH		(MAXIMUM_OVERSAMPLE * DEFAULT_BUF_LENGTH)
#define AUTO_GAIN			-100
#define BUFFER_DUMP			4096

#define FREQUENCIES_LIMIT		1000

typedef struct dongle_state dongle_state;
typedef struct demod_state demod_state;
typedef struct output_state output_state;
typedef struct controller_state controller_state;


struct dongle_state
{
	int      exit_flag;
//	pthread_t thread;
	rtlsdr_dev_t *dev;
	int      dev_index;
	u32int freq;
	u32int rate;
	int      gain;
	u16int buf16[MAXIMUM_BUF_LENGTH];
	u32int buf_len;
	int      ppm_error;
	int      offset_tuning;
	int      direct_sampling;
	int      mute;
	struct demod_state *demod_target;
};

struct demod_state
{
	int      exit_flag;
//	pthread_t thread;
	s16int  lowpassed[MAXIMUM_BUF_LENGTH];
	int      lp_len;
	s16int  lp_i_hist[10][6];
	s16int  lp_q_hist[10][6];
	s16int  result[MAXIMUM_BUF_LENGTH];
	s16int  droop_i_hist[9];
	s16int  droop_q_hist[9];
	int      result_len;
	int      rate_in;
	int      rate_out;
	int      rate_out2;
	int      now_r, now_j;
	int      pre_r, pre_j;
	int      prev_index;
	int      downsample;    /* min 1, max 256 */
	int      post_downsample;
	int      output_scale;
	int      squelch_level, conseq_squelch, squelch_hits, terminate_on_squelch;
	int      downsample_passes;
	int      comp_fir_size;
	int      custom_atan;
	int      deemph, deemph_a;
	int      now_lpr;
	int      prev_lpr_index;
	int      dc_block, dc_avg;
	void     (*mode_demod)(struct demod_state*);
//	pthread_rwlock_t rw;
//	pthread_cond_t ready;
//	pthread_mutex_t ready_m;
	struct output_state *output_target;
};

struct output_state
{
	int      exit_flag;
//	pthread_t thread;
//	FILE     *file;
	char     *filename;
	s16int  result[MAXIMUM_BUF_LENGTH];
	int      result_len;
	int      rate;
//	pthread_rwlock_t rw;
//	pthread_cond_t ready;
//	pthread_mutex_t ready_m;
};

struct controller_state
{
	int      exit_flag;
//	pthread_t thread;
	u32int freqs[FREQUENCIES_LIMIT];
	int      freq_len;
	int      freq_now;
	int      edge;
	int      wb_mode;
//	pthread_cond_t hop;
//	pthread_mutex_t hop_m;
};
