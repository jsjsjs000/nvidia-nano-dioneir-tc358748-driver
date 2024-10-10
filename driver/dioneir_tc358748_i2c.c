// SPDX-License-Identifier: GPL-2.0

#include <linux/delay.h>
// #include <linux/math.h>
#include <linux/media-bus-format.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <media/tegracam_core.h>

	/* for debug only */
// #define DEBUG_MODE_COLOR_BAR

#define TAG "tc358748: "

#ifndef UNUSED
	#define UNUSED  __attribute__((__unused__))
#endif

#define CHIPID          0x0000
#define SYSCTL          0x0002
#define CONFCTL         0x0004
#define FIFOCTL         0x0006
#define DATAFMT         0x0008
#define PLLCTL0         0x0016
#define PLLCTL1         0x0018
#define CLKCTL          0x0020
#define WORDCNT         0x0022
#define PP_MISC         0x0032
#define CLW_DPHYCONTTX  0x0100
#define D0W_DPHYCONTTX  0x0104
#define D1W_DPHYCONTTX  0x0108
#define D2W_DPHYCONTTX  0x010c
#define D3W_DPHYCONTTX  0x0110
#define CLW_CNTRL       0x0140
#define D0W_CNTRL       0x0144
#define D1W_CNTRL       0x0148
#define D2W_CNTRL       0x014c
#define D3W_CNTRL       0x0150
#define STARTCNTRL      0x0204
#define PPISTATUS       0x0208
#define LINEINITCNT     0x0210
#define LPTXTIMECNT     0x0214
#define TCLK_HEADERCNT  0x0218
#define TCLK_TRAILCNT   0x021C
#define THS_HEADERCNT   0x0220
#define TWAKEUP         0x0224
#define TCLK_POSTCNT    0x0228
#define THS_TRAILCNT    0x022C
#define HSTXVREGCNT     0x0230
#define HSTXVREGEN      0x0234
#define TXOPTIONCNTRL   0x0238
#define CSI_CONFW       0x0500
#define CSI_RESET       0x0504
#define CSI_START       0x0518

#define DBG_LCNT        0x00E0
#define DBG_WIDTH       0x00E2
#define DBG_VBLANK      0x00E4
#define DBG_DATA        0x00E8

/* Values used in the CSI_CONFW register */
#define CSI_SET_REGISTER	(5 << 29)
#define CSI_CLR_REGISTER	(6 << 29)
#define CSI_CONTROL_REG		(3 << 24)

// struct i2c_client *tc358748_i2c_client = NULL;
// struct regmap *ctl_regmap_client = NULL;

// static bool i2c_read(struct regmap *ctl_regmap, u8 *send, u16 send_size,
// 		u8 *receive, u16 receive_size)
// {
// 	int result = 0;

// 	if (!client)
// 	{
// 		pr_err(TAG "I2C client = null");
// 		return false;
// 	}

// 	result = i2c_master_send(client, send, send_size);
// 	if (result < 0)
// 	{
// 		pr_err(TAG "Failed to send I2C the data.\n"); // %.2x
// 		return false;
// 	}

// 	result = i2c_master_recv(client, receive, receive_size);
// 	if (result < 0)
// 	{
// 		pr_err(TAG "Failed to receive I2C the data.\n");
// 		return false;
// 	}

// 	return true;
// }

// static bool i2c_write(struct regmap *ctl_regmap, u8 *send, u16 send_size)
// {
// 	int result = 0;

// 	if (!client)
// 	{
// 		pr_err(TAG "I2C client = null");
// 		return false;
// 	}

// 	result = i2c_master_send(client, send, send_size);
// 	if (result < 0)
// 	{
// 		pr_err(TAG "Failed to send I2C the data.\n"); // %.2x
// 		return false;
// 	}

// 	return true;
// }

// static bool i2c_write_reg16(struct regmap *ctl_regmap, u16 reg, u16 value)
// {
// 	u8 send[4] = { reg >> 8, reg & 0xff, value >> 8, value & 0xff };
// 	return i2c_write(client, send, sizeof(send));
// }

// static bool i2c_read_reg16(struct regmap *ctl_regmap, u16 reg, u16 *ret_value)
// {
// 	u8 send[2] = { reg >> 8, reg & 0xff };
// 	u8 receive[2] = { 0 };
// 	if (!i2c_read(client, send, sizeof(send), receive, sizeof(receive)))
// 		return false;
	
// 	*ret_value = (receive[0] << 8) | receive[1];
// 	return true;
// }

// static bool i2c_write_reg32(struct regmap *ctl_regmap, u16 reg, u32 value)
// {
// 	u8 send[2 + 4] = { reg >> 8, reg & 0xff,
// 			(value >> 8) & 0xff, value & 0xff, (value >> 24) & 0xff, (value >> 16) & 0xff };
// 	return i2c_write(client, send, sizeof(send));
// }

// static bool i2c_read_reg32(struct regmap *ctl_regmap, u16 reg, u32 *ret_value)
// {
// 	u8 send[2] = { reg >> 8, reg & 0xff };
// 	u8 receive[4] = { 0 };
// 	if (!i2c_read(client, send, sizeof(send), receive, sizeof(receive)))
// 		return false;
	
// 	*ret_value = (receive[2] << 24) | (receive[3] << 16) | (receive[0] << 8) | receive[1];
// 	return true;
// }

static unsigned int clk_count(u64 rate, unsigned int ns)
{
	rate *= ns;
	if (do_div(rate, 1000000000))
		rate++; /* Round up the count */
	return rate;
}

static unsigned int clk_ns(unsigned long rate, u64 count)
{
	count *= 1000000000u;
	if (do_div(count, rate))
		count++; /* Round up the time */
	return count;
}

	/* setup PLL in Toshiba TC358748 by I2C */
static bool tc358748_set_pll(struct regmap *ctl_regmap, struct regmap *csi_regmap)
{
/*
		CSI clock dla PCLK = 12'676'060 Hz
	Pixel clock:           800 * 525 * 30,181095238 = 12'676'060 Hz
	Bandwich:              12'676'060 * 24 = 304'225'440 bps
	Data Rate Per Line:    304'225'440 / 4 = 76'056'360 bps
	MIPI D-PHY Clock Rate: 76'056'360 / 2 = 38'028'180 Hz    # / 2 - Double Data Rate (?)

	CSITxClk is obtained by dividing pll_clk by 2.
	Pll_clk = CSITxClk * 2
	Pll_clk = 38'028'180 * 2 = 76'056'360 Hz   # dla DDR - Double Data Rate
	Pll_clk = 76'056'360 * 2 = 152'112'720 Hz  # bez DDR

	In CSI 2 Tx mode, RefClk can be tie to ground.
	In this case, PClk / 4 will be used to drive PLL, Figure 5-3.
	REFCLK = 12'676'060 / 4 = 3'169'015 Hz

	REFCLK * ((FBD + 1) / (PRD + 1)) * (1 / (2 ^ FRS)) = Pll_clk
	3'169'015 * ((383 + 1) / (1 + 1)) * (1 / (2 ^ 3)) =  76'056'360  #    with DDR 4 lanes
	3'169'015 * ((383 + 1) / (1 + 1)) * (1 / (2 ^ 2)) = 152'112'720  # without DDR 4 lanes
	3'169'015 * ((383 + 1) / (1 + 1)) * (1 / (2 ^ 1)) = 304'225'440  #    with DDR 1 lane
	3'169'015 * ((383 + 1) / (1 + 1)) * (1 / (2 ^ 0)) = 608'450'880  # without DDR 1 lane
	see 'Toshiba PLL calculation.ods'

	For FRS (HSCK = pll_clk):
	Frequency range setting (post divider) for HSCK frequency
	2’b00: 500MHz – 1GHz HSCK frequency
	2’b01: 250MHz – 500MHz HSCK frequency
	2’b10: 125 MHz – 250MHz HSCK frequency
	2’b11: 62.5MHz – 125MHz HSCK frequency

	example in datasheet:
	16600000 * (255 + 1) / (7 + 1) * (1 / (2 ^ 1)) = 265600000
	16600000 * (319 + 1) / (5 + 1) * (1 / (2 ^ 2)) = 221333333
*/

	const u16 fbd = 383;
	const u8 prd = 1;

	// const u8 frs = 3;      // Pll_clk =  76'056'360    with DDR 4 lanes
	const u8 frs = 2;      // Pll_clk = 152'112'720 without DDR 4 lanes  $$
	// const u8 frs = 1;         // Pll_clk = 304'225'440    with DDR 1 lane   $$
	                          // CSI TX Clk = Pll_clk / 2
	const u8 sclk_div = frs > 2 ? 2 : frs;
	const u8 clk_div = sclk_div;

	u16 pllctl0;
	u16 pllctl1;
	u16 clkctl;

		/* Setup PLL divider */
	pllctl0 = (prd << 12) | fbd;
	if (regmap_write(ctl_regmap, PLLCTL0, pllctl0))
	{
		pr_err(TAG "Can't write PLLCTL0");
		return false;
	}
	pr_info(TAG "PLLCTL0 (0x%04x) = 0x%04x - Setup PLL divider", PLLCTL0, pllctl0);

		/* Start PLL */
	pllctl1 = (frs << 10) |
			(2 << 8) |               /* loop bandwidth 50% */
			(1 << 1) |               /* PLL not reset */
			(1 << 0);                /* PLL enable */
	if (regmap_write(ctl_regmap, PLLCTL1, pllctl1))
	{
		pr_err(TAG "Can't write PLLCTL1");
		return false;
	}
	pr_info(TAG "PLLCTL1 (0x%04x) = 0x%04x - Start PLL", PLLCTL1, pllctl1);

		/* Wait for PLL to lock */
	usleep_range(20, 20);
	pr_info(TAG "Wait 20us for PLL to lock");

		/* Clocks dividers */
	clkctl = (clk_div << 4) | (clk_div << 2) | sclk_div;
	if (regmap_write(ctl_regmap, CLKCTL, clkctl))
	{
		pr_err(TAG "Can't write CLKCTL");
		return false;
	}
	pr_info(TAG "CLKCTL (0x%04x) = 0x%04x - Setup PLL divider", CLKCTL, clkctl);

		/* Turn on clocks */
	pllctl1 |= 1 << 4;
	if (regmap_write(ctl_regmap, PLLCTL1, pllctl1))
	{
		pr_err(TAG "Can't write PLLCTL1");
		return false;
	}
	pr_info(TAG "PLLCTL1 (0x%04x) = 0x%04x - Turn on clocks", PLLCTL1, pllctl1);

	return true;
}

// static void tc358746_setup(struct regmap *ctl_regmap)
// {
// 	struct phy_configure_opts_mipi_dphy *cfg = &tc358746->dphy_cfg;
// 	bool non_cont_clk = !!(tc358746->csi_vep.bus.mipi_csi2.flags &
// 			       V4L2_MBUS_CSI2_NONCONTINUOUS_CLOCK);
// 	struct device *dev = tc358746->sd.dev;
// 	unsigned long hs_byte_clk, hf_clk;
// 	u32 val, val2, lptxcnt;
// 	int err;

// 	/* The hs_byte_clk is also called SYSCLK in the excel sheet */
// 	hs_byte_clk = cfg->hs_clk_rate / 8;
// 	hs_byte_clk /= HZ_PER_MHZ;
// 	hf_clk = hs_byte_clk / 2;

// 	val = tc358746_us_to_cnt(cfg->init, hf_clk) - 1;
// 	dev_dbg(dev, "LINEINITCNT: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, LINEINITCNT_REG, val);
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->lpx, hs_byte_clk) - 1;
// 	lptxcnt = val;
// 	dev_dbg(dev, "LPTXTIMECNT: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, LPTXTIMECNT_REG, val);
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->clk_prepare, hs_byte_clk) - 1;
// 	val2 = tc358746_ps_to_cnt(cfg->clk_zero, hs_byte_clk) - 1;
// 	dev_dbg(dev, "TCLK_PREPARECNT: %u (0x%x)\n", val, val);
// 	dev_dbg(dev, "TCLK_ZEROCNT: %u (0x%x)\n", val2, val2);
// 	dev_dbg(dev, "TCLK_HEADERCNT: 0x%x\n",
// 		(u32)(TCLK_PREPARECNT(val) | TCLK_ZEROCNT(val2)));
// 	err = tc358746_write(tc358746, TCLK_HEADERCNT_REG,
// 			     TCLK_PREPARECNT(val) | TCLK_ZEROCNT(val2));
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->clk_trail, hs_byte_clk);
// 	dev_dbg(dev, "TCLK_TRAILCNT: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, TCLK_TRAILCNT_REG, val);
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->hs_prepare, hs_byte_clk) - 1;
// 	val2 = tc358746_ps_to_cnt(cfg->hs_zero, hs_byte_clk) - 1;
// 	dev_dbg(dev, "THS_PREPARECNT: %u (0x%x)\n", val, val);
// 	dev_dbg(dev, "THS_ZEROCNT: %u (0x%x)\n", val2, val2);
// 	dev_dbg(dev, "THS_HEADERCNT: 0x%x\n",
// 		(u32)(THS_PREPARECNT(val) | THS_ZEROCNT(val2)));
// 	err = tc358746_write(tc358746, THS_HEADERCNT_REG,
// 			     THS_PREPARECNT(val) | THS_ZEROCNT(val2));
// 	if (err)
// 		return err;

// 	/* TWAKEUP > 1ms in lptxcnt steps */
// 	val = tc358746_us_to_cnt(cfg->wakeup, hs_byte_clk);
// 	val = val / (lptxcnt + 1) - 1;
// 	dev_dbg(dev, "TWAKEUP: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, TWAKEUP_REG, val);
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->clk_post, hs_byte_clk);
// 	dev_dbg(dev, "TCLK_POSTCNT: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, TCLK_POSTCNT_REG, val);
// 	if (err)
// 		return err;

// 	val = tc358746_ps_to_cnt(cfg->hs_trail, hs_byte_clk);
// 	dev_dbg(dev, "THS_TRAILCNT: %u (0x%x)\n", val, val);
// 	err = tc358746_write(tc358746, THS_TRAILCNT_REG, val);
// 	if (err)
// 		return err;
// }



	/* setup Toshiba TC358748 by I2C */
bool tc358748_setup(struct regmap *ctl_regmap, struct regmap *csi_regmap)
{
	UNUSED const u16 width = 640;
	// UNUSED const u16 height = 480;
	// UNUSED const u16 total_width = 800;
	// UNUSED const u16 total_height = 525;
	// UNUSED const u16 h_front_porch = 16;
	// UNUSED const u16 h_sync = 96;
	// UNUSED const u16 h_back_porch = 48;
	// UNUSED const u16 v_front_porch = 10;
	// UNUSED const u16 v_sync = 2;
	// UNUSED const u16 v_back_porch = 33;

	const u8 bpp = 24;  // RGB
	// const u8 bpp = 16;  // YUVx 16
	// const u8 bpp = 8;  // RAW8
	// const u8 num_data_lanes = 4;  // $$
	const u8 num_data_lanes = 2;  // $$
	// const u8 num_data_lanes = 1;
	const u32 pixelclock = 12676060;                      // 800 * 525 * 30,181095238 = 12'676'060
	// const u32 csi_bus = 38028180;                      // 38'028'180 with DDR 4 lanes
	// const u32 csi_bus = 76056360;                      // 76'056'360 without DDR 4 lanes  $$
	// const u32 csi_bus = 152112720;                        // 152'112'720 with DDR 1 lane  $$
	const u32 csi_rate = bpp * pixelclock;                // 304'225'440 bps
	const u32 csi_lane_rate = csi_rate / num_data_lanes;  // 76'056'360 (min 62'500'000, max 1G)

	u32 chip_id;
	u16 confctl;
	u16 fifoctl;
	u16 datafmt;
	u16 wordcnt;

	u32 hsbyte_clk;
	u32 linecnt;
	u32 lptxtime;
	u32 t_wakeup;
	u32 tclk_prepare;
	u32 tclk_zero;
	u32 tclk_trail;
	u32 tclk_post;
	u32 ths_prepare;
	u32 ths_zero;
	u32 ths_trail;

	u32 tclk_headercnt;
	u32 ths_headercnt;
	u32 hstxvregcnt;
	u32 hstxvregen;
	u32 csi_confw;
	u32 continuous_clock_mode;
	// u32 output_current_capacitor;
	UNUSED u32 dbg_cnt;
	UNUSED u32 dbg_width;
	UNUSED u32 dbg_vblank;

	// ctl_regmap_client = ctl_regmap;

	pr_info(TAG "  bpp = %d", bpp);
	pr_info(TAG "  num_data_lanes = %d", num_data_lanes);
	pr_info(TAG "  pixelclock = %u", pixelclock);
	// pr_info(TAG "  csi_bus = %u", csi_bus);
	pr_info(TAG "  csi_rate = %u", csi_rate);
	pr_info(TAG "  csi_lane_rate = %u", csi_lane_rate);

	if (regmap_read(ctl_regmap, CHIPID, &chip_id))
	{
		pr_err(TAG "Can't read ChipId");
		return false;
	}

	if (chip_id != 0x4401)
	{
		pr_err(TAG "Chip not found - ChipId 0x04%x is not 0x4401", chip_id);
		return false;
	}
	pr_info(TAG "ChipId (0x%04x) = 0x%04x - ok", CHIPID, chip_id);

		/* Reset */
	if (regmap_write(ctl_regmap, SYSCTL, 1))
	{
		pr_err(TAG "Can't write SYSCTL");
		return false;
	}
	pr_info(TAG "SYSCTL (0x%04x) = 1 - Reset", SYSCTL);

	msleep(50);
	pr_info(TAG "Wait 50ms");

		/* End of reset */
	if (regmap_write(ctl_regmap, SYSCTL, 0))
	{
		pr_err(TAG "Can't write SYSCTL");
		return false;
	}
	pr_info(TAG "SYSCTL (0x%04x) = 0 - End of reset", SYSCTL);

	msleep(50);
	pr_info(TAG "Wait 50ms");

		/* setup PLL */
	if (!tc358748_set_pll(ctl_regmap, csi_regmap))
	{
		return false;
	}

		/* CONFCTL */
	confctl = num_data_lanes - 1;
	confctl |=
			(1 << 2) |  /* I2C slave index increment */
			// (1 << 3) |  /* Parallel clock polarity inverted */
			// (1 << 4) |  /* H Sync active low */
			// (1 << 5) |  /* V Sync active low */
			(0 << 8);   /* Parallel data format - Mode 0 */
	if (regmap_write(ctl_regmap, CONFCTL, confctl))
	{
		pr_err(TAG "Can't write CONFCTL");
		return false;
	}
	pr_info(TAG "CONFCTL (0x%04x) = 0x%04x", CONFCTL, confctl);



// // regmap_write(ctl_regmap, DATAFMT, 0x60);      // YUV...
// regmap_write(ctl_regmap, DATAFMT, 0x30);   // RGB888
// regmap_write(ctl_regmap, CONFCTL, confctl);
// regmap_write(ctl_regmap, FIFOCTL, 0x20);
// // regmap_write(ctl_regmap, WORDCNT, 0xf00);
// // regmap_write(ctl_regmap, WORDCNT, 640 * 2); // 640 dla YUV...
// regmap_write(ctl_regmap, WORDCNT, 640 * 3); // 640 dla RGB
// regmap_write(ctl_regmap, CSI_RESET, 0);

// confctl |= (1 << 6);                                   /* Parallel port enable */
// regmap_write(ctl_regmap, CONFCTL, confctl);

// regmap_write(ctl_regmap, CSI_START, 1);    /* Start CSI module before writ its registers */
// msleep(10);

// regmap_write(ctl_regmap, CLW_CNTRL, 0x140);
// regmap_write(ctl_regmap, D0W_CNTRL, 0x144);
// regmap_write(ctl_regmap, D1W_CNTRL, 0x148);
// regmap_write(ctl_regmap, D2W_CNTRL, 0x14c);
// regmap_write(ctl_regmap, D3W_CNTRL, 0x150);
// regmap_write(ctl_regmap, LINEINITCNT, 0x15ba);
// regmap_write(ctl_regmap, LPTXTIMECNT, 0x2);
// regmap_write(ctl_regmap, TCLK_HEADERCNT, 0xa03);

// // regmap_write(ctl_regmap, LINEINITCNT, 0x012a);
// // regmap_write(ctl_regmap, LPTXTIMECNT, 0x2);
// // regmap_write(ctl_regmap, TCLK_HEADERCNT, 0x23);

// // regmap_write(ctl_regmap, TCLK_TRAILCNT, 0xffffffff);
// regmap_write(ctl_regmap, TCLK_TRAILCNT, 1);
// // regmap_write(ctl_regmap, TCLK_TRAILCNT, 2);
// // regmap_write(ctl_regmap, THS_HEADERCNT, 0xffffee03);
// regmap_write(ctl_regmap, THS_HEADERCNT, 0x0101);
// // regmap_write(ctl_regmap, THS_HEADERCNT, 0x0101);

// regmap_write(ctl_regmap, TWAKEUP, 0x49e0);
// regmap_write(ctl_regmap, TCLK_POSTCNT, 0x7);
// regmap_write(ctl_regmap, THS_TRAILCNT, 0x1);

// // regmap_write(ctl_regmap, TWAKEUP, 0x02);
// // regmap_write(ctl_regmap, TCLK_POSTCNT, 0x01);
// // regmap_write(ctl_regmap, THS_TRAILCNT, 0x01);

// regmap_write(ctl_regmap, HSTXVREGEN, 0x1f);
// regmap_write(ctl_regmap, STARTCNTRL, 0x1);
// regmap_write(ctl_regmap, CSI_CONFW, 2734719110);
// return true;


// tc358746_setup();
// return true;


		/* FIFOCTL - FiFo level */
	fifoctl = 1;
	// fifoctl = 44;
// fifoctl = 12 * 3; // 48 $$
// fifoctl = 24; // $$ ok
// fifoctl = 32; // $$ ok
// fifoctl = 34; // $$ ok
// fifoctl = 35; // $$ nie ok
// fifoctl = 36; // $$ nie ok
// fifoctl = 40; // $$ nie ok
// fifoctl = 0x20; // for YUV422 $$
	if (regmap_write(ctl_regmap, FIFOCTL, fifoctl))
	{
		pr_err(TAG "Can't write FIFOCTL");
		return false;
	}
	pr_info(TAG "FIFOCTL (0x%04x) = %d - FiFo Level", FIFOCTL, fifoctl);

		/* DATAFMT - Data Format */
	datafmt = (3 << 4);  /* 3 - RGB888 */
// datafmt = (6 << 4);  /* 6 - YUV422 8-bit $$ */
	// datafmt = (0 << 4);  /* 0 - RAW8 */
	if (regmap_write(ctl_regmap, DATAFMT, datafmt))
	{
		pr_err(TAG "Can't write DATAFMT");
		return false;
	}
	pr_info(TAG "DATAFMT (0x%04x) = 0x%04x - Data Format", DATAFMT, datafmt);

		/* WORDCNT */
	wordcnt = width * bpp / 8;
	if (regmap_write(ctl_regmap, WORDCNT, wordcnt))
	{
		pr_err(TAG "Can't write WORDCNT");
		return false;
	}
	pr_info(TAG "WORDCNT (0x%04x) = %d - Word count", WORDCNT, wordcnt);

		/* Parallel port enable */
	confctl |= (1 << 6);
	if (regmap_write(ctl_regmap, CONFCTL, confctl))
	{
		pr_err(TAG "Can't write CONFCTL");
		return false;
	}
	pr_info(TAG "CONFCTL (0x%04x) = 0x%04x", CONFCTL, confctl);

		/* CSI_START */
	if (regmap_write(csi_regmap, CSI_START, 1))
	{
		pr_err(TAG "Can't write CSI_START");
		return false;
	}
	pr_info(TAG "CSI_START (0x%04x) = 1", CSI_START);
	msleep(10);


		/* from driver: https://github.com/avionic-design/linux-l4t/blob/meerkat/l4t-r21-5/drivers/media/i2c/tc358748.c#L574 */
		/* Compute the D-PHY settings */
	hsbyte_clk = csi_lane_rate / 8;

		/* LINEINITCOUNT >= 100us */
	linecnt = clk_count(hsbyte_clk / 2, 100000);

		/* LPTX clk must be less than 20MHz -> LPTXTIMECNT >= 50 ns */
	lptxtime = clk_count(hsbyte_clk, 50);

		/* TWAKEUP >= 1ms (in LPTX clock count) */
	t_wakeup = clk_count(hsbyte_clk / lptxtime, 1000000);

		/* 38ns <= TCLK_PREPARE <= 95ns */
	tclk_prepare = clk_count(hsbyte_clk, 38);
	if (tclk_prepare > clk_count(hsbyte_clk, 95))
		pr_warn(TAG "TCLK_PREPARE is too long (%u ns)\n", clk_ns(hsbyte_clk, tclk_prepare));
	// TODO: Check that TCLK_PREPARE <= 95ns

/*
CLKCTL (0x0020) = 0x002a - Setup PLL divider
PLLCTL1 (0x0018) = 0x0e13 - Turn on clocks
CONFCTL (0x0004) = 0x0007
FIFOCTL (0x0006) = 1 - FiFo Level
DATAFMT (0x0008) = 0x0030 - Data Format
WORDCNT (0x0022) = 1920 - Word count
CONFCTL (0x0004) = 0x0047
CSI_START (0x0518) = 1
  hsbyte_clk = 9507045
  linecnt = 476
  lptxtime = 1
  t_wakeup = 9508
  tclk_prepare = 1
  tclk_zero = 2
  tclk_trail = 1
  tclk_post = 8
  ths_prepare = 1
  ths_zero = 2
  ths_trail = 2
LINEINITCNT (0x0210) = 476
LPTXTIMECNT (0x0214) = 1
TCLK_HEADERCNT (0x0218) = 0x00000201
TCLK_TRAILCNT (0x021c) = 1
THS_HEADERCNT (0x0220) = 0x00000201
TWAKEUP (0x0224) = 9508
TCLK_POSTCNT (0x0228) = 8
THS_TRAILCNT (0x022c) = 2
HSTXVREGCNT (0x0230) = 5
HSTXVREGEN (0x0234) = 0x0000001f
TXOPTIONCNTRL (0x0238) = 1
STARTCNTRL (0x0204) = 1
CSI_CONFW (0x0500) = 0xa3008086
*/

// linecnt = 476
// lptxtime = 1
// t_wakeup = 9508
// tclk_post = 8    // 12 bad px

// linecnt = 1000;
// lptxtime = 0;
// t_wakeup = 1000;
// tclk_post = 16;  // 17 bad px 

// linecnt = 700;
// lptxtime = 2;
// t_wakeup = 10000;
// tclk_post = 2;   // 17

// linecnt = 300;
// lptxtime = 0;
// t_wakeup = 20000;
// tclk_post = 0;  // 16 bad px 

// linecnt = 1000;
// lptxtime = 2;
// t_wakeup = 20000;
// tclk_post = 8;  // popsuty obraz

// linecnt = 2000;
// lptxtime = 3;
// t_wakeup = 20000;
// tclk_post = 8;  // 12 bad px

		/* TCLK_ZERO + TCLK_PREPARE >= 300ns */
	tclk_zero = clk_count(hsbyte_clk, 300) - tclk_prepare;

		/* TCLK_TRAIL >= 60ns */
	tclk_trail = clk_count(hsbyte_clk, 60);

		/* TCLK_POST >= 60ns + 52*UI */
	tclk_post = clk_count(hsbyte_clk, 60 + clk_ns(csi_lane_rate, 52));

		/* 40ns + 4*UI <= THS_PREPARE <= 85ns + 6*UI */
	ths_prepare = clk_count(hsbyte_clk, 40 + clk_ns(csi_lane_rate, 4));
	if (ths_prepare > 85 + clk_ns(csi_lane_rate, 6))
		pr_warn(TAG "THS_PREPARE is too long (%u ns)\n", clk_ns(hsbyte_clk, ths_prepare));

		/* THS_ZERO + THS_PREPARE >= 145ns + 10*UI */
	ths_zero = clk_count(hsbyte_clk, 145 +
			clk_ns(csi_lane_rate, 10)) - ths_prepare;

		/* 105ns + 12*UI > THS_TRAIL >= max(8*UI, 60ns + 4*UI) */
	ths_trail = clk_count(hsbyte_clk,
			max(clk_ns(csi_lane_rate, 8), 60 + clk_ns(csi_lane_rate, 4)));

// tclk_post = 4;
// ths_prepare = 4;
// ths_zero = 4;
// ths_trail = 4;

// 	1 line
// linecnt = 1902;
// lptxtime = 2;
// t_wakeup = 19015;
// tclk_prepare = 2;
// tclk_zero = 10;
// tclk_trail = 3;
// tclk_post = 9;
// ths_prepare = 3;
// ths_zero = 4;
// ths_trail = 3;

// linecnt = 402;
// lptxtime = 1;
// t_wakeup = 49015;
// tclk_prepare = 4;
// tclk_zero = 2;
// tclk_trail = 2;
// tclk_post = 2;
// ths_prepare = 2;
// ths_zero = 2;
// ths_trail = 40;

	pr_info(TAG "  hsbyte_clk = %u", hsbyte_clk);
	pr_info(TAG "  linecnt = %u", linecnt);
	pr_info(TAG "  lptxtime = %u", lptxtime);
	pr_info(TAG "  t_wakeup = %u", t_wakeup);
	pr_info(TAG "  tclk_prepare = %u", tclk_prepare);
	pr_info(TAG "  tclk_zero = %u", tclk_zero);
	pr_info(TAG "  tclk_trail = %u", tclk_trail);
	pr_info(TAG "  tclk_post = %u", tclk_post);
	pr_info(TAG "  ths_prepare = %u", ths_prepare);
	pr_info(TAG "  ths_zero = %u", ths_zero);
	pr_info(TAG "  ths_trail = %u", ths_trail);

		/* Setup D-PHY */
		/* LINEINITCNT */
	if (regmap_write(csi_regmap, LINEINITCNT, linecnt))
	{
		pr_err(TAG "Can't write LINEINITCNT");
		return false;
	}
	pr_info(TAG "LINEINITCNT (0x%04x) = %u", LINEINITCNT, linecnt);

		/* LPTXTIMECNT */
	if (regmap_write(csi_regmap, LPTXTIMECNT, lptxtime))
	{
		pr_err(TAG "Can't write LPTXTIMECNT");
		return false;
	}
	pr_info(TAG "LPTXTIMECNT (0x%04x) = %u", LPTXTIMECNT, lptxtime);

		/* TCLK_HEADERCNT */
	tclk_headercnt = tclk_prepare | (tclk_zero << 8);
	if (regmap_write(csi_regmap, TCLK_HEADERCNT, tclk_headercnt))
	{
		pr_err(TAG "Can't write TCLK_HEADERCNT");
		return false;
	}
	pr_info(TAG "TCLK_HEADERCNT (0x%04x) = 0x%08x", TCLK_HEADERCNT, tclk_headercnt);

		/* TCLK_TRAILCNT */
	if (regmap_write(csi_regmap, TCLK_TRAILCNT, tclk_trail))
	{
		pr_err(TAG "Can't write TCLK_TRAILCNT");
		return false;
	}
	pr_info(TAG "TCLK_TRAILCNT (0x%04x) = %u", TCLK_TRAILCNT, tclk_trail);

		/* THS_HEADERCNT */
	ths_headercnt = ths_prepare | (ths_zero << 8);
	if (regmap_write(csi_regmap, THS_HEADERCNT, ths_headercnt))
	{
		pr_err(TAG "Can't write THS_HEADERCNT");
		return false;
	}
	pr_info(TAG "THS_HEADERCNT (0x%04x) = 0x%08x", THS_HEADERCNT, ths_headercnt);

		/* TWAKEUP */
	if (regmap_write(csi_regmap, TWAKEUP, t_wakeup))
	{
		pr_err(TAG "Can't write TWAKEUP");
		return false;
	}
	pr_info(TAG "TWAKEUP (0x%04x) = %u", TWAKEUP, t_wakeup);

		/* TCLK_POSTCNT */
	if (regmap_write(csi_regmap, TCLK_POSTCNT, tclk_post))
	{
		pr_err(TAG "Can't write TCLK_POSTCNT");
		return false;
	}
	pr_info(TAG "TCLK_POSTCNT (0x%04x) = %u", TCLK_POSTCNT, tclk_post);

		/* THS_TRAILCNT */
	if (regmap_write(csi_regmap, THS_TRAILCNT, ths_trail))
	{
		pr_err(TAG "Can't write THS_TRAILCNT");
		return false;
	}
	pr_info(TAG "THS_TRAILCNT (0x%04x) = %u", THS_TRAILCNT, ths_trail);

		/* HSTXVREGCNT */
	hstxvregcnt = 5;
	if (regmap_write(csi_regmap, HSTXVREGCNT, hstxvregcnt))
	{
		pr_err(TAG "Can't write HSTXVREGCNT");
		return false;
	}
	pr_info(TAG "HSTXVREGCNT (0x%04x) = %u", HSTXVREGCNT, hstxvregcnt);

		/* HSTXVREGEN */
	hstxvregen = (((1 << num_data_lanes) - 1) << 1) | (1 << 0);
	if (regmap_write(csi_regmap, HSTXVREGEN, hstxvregen))
	{
		pr_err(TAG "Can't write HSTXVREGEN");
		return false;
	}
	pr_info(TAG "HSTXVREGEN (0x%04x) = 0x%08x", HSTXVREGEN, hstxvregen);

		/* TXOPTIONCNTRL */
	continuous_clock_mode = 0;
	if (regmap_write(csi_regmap, TXOPTIONCNTRL, continuous_clock_mode))
	{
		pr_err(TAG "Can't write TXOPTIONCNTRL");
		return false;
	}
	pr_info(TAG "TXOPTIONCNTRL (0x%04x) = 1", TXOPTIONCNTRL);


	// output_current_capacitor =
	// 		(0 << 8) |   // 0 - 0pF, 1 - 2.8pF, 2 - 3.2pF, 3.6pF
	// 		(15 << 4) |  // 0-15 - HS clock output delay
	// 		0;           // additional output current: 0 - 0%, 1 - 25%, 2 - 50%, 3 - 75%
	// if (regmap_write(ctl_regmap, CLW_DPHYCONTTX, output_current_capacitor))
	// 	{ pr_err(TAG "Can't write CLW_DPHYCONTTX"); return false; }
	// if (regmap_write(ctl_regmap, D0W_DPHYCONTTX, output_current_capacitor))
	// 	{ pr_err(TAG "Can't write D0W_DPHYCONTTX"); return false; }
	// if (regmap_write(ctl_regmap, D1W_DPHYCONTTX, output_current_capacitor))
	// 	{ pr_err(TAG "Can't write D1W_DPHYCONTTX"); return false; }
	// if (regmap_write(ctl_regmap, D2W_DPHYCONTTX, output_current_capacitor))
	// 	{ pr_err(TAG "Can't write D2W_DPHYCONTTX"); return false; }
	// if (regmap_write(ctl_regmap, D3W_DPHYCONTTX, output_current_capacitor))
	// 	{ pr_err(TAG "Can't write D3W_DPHYCONTTX"); return false; }


		/* Setup the debug output */
#ifdef DEBUG_MODE_COLOR_BAR
		/* DBG_LCNT */
	dbg_cnt = 1 << 14;
	// dbg_cnt = height - 1;
	if (regmap_write(ctl_regmap, DBG_LCNT, dbg_cnt))
	{
		pr_err(TAG "Can't write DBG_LCNT");
		return false;
	}
	pr_info(TAG "DBG_LCNT (0x%04x) = %d", DBG_LCNT, dbg_cnt);

		/* DBG_WIDTH */
	dbg_width = total_width * bpp / 8;
	if (regmap_write(ctl_regmap, DBG_WIDTH, (u16)dbg_width))
	{
		pr_err(TAG "Can't write DBG_WIDTH");
		return false;
	}
	pr_info(TAG "DBG_WIDTH (0x%04x) = %d", DBG_WIDTH, dbg_width);

		/* DBG_VBLANK */
	dbg_vblank = v_sync - 1;
	if (regmap_write(ctl_regmap, DBG_VBLANK, dbg_vblank))
	{
		pr_err(TAG "Can't write DBG_VBLANK");
		return false;
	}
	pr_info(TAG "DBG_VBLANK (0x%04x) = %d", DBG_VBLANK, dbg_vblank);
#endif

		/* STARTCNTRL */
	if (regmap_write(csi_regmap, STARTCNTRL, 1))
	{
		pr_err(TAG "Can't write STARTCNTRL");
		return false;
	}
	pr_info(TAG "STARTCNTRL (0x%04x) = 1", STARTCNTRL);

		/* CSI_CONFW */
	csi_confw = CSI_SET_REGISTER | CSI_CONTROL_REG |
				((num_data_lanes - 1) << 1) |
				(1 << 7) |   /* High-speed mode */
				(1 << 15);   /* CSI mode */
	if (regmap_write(csi_regmap, CSI_CONFW, csi_confw))
	{
		pr_err(TAG "Can't write CSI_CONFW");
		return false;
	}
	pr_info(TAG "CSI_CONFW (0x%04x) = 0x%08x", CSI_CONFW, csi_confw);

	return true;
}

	/* software reset is not working - reset chip by power off */
bool tc358748_stop(struct regmap *ctl_regmap, struct regmap *csi_regmap)
{
	bool ok;
	// regmap_write(ctl_regmap, STARTCNTRL, 0);  // writing 0 is not allowed (datasheet)
	// regmap_write(ctl_regmap, CSI_START, 0);   // writing 0 is not allowed (datasheet)
	ok = regmap_write(csi_regmap, CSI_RESET, 3);
	ok &= regmap_write(ctl_regmap, SYSCTL, 1);
	if (!ok)
		pr_info(TAG "Can't reset TC358748");

	return true;
}
