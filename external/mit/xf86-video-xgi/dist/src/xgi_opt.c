/*
 * Video driver option evaluation
 *
 * Copyright (C) 2001-2004 by Thomas Winischhofer, Vienna, Austria
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3) The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * "NoAccel", "NoXVideo", "SWCursor", "HWCursor" and "Rotate" option portions
 * Copyright (C) 1999-2004 The XFree86 Project, Inc. Licensed under the terms
 * of the XFree86 license (http://www.xfree86.org/current/LICENSE1.html).
 *
 * Authors:  	Thomas Winischhofer <thomas@winischhofer.net>
 *              ?
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86PciInfo.h"
#include "xf86str.h"
#include "xf86Cursor.h"

#include "xgi.h"

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_ACCELMETHOD,
    OPTION_NOACCEL,
    OPTION_TURBOQUEUE,
    OPTION_RENDER,
    OPTION_FORCE_CRT1TYPE,
    OPTION_FORCE_CRT2TYPE,
    OPTION_YPBPRAR,
    OPTION_SHADOW_FB,
    OPTION_DRI,
    OPTION_AGP_SIZE,
    OPTION_AGP_SIZE2,
    OPTION_ROTATE,
    OPTION_NOXVIDEO,
    OPTION_MAXXFBMEM,
    OPTION_USEROMDATA,
    OPTION_NOINTERNALMODES,
    OPTION_RESTOREBYSET,
    OPTION_NODDCFORCRT2,
    OPTION_FORCECRT2REDETECTION,
    OPTION_SENSEYPBPR,
    OPTION_CRT1GAMMA,
    OPTION_CRT2GAMMA,
    OPTION_XVGAMMA,
    OPTION_XVDEFCONTRAST,
    OPTION_XVDEFBRIGHTNESS,
    OPTION_XVDEFHUE,
    OPTION_XVDEFSATURATION,
    OPTION_XVDEFDISABLEGFX,
    OPTION_XVDEFDISABLEGFXLR,
    OPTION_XVMEMCPY,
    OPTION_SCALELCD,
    OPTION_CENTERLCD,
    OPTION_SPECIALTIMING,
    OPTION_ENABLEHOTKEY,
    OPTION_MERGEDFB,
    OPTION_MERGEDFBAUTO,
    OPTION_CRT2HSYNC,
    OPTION_CRT2VREFRESH,
    OPTION_CRT2POS,
    OPTION_METAMODES,
    OPTION_MERGEDFB2,
    OPTION_CRT2HSYNC2,
    OPTION_CRT2VREFRESH2,
    OPTION_CRT2POS2,
    OPTION_NOXGIXINERAMA,
    OPTION_NOXGIXINERAMA2,
    OPTION_CRT2ISSCRN0,
    OPTION_MERGEDDPI,
    OPTION_ENABLEXGICTRL,
    OPTION_STOREDBRI,
    OPTION_STOREDPBRI,
    OPTION_RUNTIME_DEBUG,  /* Jong 07/27/2009 */
	OPTION_TARGET_RATE,    /* Jong@09032009 */
	OPTION_IGNORE_DDC,     /* Jong@09032009 */
	OPTION_NONDDC_DEFAULT_MODE, /* Jong@09042009 */
	OPTION_GAMMA_RGB,			/* Jong@09092009 */
#ifdef XGI_CP
    XGI_CP_OPT_OPTIONS
#endif
    OPTION_PSEUDO
} XGIOpts;

static const OptionInfoRec XGIOptions[] = {
    { OPTION_SW_CURSOR,         	"SWcursor",               OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_HW_CURSOR,         	"HWcursor",               OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_ACCELMETHOD,			"AccelMethod",			OPTV_STRING,	{0}, FALSE },
    { OPTION_NOACCEL,           	"NoAccel",                OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_TURBOQUEUE,        	"TurboQueue",             OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_RENDER,        		"RenderAcceleration",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_FORCE_CRT1TYPE,    	"ForceCRT1Type",          OPTV_STRING,    {0}, FALSE },
    { OPTION_FORCE_CRT2TYPE,    	"ForceCRT2Type",          OPTV_STRING,    {0}, FALSE },
    { OPTION_YPBPRAR,  		  		"YPbPrAspectRatio",       OPTV_STRING,    {0}, FALSE },
    { OPTION_SHADOW_FB,         	"ShadowFB",               OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_DRI,         		"DRI",               	  OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_AGP_SIZE,			"AGPSize",      	  OPTV_INTEGER,   {0}, FALSE },
    { OPTION_AGP_SIZE2,			"GARTSize",      	  OPTV_INTEGER,   {0}, FALSE },
    { OPTION_ROTATE,            	"Rotate",                 OPTV_STRING,    {0}, FALSE },
    { OPTION_NOXVIDEO,          	"NoXvideo",               OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_MAXXFBMEM,         	"MaxXFBMem",              OPTV_INTEGER,   {0}, -1    },
    { OPTION_SPECIALTIMING,        	"SpecialTiming",          OPTV_STRING,    {0}, -1    },
    { OPTION_USEROMDATA,		"UseROMData",	          OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_NOINTERNALMODES,   	"NoInternalModes",        OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_RESTOREBYSET,		"RestoreBySetMode", 	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_NODDCFORCRT2,		"NoCRT2Detection", 	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_FORCECRT2REDETECTION,	"ForceCRT2ReDetection",   OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_SENSEYPBPR,		"SenseYPbPr",   	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_CRT1GAMMA,			"CRT1Gamma", 	  	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_CRT2GAMMA,			"CRT2Gamma", 	  	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_STOREDBRI,			"StoredGammaBrightness",  OPTV_STRING,    {0}, -1    },
    { OPTION_STOREDPBRI,		"StoredGammaPreBrightness",OPTV_STRING,   {0}, -1    },
    { OPTION_XVGAMMA,			"XvGamma", 	  	  OPTV_STRING,    {0}, -1    },
    { OPTION_XVDEFCONTRAST,		"XvDefaultContrast", 	  OPTV_INTEGER,   {0}, -1    },
    { OPTION_XVDEFBRIGHTNESS,		"XvDefaultBrightness", 	  OPTV_INTEGER,   {0}, -1    },
    { OPTION_XVDEFHUE,			"XvDefaultHue", 	  OPTV_INTEGER,   {0}, -1    },
    { OPTION_XVDEFSATURATION,		"XvDefaultSaturation", 	  OPTV_INTEGER,   {0}, -1    },
    { OPTION_XVDEFDISABLEGFX,		"XvDefaultDisableGfx", 	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_XVDEFDISABLEGFXLR,		"XvDefaultDisableGfxLR",  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_XVMEMCPY,			"XvUseMemcpy",  	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_ENABLEHOTKEY,		"EnableHotkey",	   	  OPTV_BOOLEAN,   {0}, -1    },
    { OPTION_ENABLEXGICTRL,		"EnableXGICtrl",   	  OPTV_BOOLEAN,   {0}, -1    },
#ifdef XGIMERGED
    { OPTION_MERGEDFB,			"MergedFB",		  OPTV_BOOLEAN,	  {0}, FALSE },
    { OPTION_MERGEDFB2,			"TwinView",		  OPTV_BOOLEAN,	  {0}, FALSE },	  /* alias */
    { OPTION_MERGEDFBAUTO,		"MergedFBAuto",		  OPTV_BOOLEAN,	  {0}, FALSE },
    { OPTION_CRT2HSYNC,			"CRT2HSync",		  OPTV_STRING,	  {0}, FALSE },
    { OPTION_CRT2HSYNC2,		"SecondMonitorHorizSync", OPTV_STRING,	  {0}, FALSE },   /* alias */
    { OPTION_CRT2VREFRESH,		"CRT2VRefresh",		  OPTV_STRING,    {0}, FALSE },
    { OPTION_CRT2VREFRESH2,		"SecondMonitorVertRefresh", OPTV_STRING,  {0}, FALSE },   /* alias */
    { OPTION_CRT2POS,   		"CRT2Position",		  OPTV_STRING,	  {0}, FALSE },
    { OPTION_CRT2POS2,   		"TwinViewOrientation",	  OPTV_STRING,	  {0}, FALSE },   /* alias */
    { OPTION_METAMODES,   		"MetaModes",  		  OPTV_STRING,	  {0}, FALSE },
    { OPTION_MERGEDDPI,			"MergedDPI", 		  OPTV_STRING,	  {0}, FALSE },
    { OPTION_RUNTIME_DEBUG,		"RunTimeDebug",      	  OPTV_BOOLEAN,   {0}, -1 }, /* Jong 07/27/2009 */
    { OPTION_TARGET_RATE,		"TargetRate",      	  OPTV_INTEGER,   {0}, -1 }, /* Jong@09032009 */
    { OPTION_IGNORE_DDC,		"IgnoreDDC",      	  OPTV_BOOLEAN,   {0}, -1 }, /* Jong@09032009 */
    { OPTION_NONDDC_DEFAULT_MODE,		"NonDDCDefaultMode",      	  OPTV_STRING,   {0}, FALSE }, /* Jong@09042009 */
    { OPTION_GAMMA_RGB,			"GammaRGB",      	  OPTV_STRING,   {0}, FALSE }, /* Jong@09092009 */
#ifdef XGIXINERAMA
    { OPTION_NOXGIXINERAMA,		"NoMergedXinerama",	  OPTV_BOOLEAN,	  {0}, FALSE },
    { OPTION_NOXGIXINERAMA2,		"NoTwinviewXineramaInfo", OPTV_BOOLEAN,   {0}, FALSE },   /* alias */
    { OPTION_CRT2ISSCRN0,		"MergedXineramaCRT2IsScreen0",OPTV_BOOLEAN,{0},FALSE },
#endif
#endif
#ifdef XGI_CP
    XGI_CP_OPTION_DETAIL
#endif
    { -1,                       	NULL,                     OPTV_NONE,      {0}, FALSE }
};

unsigned int g_GammaRed;
unsigned int g_GammaGreen;
unsigned int g_GammaBlue;

void
xgiOptions(ScrnInfoPtr pScrn)
{
    XGIPtr      pXGI = XGIPTR(pScrn);
    MessageType from;
    char        *strptr;
    static const char *mybadparm = "\"%s\" is is not a valid parameter for option \"%s\"\n";
    static const char *disabledstr = "disabled";
    static const char *enabledstr = "enabled";
    static const char *ilrangestr = "Illegal %s parameter. Valid range is %d through %d\n";

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);

    /* Process the options */
    if(!(pXGI->Options = xalloc(sizeof(XGIOptions)))) return;

    memcpy(pXGI->Options, XGIOptions, sizeof(XGIOptions));

    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pXGI->Options);

    /* Set defaults */
/*
#ifdef __powerpc__
    pXGI->NoAccel = TRUE;
#endif 
*/
    pXGI->TurboQueue = TRUE;
#ifdef XGIVRAMQ
    /* TODO: Option (315 series VRAM command queue) */
    /* But beware: xgifb does not know about this!!! */
    pXGI->cmdQueueSize = 512*1024;
#endif
    pXGI->doRender = TRUE;
    pXGI->HWCursor = TRUE;
    pXGI->Rotate = FALSE;
    pXGI->ShadowFB = FALSE;

	/* Jong 01/22/2009; only XG40 has 3-d feature */
	if(pXGI->Chipset == PCI_CHIP_XGIXG40)
		pXGI->loadDRI = TRUE;
	else
		pXGI->loadDRI = FALSE;

    pXGI->agpWantedPages = AGP_PAGES;
    pXGI->NoXvideo = FALSE;
    pXGI->maxxfbmem = 0;
    pXGI->OptROMUsage = -1;
    pXGI->noInternalModes = FALSE;
    pXGI->NonDefaultPAL = pXGI->NonDefaultNTSC = -1;
    pXGI->restorebyset = TRUE;
    pXGI->nocrt2ddcdetection = FALSE;
    pXGI->forcecrt2redetection = TRUE;   /* default changed since 13/09/2003 */
    pXGI->SenseYPbPr = TRUE;
    pXGI->ForceCRT1Type = CRT1_VGA;
    pXGI->ForceCRT2Type = CRT2_DEFAULT;
    pXGI->ForceYPbPrAR = TV_YPBPR169;
    pXGI->ForceTVType = -1;
    pXGI->CRT1gamma = TRUE;
    pXGI->CRT1gammaGiven = FALSE;
    pXGI->CRT2gamma = TRUE;
    pXGI->XvGamma = FALSE;
    pXGI->XvGammaGiven = FALSE;
    pXGI->enablexgictrl = FALSE;
 
       pXGI->XvDefBri = 0;
       pXGI->XvDefCon = 4;

    pXGI->XvDefHue = 0;
    pXGI->XvDefSat = 0;
    pXGI->XvDefDisableGfx = FALSE;
    pXGI->XvDefDisableGfxLR = FALSE;
    pXGI->XvUseMemcpy = TRUE;
    pXGI->XvGammaRed = pXGI->XvGammaGreen = pXGI->XvGammaBlue = 1000;
#ifdef XGIMERGED
    pXGI->MergedFB = pXGI->MergedFBAuto = FALSE;
    pXGI->CRT2Position = xgiRightOf;
    pXGI->CRT2HSync = NULL;
    pXGI->CRT2VRefresh = NULL;
    pXGI->MetaModes = NULL;
    pXGI->MergedFBXDPI = pXGI->MergedFBYDPI = 0;
#ifdef XGIXINERAMA
    pXGI->UsexgiXinerama = TRUE;
    pXGI->CRT2IsScrn0 = FALSE;
#endif
#endif
#ifdef XGI_CP
    XGI_CP_OPT_DEFAULT
#endif


    /* Collect the options */

	int	TargetRefreshRate = 0;
    if(xf86GetOptValInteger(pXGI->Options /* pScrn->monitor->options */, OPTION_TARGET_RATE, &TargetRefreshRate)) 
	{
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Monitor (Option) : Set target refresh rate at %d for all modes...\n", TargetRefreshRate);
	}

    pXGI->TargetRefreshRate = TargetRefreshRate;

	pXGI->IgnoreDDC = FALSE;
    if(xf86GetOptValBool(pXGI->Options, OPTION_IGNORE_DDC, &pXGI->IgnoreDDC))
	{
		if(pXGI->IgnoreDDC == TRUE)
			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Monitor (Option) : IgnoreDDC \n");
	} 
#if 0 /* can support 1280x768 but not being applied */
	else
	{
		pXGI->IgnoreDDC = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Monitor (Option) : set IgnoreDDC as default\n");
	}
#endif

	pXGI->Non_DDC_DefaultMode = FALSE;
	pXGI->Non_DDC_DefaultResolutionX = 1024;
	pXGI->Non_DDC_DefaultResolutionY = 768;
	pXGI->Non_DDC_DefaultRefreshRate = 60;

	char	ModeStringFormat[32] = 	"%[^x]x%[^@]@%[^\n]" /* "%[^x]+%[^@]+%[^H^z]" */;
    char*   Non_DDCDefaultMode = "";
    char   Non_DDCDefaultResolutionX[8] = "";
    char   Non_DDCDefaultResolutionY[8] = "";
    char   Non_DDCDefaultRefreshRate[8] = "";

	/* strcpy(ModeStringFormat, "%[^+]x%[^+]@%[^\n]"); */

	if((Non_DDCDefaultMode = (char *)xf86GetOptValString(pXGI->Options, OPTION_NONDDC_DEFAULT_MODE))) 
	{
		sscanf(Non_DDCDefaultMode, ModeStringFormat, 
				Non_DDCDefaultResolutionX , 
				Non_DDCDefaultResolutionY , 
				Non_DDCDefaultRefreshRate  );

		if( (xf86NameCmp(Non_DDCDefaultResolutionX,"") == 0) || (xf86NameCmp(Non_DDCDefaultResolutionY,"") == 0) ) 
		{
			strcpy(Non_DDCDefaultResolutionX, "1024");
			strcpy(Non_DDCDefaultResolutionY, "768");
		}

		if( (xf86NameCmp(Non_DDCDefaultRefreshRate,"") == 0) || (xf86NameCmp(Non_DDCDefaultRefreshRate,"auto") == 0) ) 
				strcpy(Non_DDCDefaultRefreshRate, "60");

		ErrorF("Non-DDC default mode is (%s x %s @ %s Hz)...\n", 
					Non_DDCDefaultResolutionX ,
					Non_DDCDefaultResolutionY ,
					Non_DDCDefaultRefreshRate );

		pXGI->Non_DDC_DefaultMode = TRUE;

		pXGI->Non_DDC_DefaultResolutionX = atoi(Non_DDCDefaultResolutionX);
		pXGI->Non_DDC_DefaultResolutionY = atoi(Non_DDCDefaultResolutionY);
		pXGI->Non_DDC_DefaultRefreshRate = atoi(Non_DDCDefaultRefreshRate);

		ErrorF("Non-DDC default mode is (%d x %d @ %d Hz)...\n", 
					pXGI->Non_DDC_DefaultResolutionX ,
					pXGI->Non_DDC_DefaultResolutionY ,
					pXGI->Non_DDC_DefaultRefreshRate );
	}

	/* Jong@09092009; gamma value */
	g_GammaRed = g_GammaGreen = g_GammaBlue = 1000;

	char	GammaStringFormat[32] = "%[^,],%[^,],%[^\n]";
    char*   GammaRGB = "";
    char   GammaRed[8] = "";
    char   GammaGreen[8] = "";
    char   GammaBlue[8] = "";

	if((GammaRGB = (char *)xf86GetOptValString(pXGI->Options, OPTION_GAMMA_RGB))) 
	{
		ErrorF("GammaRGB is (%s) from xorg.conf\n", GammaRGB);
		sscanf(GammaRGB, GammaStringFormat, 
				GammaRed , 
				GammaGreen , 
				GammaBlue  );
		ErrorF("GammaRGB is (%s, %s, %s) after parsing\n", GammaRed, GammaGreen, GammaBlue);
		
		g_GammaRed = atoi(GammaRed);
		g_GammaGreen = atoi(GammaGreen);
		g_GammaBlue = atoi(GammaBlue);

		ErrorF("GammaRGB is (%d, %d, %d) after atoi()\n", g_GammaRed, g_GammaGreen, g_GammaBlue);
	}

    /* MaxXFBMem
     * This options limits the amount of video memory X uses for screen
     * and off-screen buffers. This option should be used if using DRI
     * is intended. The kernel framebuffer driver required for DRM will
     * start its memory heap at 12MB if it detects more than 16MB, at 8MB if
     * between 8 and 16MB are available, otherwise at 4MB. So, if the amount
     * of memory X uses, a clash between the framebuffer's memory heap
     * and X is avoided. The amount is to be specified in KB.
     */
    if(xf86GetOptValULong(pXGI->Options, OPTION_MAXXFBMEM,
                                &pXGI->maxxfbmem)) {
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                    "MaxXFBMem: Framebuffer memory shall be limited to %ld KB\n",
		    pXGI->maxxfbmem);
	    pXGI->maxxfbmem *= 1024;
    }

    /* NoAccel
     * Turns off 2D acceleration
     */
    if(xf86ReturnOptValBool(pXGI->Options, OPTION_NOACCEL, FALSE)) {
        pXGI->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "2D Acceleration disabled\n");
    }

	/* Jong@10022009; for xvinfo */
	if ((pXGI->Chipset== PCI_CHIP_XGIXG20)||(pXGI->Chipset== PCI_CHIP_XGIXG21)||(pXGI->Chipset== PCI_CHIP_XGIXG27))
		pXGI->NoXvideo = TRUE; 
		
	pXGI->useEXA = FALSE; /* default : XAA */
    if(!pXGI->NoAccel) 
	{
		from = X_DEFAULT;
		if((strptr = (char *)xf86GetOptValString(pXGI->Options, OPTION_ACCELMETHOD))) {
			if(!xf86NameCmp(strptr,"XAA")) {
				from = X_CONFIG;
				pXGI->useEXA = FALSE;
			} else if(!xf86NameCmp(strptr,"EXA")) {
				from = X_CONFIG;
				pXGI->useEXA = TRUE;
			}
		}

		xf86DrvMsg(pScrn->scrnIndex, from, "Using %s acceleration architecture\n",
			pXGI->useEXA ? "EXA" : "XAA");
    }

    /* SWCursor
     * HWCursor
     * Chooses whether to use the hardware or software cursor
     */
    from = X_DEFAULT;
    if(xf86GetOptValBool(pXGI->Options, OPTION_HW_CURSOR, &pXGI->HWCursor)) {
        from = X_CONFIG;
    }
    if(xf86ReturnOptValBool(pXGI->Options, OPTION_SW_CURSOR, FALSE)) {
        from = X_CONFIG;
        pXGI->HWCursor = FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
                                pXGI->HWCursor ? "HW" : "SW");

    /*
     * MergedFB
     *
     * Enable/disable and configure merged framebuffer mode
     *
     */
#ifdef XGIMERGED
    if (IS_DUAL_HEAD(pXGI)) {
       Bool val;
       if(xf86GetOptValBool(pXGI->Options, OPTION_MERGEDFB, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	     "Option \"MergedFB\" cannot be used in Dual Head mode\n");
       }
    } else
#endif

    /* Some options can only be specified in the Master Head's Device
     * section. Here we give the user a hint in the log.
     */
    if (IS_DUAL_HEAD(pXGI) && IS_SECOND_HEAD(pXGI)) {
       static const char *mystring = "Option \"%s\" is only accepted in Master Head's device section\n";
       Bool val;

       if(xf86GetOptValBool(pXGI->Options, OPTION_TURBOQUEUE, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "TurboQueue");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_RESTOREBYSET, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "RestoreBySetMode");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_ENABLEHOTKEY, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "EnableHotKey");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_ENABLEXGICTRL, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "EnableXGICtrl");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_USEROMDATA, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "UseROMData");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_NODDCFORCRT2, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "NoCRT2Detection");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_FORCECRT2REDETECTION, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "ForceCRT2ReDetection");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_SENSEYPBPR, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "SenseYPbPr");
       }
       if(xf86GetOptValString(pXGI->Options, OPTION_FORCE_CRT1TYPE)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "ForceCRT1Type");
       }
       if(xf86GetOptValString(pXGI->Options, OPTION_FORCE_CRT2TYPE)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "ForceCRT2Type");
       }
       if(xf86GetOptValString(pXGI->Options, OPTION_YPBPRAR)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "YPbPrAspectRatio");
       }
       if(xf86GetOptValString(pXGI->Options, OPTION_SPECIALTIMING)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "SpecialTiming");
       }
       if(xf86GetOptValBool(pXGI->Options, OPTION_CRT2GAMMA, &val)) {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mystring, "CRT2Gamma");
       }
#ifdef XGI_CP
       XGI_CP_OPT_DH_WARN
#endif
    }
    else {
	  /* TurboQueue */

          from = X_DEFAULT;
          if(xf86GetOptValBool(pXGI->Options, OPTION_TURBOQUEUE, &pXGI->TurboQueue)) {
    	     from = X_CONFIG;
          }
          xf86DrvMsg(pScrn->scrnIndex, from, "TurboQueue %s\n",
                    pXGI->TurboQueue ? enabledstr : disabledstr);

#ifdef XGI_CP
       XGI_CP_OPT_DOOPT
#endif

    }  /* DualHead */

    /* CRT1Gamma - enable/disable gamma correction for CRT1
     */
    {
       Bool val;
       if(xf86GetOptValBool(pXGI->Options, OPTION_CRT1GAMMA, &val)) {
	  pXGI->CRT1gamma = val;
	  pXGI->CRT1gammaGiven = TRUE;
       }
    }


    /* ShadowFB */
    from = X_DEFAULT;
    if(xf86GetOptValBool(pXGI->Options, OPTION_SHADOW_FB, &pXGI->ShadowFB)) {
#ifdef XGIMERGED
       if(pXGI->MergedFB) {
          pXGI->ShadowFB = FALSE;
	  xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	      "Shadow Frame Buffer not supported in MergedFB mode\n");
       } else
#endif
          from = X_CONFIG;
    }
    if(pXGI->ShadowFB) {
	pXGI->NoAccel = TRUE;
    	xf86DrvMsg(pScrn->scrnIndex, from,
	   "Using \"Shadow Frame Buffer\" - 2D acceleration disabled\n");
    }

    /* Rotate */
    if((strptr = (char *)xf86GetOptValString(pXGI->Options, OPTION_ROTATE))) {
#ifdef XGIMERGED
       if(pXGI->MergedFB) {
	  xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	      "Screen rotation not supported in MergedFB mode\n");
       } else
#endif
       if(!xf86NameCmp(strptr, "CW")) {
          pXGI->Rotate = 1;
       } else if(!xf86NameCmp(strptr, "CCW")) {
          pXGI->Rotate = -1;
       } else {
          xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mybadparm, strptr, "Rotate");
          xf86DrvMsg(pScrn->scrnIndex, X_INFO,
              "Valid parameters are \"CW\" or \"CCW\"\n");
       }

       if(pXGI->Rotate) {
          pXGI->ShadowFB = TRUE;
          pXGI->NoAccel  = TRUE;
          pXGI->HWCursor = FALSE;
	  xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
              "Rotating screen %sclockwise (2D acceleration %sdisabled)\n",
	      (pXGI->Rotate == -1) ? "counter " : "",
              "and RandR extension "
	      );
       }
    }

	/* Jong 07/27/2009; get option of run-time debug */
    if(!xf86GetOptValBool(pXGI->Options, OPTION_RUNTIME_DEBUG, &g_bRunTimeDebug))
		g_bRunTimeDebug=0;

#ifdef XF86DRI
    /* DRI */
    from = X_DEFAULT;
    if(xf86GetOptValBool(pXGI->Options, OPTION_DRI, &pXGI->loadDRI)) {
       from = X_CONFIG;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "DRI %s\n",
       pXGI->loadDRI ? enabledstr : disabledstr);

    /* AGPSize */
    {
       int vali;
       Bool gotit = FALSE;
       if(xf86GetOptValInteger(pXGI->Options, OPTION_AGP_SIZE, &vali)) {
          gotit = TRUE;
       } else if(xf86GetOptValInteger(pXGI->Options, OPTION_AGP_SIZE2, &vali)) {
          gotit = TRUE;
       }
       if(gotit) {
	  if((vali >= 8) && (vali <= 512)) {
	     pXGI->agpWantedPages = (vali * 1024 * 1024) / AGP_PAGE_SIZE;
	  } else {
	     xf86DrvMsg(pScrn->scrnIndex, X_WARNING, ilrangestr, "AGPSize (alias GARTSize)", 8, 512);
	  }
       }
    }
#endif

    /* NoXVideo
     * Set this to TRUE to disable Xv hardware video acceleration
     */
    if(!pXGI->NoXvideo) {
       if(xf86ReturnOptValBool(pXGI->Options, OPTION_NOXVIDEO, FALSE)) {
          pXGI->NoXvideo = TRUE;
          xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "XVideo extension disabled\n");
       }

       if(!pXGI->NoXvideo) {
          Bool val;
	  int tmp;

	  /* Some Xv properties' defaults can be set by options */
          if(xf86GetOptValInteger(pXGI->Options, OPTION_XVDEFCONTRAST, &tmp)) {
             if((tmp >= 0) && (tmp <= 7)) pXGI->XvDefCon = tmp;
             else xf86DrvMsg(pScrn->scrnIndex, X_WARNING, ilrangestr,
       		      "XvDefaultContrast" ,0, 7);
          }
          if(xf86GetOptValInteger(pXGI->Options, OPTION_XVDEFBRIGHTNESS, &tmp)) {
             if((tmp >= -128) && (tmp <= 127)) pXGI->XvDefBri = tmp;
             else xf86DrvMsg(pScrn->scrnIndex, X_WARNING, ilrangestr,
       		      "XvDefaultBrightness", -128, 127);
          }
          
	  if(xf86GetOptValBool(pXGI->Options, OPTION_XVDEFDISABLEGFX, &val)) {
	     if(val)  pXGI->XvDefDisableGfx = TRUE;
	     xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
	        "Graphics display will be %s during Xv usage\n",
	     	val ? disabledstr : enabledstr);
          }
	  
	  if(xf86GetOptValBool(pXGI->Options, OPTION_XVMEMCPY, &val)) {
	     pXGI->XvUseMemcpy = val ? TRUE : FALSE;
	     xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Xv will %suse memcpy()\n",
	     	val ? "" : "not ");
          }
       }
    }
}

const OptionInfoRec *
XGIAvailableOptions(int chipid, int busid)
{
    return XGIOptions;
}
