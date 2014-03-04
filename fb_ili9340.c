#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "fbtft.h"

#define DRVNAME		"fb_ili9340"
#define WIDTH		240
#define HEIGHT		320
#define TXBUFLEN	(4 * PAGE_SIZE)
#define DEFAULT_GAMMA	"1F 1A 18 0A 0F 06 45 87 32 0A 07 02 07 05 00\n" \
			"00 25 27 05 10 09 3A 78 4D 05 18 0D 38 3A 1F"

#define ILI9340_NOP 0x00
#define ILI9340_SWRESET 0x01
#define ILI9340_RDDID 0x04
#define ILI9340_RDDST 0x09

#define ILI9340_SLPIN 0x10
#define ILI9340_SLPOUT 0x11
#define ILI9340_PTLON 0x12
#define ILI9340_NORON 0x13

#define ILI9340_RDMODE 0x0A
#define ILI9340_RDMADCTL 0x0B
#define ILI9340_RDPIXFMT 0x0C
#define ILI9340_RDIMGFMT 0x0A
#define ILI9340_RDSELFDIAG 0x0F

#define ILI9340_INVOFF 0x20
#define ILI9340_INVON 0x21
#define ILI9340_GAMMASET 0x26
#define ILI9340_DISPOFF 0x28
#define ILI9340_DISPON 0x29

#define ILI9340_CASET 0x2A
#define ILI9340_PASET 0x2B
#define ILI9340_RAMWR 0x2C
#define ILI9340_RAMRD 0x2E

#define ILI9340_PTLAR 0x30
#define ILI9340_MADCTL 0x36


#define ILI9340_MADCTL_MY 0x80
#define ILI9340_MADCTL_MX 0x40
#define ILI9340_MADCTL_MV 0x20
#define ILI9340_MADCTL_ML 0x10
#define ILI9340_MADCTL_RGB 0x00
#define ILI9340_MADCTL_BGR 0x08
#define ILI9340_MADCTL_MH 0x04

#define ILI9340_PIXFMT 0x3A

#define ILI9340_FRMCTR1 0xB1
#define ILI9340_FRMCTR2 0xB2
#define ILI9340_FRMCTR3 0xB3
#define ILI9340_INVCTR 0xB4
#define ILI9340_DFUNCTR 0xB6

#define ILI9340_PWCTR1 0xC0
#define ILI9340_PWCTR2 0xC1
#define ILI9340_PWCTR3 0xC2
#define ILI9340_PWCTR4 0xC3
#define ILI9340_PWCTR5 0xC4
#define ILI9340_VMCTR1 0xC5
#define ILI9340_VMCTR2 0xC7

#define ILI9340_RDID1 0xDA
#define ILI9340_RDID2 0xDB
#define ILI9340_RDID3 0xDC
#define ILI9340_RDID4 0xDD

#define ILI9340_GMCTRP1 0xE0
#define ILI9340_GMCTRN1 0xE1


static int init_display(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	par->fbtftops.reset(par);

#if 1
        write_reg(par, 0x01); /* software reset */
        mdelay(5);
        write_reg(par, 0x28); /* display off */

	/* startup sequence taken from Adafruit, registers are undocumented */
	write_reg(par, 0xEF, 0x03, 0x80, 0x02);
	write_reg(par, 0xCF, 0x00, 0xC1, 0x30);
	write_reg(par, 0xED, 0x64, 0x03, 0x12, 0x81);
	write_reg(par, 0xE8, 0x85, 0x00, 0x78);
	write_reg(par, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02);
	write_reg(par, 0xF7, 0x20);
	write_reg(par, 0xEA, 0x00, 0x00);

	/* power control */
	write_reg(par, ILI9340_PWCTR1, 0x23); //VRH[5:0]
	write_reg(par, ILI9340_PWCTR2, 0x10); //SAP[2:0];BT[3:0]

	/* VCM control */
	write_reg(par, ILI9340_VMCTR1, 0x3e, 0x28);
	write_reg(par, ILI9340_VMCTR2, 0x86);
	write_reg(par, ILI9340_MADCTL, ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
	write_reg(par, ILI9340_PIXFMT, 0x55);
        /* ------------frame rate----------------------------------- */
	write_reg(par, ILI9340_FRMCTR1, 0x00, 0x18);

	/* display function control */
	write_reg(par, ILI9340_DFUNCTR, 0x08, 0x82, 0x27);

	/* Gamma function disable */
	write_reg(par, 0xF2, 0x00);

	/* gamma curve selected */
	write_reg(par, ILI9340_GAMMASET, 0x01);

	/* set gamma */
	write_reg(par, ILI9340_GMCTRP1, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
			  0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E,
			  0x09, 0x00);

	/* set gamma */
	write_reg(par, ILI9340_GMCTRN1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
			0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F);
        /* ------------Gamma---------------------------------------- */
        /* write_reg(par, 0xF2, 0x08); */ /* Gamma Function Disable */
        //write_reg(par, 0x26, 0x01);

	/* exit sleep */
	write_reg(par, ILI9340_SLPOUT);

	mdelay(100);

	/* display on */
	write_reg(par, ILI9340_DISPON);

	mdelay(20);

#else
        /* startup sequence for MI0283QT-9A */
        write_reg(par, 0x01); /* software reset */
        mdelay(5);
        write_reg(par, 0x28); /* display off */
        /* --------------------------------------------------------- */
        write_reg(par, 0xCF, 0x00, 0x83, 0x30);
        write_reg(par, 0xED, 0x64, 0x03, 0x12, 0x81);
        write_reg(par, 0xE8, 0x85, 0x01, 0x79);
        write_reg(par, 0xCB, 0x39, 0X2C, 0x00, 0x34, 0x02);
        write_reg(par, 0xF7, 0x20);
        write_reg(par, 0xEA, 0x00, 0x00);
        /* ------------power control-------------------------------- */
        write_reg(par, 0xC0, 0x26);
        write_reg(par, 0xC1, 0x11);
        /* ------------VCOM --------- */
        write_reg(par, 0xC5, 0x35, 0x3E);
        write_reg(par, 0xC7, 0xBE);
        /* ------------memory access control------------------------ */
        write_reg(par, 0x3A, 0x55); /* 16bit pixel */
        /* ------------frame rate----------------------------------- */
        write_reg(par, 0xB1, 0x00, 0x1B);
        /* ------------Gamma---------------------------------------- */
        /* write_reg(par, 0xF2, 0x08); */ /* Gamma Function Disable */
        write_reg(par, 0x26, 0x01);
        /* ------------display-------------------------------------- */
        write_reg(par, 0xB7, 0x07); /* entry mode set */
        write_reg(par, 0xB6, 0x0A, 0x82, 0x27, 0x00);
        write_reg(par, 0x11); /* sleep out */
        mdelay(100);
        write_reg(par, 0x29); /* display on */
        mdelay(20);
#endif
	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe, ye);

	/* Column address set */
	write_reg(par, 0x2A,
		(xs >> 8) & 0xFF, xs & 0xFF, (xe >> 8) & 0xFF, xe & 0xFF);

	/* Row adress set */
	write_reg(par, 0x2B,
		(ys >> 8) & 0xFF, ys & 0xFF, (ye >> 8) & 0xFF, ye & 0xFF);

	/* Memory write */
	write_reg(par, 0x2C);
}

#define MEM_Y   (7) /* MY row address order */
#define MEM_X   (6) /* MX column address order */
#define MEM_V   (5) /* MV row / column exchange */
#define MEM_L   (4) /* ML vertical refresh order */
#define MEM_H   (2) /* MH horizontal refresh order */
#define MEM_BGR (3) /* RGB-BGR Order */
static int set_var(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, 0x36, (1 << MEM_X) | (par->bgr << MEM_BGR));
		break;
	case 270:
		write_reg(par, 0x36,
			(1<<MEM_V) | (1 << MEM_L) | (par->bgr << MEM_BGR));
		break;
	case 180:
		write_reg(par, 0x36, (1 << MEM_Y) | (par->bgr << MEM_BGR));
		break;
	case 90:
		write_reg(par, 0x36, (1 << MEM_Y) | (1 << MEM_X) |
				     (1 << MEM_V) | (par->bgr << MEM_BGR));
		break;
	}

	return 0;
}

/*
  Gamma string format:
    Positive: Par1 Par2 [...] Par15
    Negative: Par1 Par2 [...] Par15
*/
#define CURVE(num, idx)  curves[num*par->gamma.num_values + idx]
static int set_gamma(struct fbtft_par *par, unsigned long *curves)
{
	int i;

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	for (i = 0; i < par->gamma.num_curves; i++)
		write_reg(par, 0xE0 + i,
			CURVE(i, 0), CURVE(i, 1), CURVE(i, 2),
			CURVE(i, 3), CURVE(i, 4), CURVE(i, 5),
			CURVE(i, 6), CURVE(i, 7), CURVE(i, 8),
			CURVE(i, 9), CURVE(i, 10), CURVE(i, 11),
			CURVE(i, 12), CURVE(i, 13), CURVE(i, 14));

	return 0;
}
#undef CURVE


static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = TXBUFLEN,
	.gamma_num = 2,
	.gamma_len = 15,
	.gamma = DEFAULT_GAMMA,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
		.set_gamma = set_gamma,
	},
};
FBTFT_REGISTER_DRIVER(DRVNAME, &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);

MODULE_DESCRIPTION("FB driver for the ILI9340 LCD display controller");
MODULE_AUTHOR("Christian Vogelgsang");
MODULE_LICENSE("GPL");
