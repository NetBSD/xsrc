/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_dac.h,v 1.4 2002/01/17 09:57:30 eich Exp $ */
int SiScompute_vclk(int Clock, int *out_n, int *out_dn, int *out_div, 
	     			    int *out_sbit, int *out_scale);
void SISDACPreInit(ScrnInfoPtr pScrn);
unsigned int SiSddc1Read(ScrnInfoPtr pScrn);
void SISLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies,
		                LOCO *colors, VisualPtr pVisual);
void SiSCalcClock(ScrnInfoPtr pScrn, int clock, int max_VLD,
                        unsigned int *vclk);

void SiSIODump(ScrnInfoPtr pScrn);
int SiSMemBandWidth(ScrnInfoPtr pScrn);
int SiSMclk(SISPtr pSiS);
void SiSRestoreBridge(ScrnInfoPtr pScrn, SISRegPtr sisReg);

