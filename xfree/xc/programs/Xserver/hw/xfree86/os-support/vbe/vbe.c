/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/vbe/vbe.c,v 1.14 2000/12/06 15:35:32 eich Exp $ */

#include "xf86.h"
#include "xf86_ansic.h"
#include "vbe.h"
#include "Xarch.h"

#define VERSION(x) *((CARD8*)(&x) + 1),(CARD8)(x)

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
#define B_O16(x)  (x) 
#define B_O32(x)  (x)
#else
#define B_O16(x)  ((((x) & 0xff) << 8) | (((x) & 0xff) >> 8))
#define B_O32(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) \
                  | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#endif
#define L_ADD(x)  (B_O32(x) & 0xffff) + ((B_O32(x) >> 12) & 0xffff00)

static unsigned char * vbeReadEDID(vbeInfoPtr pVbe);
static Bool vbeProbeDDC(vbeInfoPtr pVbe);

static const char *ddcSymbols[] = {
    "xf86InterpretEDID",
    NULL
};

vbeInfoPtr
VBEInit(xf86Int10InfoPtr pInt, int entityIndex)
{
    int RealOff;
    pointer page = NULL;
    ScrnInfoPtr pScrn = xf86FindScreenForEntity(entityIndex);
    vbeControllerInfoPtr vbe = NULL;
    char vbeVersionString[] = "VBE2";
    Bool init_int10 = FALSE;
    vbeInfoPtr vip = NULL;
    int screen = pScrn->scrnIndex;

    if (!pInt) {
	if (!xf86LoadSubModule(pScrn, "int10"))
	    goto error;

	xf86DrvMsg(screen,X_INFO,"initializing int10\n");
	pInt = xf86InitInt10(entityIndex);
	if (!pInt)
	    goto error;
	init_int10 = TRUE;
    }
    
    page = xf86Int10AllocPages(pInt,1,&RealOff);
    if (!page) goto error;
    vbe = (vbeControllerInfoPtr) page;    
    memcpy(vbe->VbeSignature,vbeVersionString,4);

    pInt->ax = 0x4F00;
    pInt->es = SEG_ADDR(RealOff);
    pInt->di = SEG_OFF(RealOff);
    pInt->num = 0x10;
    
    xf86ExecX86int10(pInt);

    if ((pInt->ax & 0xff) != 0x4f) {
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA BIOS not detected\n");
	goto error;
    }
    
    switch (pInt->ax & 0xff00) {
    case 0:
	xf86DrvMsg(screen,X_INFO,"VESA BIOS detected\n");
	break;
    case 0x100:
	xf86DrvMsg(screen,X_INFO,"VESA BIOS function failed\n");
	goto error;
    case 0x200:
	xf86DrvMsg(screen,X_INFO,"VESA BIOS not supported\n");
	goto error;
    case 0x300:
	xf86DrvMsg(screen,X_INFO,"VESA BIOS not supported in current mode\n");
	goto error;
    default:
	xf86DrvMsg(screen,X_INFO,"Invalid\n");
	goto error;
    }
    
    xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE Version %i.%i\n",
		   VERSION(vbe->VbeVersion));
    xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE Total Mem: %i kB\n",
		   vbe->TotalMem * 64);
    xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE OEM: %s\n",
		   (CARD8*)xf86int10Addr(pInt,L_ADD(vbe->OemStringPtr)));
    
    if (B_O16(vbe->VbeVersion) >= 0x200) {
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE OEM Software Rev: %i.%i\n",
		    VERSION(vbe->OemSoftwareRev));
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE OEM Vendor: %s\n",
		    (CARD8*)xf86int10Addr(pInt,L_ADD(vbe->OemVendorNamePtr)));
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE OEM Product: %s\n",
		    (CARD8*)xf86int10Addr(pInt,L_ADD(vbe->OemProductNamePtr)));
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE OEM Product Rev: %s\n",
		    (CARD8*)xf86int10Addr(pInt,L_ADD(vbe->OemProductRevPtr)));
    }
    vip = (vbeInfoPtr)xnfalloc(sizeof(vbeInfoRec));
    vip->version = B_O16(vbe->VbeVersion);
    vip->pInt10 = pInt;
    vip->ddc = DDC_UNCHECKED;
    vip->memory = page;
    vip->real_mode_base = RealOff;
    vip->num_pages = 1;
    vip->init_int10 = init_int10;
   
    return vip;

 error:
    if (page)
	xf86Int10FreePages(pInt, page, 1);
    if (init_int10)
	xf86FreeInt10(pInt);
    return NULL;
}

void
vbeFree(vbeInfoPtr pVbe)
{
    if (!pVbe)
	return;

    xf86Int10FreePages(pVbe->pInt10,pVbe->memory,pVbe->num_pages);
    /* If we have initalized int10 we ought to free it, too */
    if (pVbe->init_int10) 
	xf86FreeInt10(pVbe->pInt10);
    xfree(pVbe);
    return;
}

static Bool
vbeProbeDDC(vbeInfoPtr pVbe)
{
    char *ddc_level;
    int screen = pVbe->pInt10->scrnIndex;
    
    if (!pVbe || (pVbe->ddc == DDC_NONE))
	return FALSE;
    if (pVbe->ddc != DDC_UNCHECKED)
	return TRUE;

    pVbe->pInt10->ax = 0x4F15;
    pVbe->pInt10->bx = 0;
    pVbe->pInt10->cx = 0;
    pVbe->pInt10->es = 0;
    pVbe->pInt10->di = 0;
    pVbe->pInt10->num = 0x10;

    xf86ExecX86int10(pVbe->pInt10);

    if ((pVbe->pInt10->ax & 0xff) != 0x4f) {
	pVbe->ddc = DDC_NONE;
	return FALSE;
    }

    switch ((pVbe->pInt10->ax >> 8) & 0xff) {
    case 0:
	xf86DrvMsg(screen,X_INFO,"VESA VBE DDC supported\n");
	switch (pVbe->pInt10->bx & 0x3) {
	case 0:
  	    ddc_level = " none"; 
	    pVbe->ddc = DDC_NONE;
	    break;
	case 1:
  	    ddc_level = " 1";
	    pVbe->ddc = DDC_1;
	    break;
	case 2:
  	    ddc_level = " 2"; 
	    pVbe->ddc = DDC_2;
	    break;
	case 3:
  	    ddc_level = " 1 + 2"; 
	    pVbe->ddc = DDC_1_2;
	    break;
	default:
 	    ddc_level = "";
	    pVbe->ddc = DDC_NONE;
	    break;
	}
  	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC Level%s\n",ddc_level); 
  	if (pVbe->pInt10->bx & 0x4) {
    	    xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC Screen blanked" 
    			"for data transfer\n"); 
    	    pVbe->ddc_blank = TRUE;
    	}  else
    	    pVbe->ddc_blank = FALSE;
	    
  	xf86DrvMsgVerb(screen,X_INFO,3,
		       "VESA VBE DDC transfer in appr. %x sec.\n", 
		       (pVbe->pInt10->bx >> 8) & 0xff); 
    }
    
    return TRUE; 
}

typedef enum {
    VBEOPT_NOVBE
} VBEOpts;

static OptionInfoRec VBEOptions[] = {
    { VBEOPT_NOVBE,	"NoVBE",	OPTV_BOOLEAN,	{0},	FALSE },
    { -1,		NULL,		OPTV_NONE,	{0},	FALSE },
};

#define nVBEOptions (sizeof(VBEOptions) / sizeof(VBEOptions[0]))

static unsigned char *
vbeReadEDID(vbeInfoPtr pVbe)
{
    int RealOff = pVbe->real_mode_base;
    pointer page = pVbe->memory;
    unsigned char *tmp = NULL;
    Bool novbe = FALSE;
    int screen = pVbe->pInt10->scrnIndex;
    OptionInfoRec options[nVBEOptions];

    if (!page) return NULL;

    (void)memcpy(options, VBEOptions, sizeof(VBEOptions));
    xf86ProcessOptions(screen, xf86Screens[screen]->options, options);
    xf86GetOptValBool(options, VBEOPT_NOVBE, &novbe);
    if (novbe) return NULL;
    
    if (!vbeProbeDDC(pVbe)) goto error;
    
    pVbe->pInt10->ax = 0x4F15;
    pVbe->pInt10->bx = 0x01;
    pVbe->pInt10->cx = 0;
    pVbe->pInt10->es = SEG_ADDR(RealOff);
    pVbe->pInt10->di = SEG_OFF(RealOff);
    pVbe->pInt10->num = 0x10;

    xf86ExecX86int10(pVbe->pInt10);

    if ((pVbe->pInt10->ax & 0xff) != 0x4f) {
        xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC invalid\n");
	goto error;
    }
    switch (pVbe->pInt10->ax & 0xff00) {
    case 0x0:
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC read successfully\n");
  	tmp = (unsigned char *)xnfalloc(128); 
  	memcpy(tmp,page,128); 
	break;
    case 0x100:
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC read failed\n");	
	break;
    default:
	xf86DrvMsgVerb(screen,X_INFO,3,"VESA VBE DDC unkown failure %i\n",
		       pVbe->pInt10->ax & 0xff00);
	break;
    }
    
 error:
    return tmp;
}

xf86MonPtr
vbeDoEDID(vbeInfoPtr pVbe, pointer pDDCModule)
{
    xf86MonPtr    pMonitor;
    pointer       pModule;
    unsigned char *DDC_data = NULL;
    
    if (!pVbe) return NULL;
    if (pVbe->version < 0x200)
	return NULL;

    if (!(pModule = pDDCModule)) {
	pModule =
	    xf86LoadSubModule(xf86Screens[pVbe->pInt10->scrnIndex], "ddc");
	if (!pModule)
	    return NULL;

	xf86LoaderReqSymLists(ddcSymbols, NULL);
    }
        
    DDC_data = vbeReadEDID(pVbe);

    if (!DDC_data) 
	return NULL;
    
    pMonitor = xf86InterpretEDID(pVbe->pInt10->scrnIndex, DDC_data);

    if (!pDDCModule)
        xf86UnloadSubModule(pModule);
    return pMonitor;
}


Bool
vbeModeInit(vbeInfoPtr pVbe, int mode)
{
    pVbe->pInt10->ax = 0x4F02;
    pVbe->pInt10->bx = mode | (1 << 14);
    xf86ExecX86int10(pVbe->pInt10);

    if ((pVbe->pInt10->ax & 0xff) != 0x4f)
	return FALSE;

    return TRUE;
    
}


