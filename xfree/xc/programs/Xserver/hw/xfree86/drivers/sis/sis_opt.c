/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_opt.c,v 1.4 2000/12/02 01:16:18 dawes Exp $ */

#include "xf86.h"

#include "sis.h"

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_PCI_RETRY,
    OPTION_RGB_BITS,
    OPTION_NOACCEL,
    OPTION_TURBOQUEUE,
    OPTION_FAST_VRAM,
    OPTION_SET_MEMCLOCK,
    OPTION_FORCE_CRT2TYPE
} SISOpts;

static OptionInfoRec SISOptions[] = {
    { OPTION_SW_CURSOR,		"SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_HW_CURSOR,		"HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_PCI_RETRY,		"PciRetry",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_RGB_BITS,		"rgbbits",	OPTV_INTEGER,	{0}, -1    },
    { OPTION_NOACCEL,		"NoAccel",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_TURBOQUEUE,	"TurboQueue",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_SET_MEMCLOCK,	"SetMClk",	OPTV_FREQ,	{0}, -1    },
    { OPTION_FAST_VRAM,		"FastVram",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_FORCE_CRT2TYPE,    "ForceCRT2Type",OPTV_ANYSTR,    {0}, FALSE },	
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

void
SiSOptions(ScrnInfoPtr pScrn)
{
	SISPtr	pSiS = SISPTR(pScrn);
	MessageType	from;
	double		temp;
	char *strptr;

	/* Collect all of the relevant option flags (fill in pScrn->options) */
	xf86CollectOptions(pScrn, NULL);

	/* Process the options */
	xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, SISOptions);

	from = X_DEFAULT;
	if (pScrn->depth <= 8)  {
#if 0
		if (xf86GetOptValInteger(SISOptions, OPTION_RGB_BITS,
			 		&pScrn->rgbBits))
		{
	    		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				"Bits per RGB set to %d\n", pScrn->rgbBits);
		}
#endif
	}
	pSiS->HWCursor = TRUE;
	if (xf86GetOptValBool(SISOptions, OPTION_HW_CURSOR, &pSiS->HWCursor))
		from = X_CONFIG;
	if (xf86ReturnOptValBool(SISOptions, OPTION_SW_CURSOR, FALSE)) {
		from = X_CONFIG;
		pSiS->HWCursor = FALSE;
	}
	xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
				pSiS->HWCursor ? "HW" : "SW");
	if (xf86ReturnOptValBool(SISOptions, OPTION_NOACCEL, FALSE)) {
		pSiS->NoAccel = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
	}
	if (xf86ReturnOptValBool(SISOptions, OPTION_PCI_RETRY, FALSE)) {
		pSiS->UsePCIRetry = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "PCI retry enabled\n");
	}
	if (xf86GetOptValFreq(SISOptions, OPTION_SET_MEMCLOCK, OPTUNITS_MHZ,
					&temp)) {
		pSiS->MemClock = (int)(temp * 1000.0);
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Memory clock set to %.3f MHz\n",
			pSiS->MemClock/1000.0);
	}
	if (xf86ReturnOptValBool(SISOptions, OPTION_FAST_VRAM, FALSE)) {
		pSiS->FastVram = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Fast VRAM enabled\n");
	}

/*	if (xf86ReturnOptValBool(SISOptions, OPTION_TURBOQUEUE, FALSE)) {
		pSiS->TurboQueue = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Enabling TurboQueue\n");
	}   */
	pSiS->TurboQueue = TRUE;


	pSiS->ForceCRT2Type= CRT2_DEFAULT; 
	strptr = (char *)xf86GetOptValString(SISOptions,OPTION_FORCE_CRT2TYPE); 
	if (strptr != 0)
	{	
		if (!xf86strcmp(strptr,"TV"))
			pSiS->ForceCRT2Type = CRT2_TV;
		if (!xf86strcmp(strptr,"LCD"))
			pSiS->ForceCRT2Type = CRT2_LCD;
		if (!xf86strcmp(strptr,"VGA"))
			pSiS->ForceCRT2Type = CRT2_VGA;
	}
}

OptionInfoPtr
SISAvailableOptions(int chipid, int busid)
{
    return SISOptions;
}

