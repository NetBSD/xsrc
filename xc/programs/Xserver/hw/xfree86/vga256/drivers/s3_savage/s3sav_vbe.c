/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_vbe.c,v 1.1.2.1 1999/07/30 11:21:37 hohndel Exp $ */
/*
 * This module provides the S3V driver interface to the VESA BIOS on
 * the graphics card.
 *
 * This module was derived from the vbetest.c sample application in
 * Josh Vanderhoof's Linux Real Mode Interface library.
 *
 * Tim Roberts, 21-June-1999.
 */

/* General and xf86 includes */
#include "X.h"
#include "input.h"
#include "screenint.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

/* Includes for Options flags */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/* S3V internal includes */
#include "s3sav_driver.h"
#include "regs3sav.h"

/* LRMI includes */

#define LRMI_PREFIX	S3V_
#include "lrmi.h"
#include "vbe.h"

#ifdef USEBIOS

struct
{
    unsigned char initialized;
    struct vbe_info_block *info;
    struct vbe_mode_info_block *mode;
} vbe;


static void *
save_state(void)
{
    struct LRMI_regs r;
    void *buffer;

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f04;
    r.ecx = 0xf; 	/* all states */
    r.edx = 0; 	/* get buffer size */

    if (!LRMI_int(0x10, &r))
    {
	ErrorF("Can't get video state buffer size (vm86 failure)\n");
	return NULL;
    }

    if ((r.eax & 0xffff) != 0x4f)
    {
	ErrorF("Get video state buffer size failed\n");
	return NULL;
    }

    buffer = LRMI_alloc_real((r.ebx & 0xffff) * 64);

    if (buffer == NULL)
    {
	ErrorF("Can't allocate video state buffer\n");
	return NULL;
    }

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f04;
    r.ecx = 0xf; 	/* all states */
    r.edx = 1; 	/* save state */
    r.es = (unsigned int)buffer >> 4;
    r.ebx = (unsigned int)buffer & 0xf;

    if (!LRMI_int(0x10, &r))
    {
	ErrorF("Can't save video state (vm86 failure)\n");
	return NULL;
    }

    if ((r.eax & 0xffff) != 0x4f)
    {
	ErrorF("Save video state failed\n");
	return NULL;
    }

    return buffer;
}

static void
restore_state(void *buffer)
{
    struct LRMI_regs r;

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f04;
    r.ecx = 0xf; 	/* all states */
    r.edx = 2; 	/* restore state */
    r.es = (unsigned int)buffer >> 4;
    r.ebx = (unsigned int)buffer & 0xf;

    if (!LRMI_int(0x10, &r))
    {
	ErrorF("Can't restore video state (vm86 failure)\n");
    }
    else if ((r.eax & 0xffff) != 0x4f)
    {
	ErrorF("Restore video state failed\n");
    }

    LRMI_free_real(buffer);
}

void
S3SAVSetTextMode(void)
{
   struct LRMI_regs r;

   memset(&r, 0, sizeof(r));

   r.eax = 3;

   if (!LRMI_int(0x10, &r))
   {
      ErrorF("Can't set text mode (vm86 failure)\n");
   }
}


void
S3SAVSetVESAMode( int n, int Refresh )
{
    struct LRMI_regs r;

    /* 
     * The Savage BIOS also writes to a debug card on port 80h.  It 
     * shouldn't, but we can work around it here.
     */
    ioperm( 0x80, 1, 1 );

    /* First, establish the refresh rate for this mode. */

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f14;	/* S3 extensions */
    r.ebx = 0x0001;	/* Set default refresh rate */
    r.ecx = n;
    r.edi = Refresh;

    if( !LRMI_int(0x10, &r))
    {
	ErrorF("Can't set refresh rate.\n");
    }

    /* Now, make this mode current. */

    /*
     * The Savage BIOS reprograms the timer chip.  One could argue that
     * these ports should be virtualized rather than laid wide open.
     */
    ioperm( 0x40, 4, 1 );
    ioperm( 0x61, 1, 1 );

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f02;
    r.ebx = n;

    if (!LRMI_int(0x10, &r))
    {
	ErrorF("Can't set video mode (vm86 failure)\n");
    }
    else if ((r.eax & 0xffff) != 0x4f)
    {
	ErrorF("Set video mode failed\n");
    }

    ioperm( 0x40, 4, 0 );
    ioperm( 0x61, 1, 0 );
    ioperm( 0x80, 1, 1 );
}


unsigned short
S3SAVGetBIOSModeCount( int iDepth )
{
    return S3SAVGetBIOSModeTable( iDepth, NULL );
}


unsigned short
S3SAVGetBIOSModeTable( int iDepth, S3VMODETABLE* s3vModeTable )
{
    struct LRMI_regs r;
    short int *mode_list;
    unsigned short iModeCount = 0;

    if( !vbe.initialized )
	if (!LRMI_init())
	    return 1;

    vbe.initialized = 1;

    vbe.info = LRMI_alloc_real(sizeof(struct vbe_info_block)
     + sizeof(struct vbe_mode_info_block));

    if (vbe.info == NULL)
    {
	ErrorF("Can't alloc real mode memory\n");
	return 1;
    }

    vbe.mode = (struct vbe_mode_info_block *)(vbe.info + 1);

    /* Allow access to bogus port 80h. */

    ioperm( 0x80, 1, 1 );

    /* Fetch VESA driver information. */

    memset(&r, 0, sizeof(r));

    r.eax = 0x4f00;
    r.es = (unsigned int)vbe.info >> 4;
    r.edi = 0;

    memcpy(vbe.info->vbe_signature, "VBE2", 4);

    if (!LRMI_int(0x10, &r))
    {
	ErrorF("Can't get VESA info (vm86 failure)\n");
	ioperm( 0x80, 1, 0 );
	return 1;
    }

    if ((r.eax & 0xffff) != 0x4f || 
	strncmp(vbe.info->vbe_signature, "VESA", 4) != 0)
    {
	ErrorF("No VESA bios\n");
	ioperm( 0x80, 1, 0 );
	return 1;
    }

    if( !s3vModeTable )
    {
	ErrorF("%s %s: VBE Version %x.%x\n",
	    XCONFIG_PROBED, vga256InfoRec.name,
	    (int)(vbe.info->vbe_version >> 8) & 0xff,
	    (int)vbe.info->vbe_version & 0xff);

	ErrorF("%s %s: BIOS label is \"%s\"\n",
	    XCONFIG_PROBED, vga256InfoRec.name,
	    (char *)(vbe.info->oem_string_seg * 16 + vbe.info->oem_string_off));
    }

    mode_list = (short int *)
	(vbe.info->video_mode_list_seg * 16 + vbe.info->video_mode_list_off);

    while (*mode_list != -1)
    {
	memset(&r, 0, sizeof(r));

	r.eax = 0x4f01;
	r.ecx = *mode_list;
	r.es = (unsigned int)vbe.mode >> 4;
	r.edi = (unsigned int)vbe.mode & 0xf;

	if (!LRMI_int(0x10, &r))
	{
	    ErrorF("Can't get mode info (vm86 failure)\n");
	    ioperm( 0x80, 1, 0 );
	    return 1;
	}

	if( 
	   (vbe.mode->bits_per_pixel == iDepth) &&
	   (
	      (vbe.mode->memory_model == VBE_MODEL_256) ||
	      (vbe.mode->memory_model == VBE_MODEL_PACKED) ||
	      (vbe.mode->memory_model == VBE_MODEL_RGB)
	   )
	)
	{
	    /* This mode is a match. */

	    iModeCount++;

	    /* If we're supposed to fetch information, do it now. */

	    if( s3vModeTable )
	    {
	        int iRefresh = 0;

		s3vModeTable->Width = vbe.mode->x_resolution;
		s3vModeTable->Height = vbe.mode->y_resolution;
		s3vModeTable->VesaMode = *mode_list;
		
		/* Query the refresh rates at this mode. */

		r.ecx = *mode_list;
		r.edx = 0;

		do
		{
		    r.eax = 0x4f14;
		    r.ebx = 0x0201;		/* query refresh rates */
		    if( !LRMI_int(0x10, &r) )
		    {
			ErrorF( "Can't get refresh rate.\n");
			ioperm( 0x80, 1, 0 );
			return 0;
		    }

		    s3vModeTable->RefreshRate[iRefresh++] = r.edi;
		}
		while( r.edx );

		s3vModeTable->RefreshCount = iRefresh;

	    	s3vModeTable++;
	    }
	}

	mode_list++;
    }

    ioperm( 0x80, 1, 0 );

    LRMI_free_real(vbe.info);

    return iModeCount;
}

#endif
