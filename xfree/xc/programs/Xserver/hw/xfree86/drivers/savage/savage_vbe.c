/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage_vbe.c,v 1.1 2000/12/02 01:16:15 dawes Exp $ */

#include "savage_driver.h"
#include "savage_vbe.h"

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
#define B_O16(x)  (x) 
#define B_O32(x)  (x)
#else
#define B_O16(x)  ((((x) & 0xff) << 8) | (((x) & 0xff) >> 8))
#define B_O32(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) \
                  | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#endif
#define L_ADD(x)  (B_O32(x) & 0xffff) + ((B_O32(x) >> 12) & 0xffff00)

static void
SavageClearVM86Regs( xf86Int10InfoPtr pInt )
{
    pInt->ax = 0;
    pInt->bx = 0;
    pInt->cx = 0;
    pInt->dx = 0;
    pInt->si = 0;
    pInt->di = 0;
    pInt->es = 0xc000;
    pInt->num = 0x10;
}

void
SavageSetTextMode( SavagePtr psav )
{
#if 0
    ioperm( 0x80, 1, 1 );
    ioperm( 0x61, 1, 1 );
    ioperm( 0x40, 4, 1 );
#endif

    SavageClearVM86Regs( psav->pInt10 );

    psav->pInt10->ax = 0x83;

    xf86ExecX86int10( psav->pInt10 );

#if 0
    ioperm( 0x40, 4, 0 );
    ioperm( 0x61, 1, 0 );
    ioperm( 0x80, 1, 1 );
#endif
}


void
SavageSetVESAMode( SavagePtr psav, int n, int Refresh )
{
#if 0
    /* 
     * The Savage BIOS writes to a debug card on port 80h and to the
     * timer chip at port 61.
     */
    ioperm( 0x80, 1, 1 );
    ioperm( 0x61, 1, 1 );
    ioperm( 0x40, 4, 1 );
#endif

    /* First, establish the refresh rate for this mode. */

    SavageClearVM86Regs( psav->pInt10 );

    xf86ExecX86int10( psav->pInt10 );

    psav->pInt10->ax = 0x4f14;	/* S3 extensions */
    psav->pInt10->bx = 0x0001;	/* Set default refresh rate */
    psav->pInt10->cx = n;
    psav->pInt10->di = Refresh;

    xf86ExecX86int10( psav->pInt10 );

    /* Now, make this mode current. */

    SavageClearVM86Regs( psav->pInt10 );
    psav->pInt10->ax = 0x4f02;	/* Set vesa mode extensions */
    psav->pInt10->bx = n;	/* Mode number */

    xf86ExecX86int10( psav->pInt10 );

    if ( (psav->pInt10->ax & 0xff) != 0x4f)
    {
	ErrorF("Set video mode failed\n");
    }

#if 0
    ioperm( 0x40, 4, 0 );
    ioperm( 0x61, 1, 0 );
    ioperm( 0x80, 1, 1 );
#endif
}


void
SavageFreeBIOSModeTable( SavagePtr psav, SavageModeTablePtr* ppTable )
{
    int i;
    SavageModeEntryPtr pMode = (*ppTable)->Modes;

    for( i = (*ppTable)->NumModes; i--; )
    {
	if( pMode->RefreshRate )
	{
	    xfree( pMode->RefreshRate );
	    pMode->RefreshRate = NULL;
	}
    }

    xfree( *ppTable );
}


SavageModeTablePtr
SavageGetBIOSModeTable( SavagePtr psav, int iDepth )
{
    int nModes = SavageGetBIOSModes( psav, iDepth, NULL );
    SavageModeTablePtr pTable;

    pTable = (SavageModeTablePtr) 
	xcalloc( 1, sizeof(SavageModeTableRec) + 
		    (nModes-1) * sizeof(SavageModeEntry) );
    if( pTable ) {
	pTable->NumModes = nModes;
	SavageGetBIOSModes( psav, iDepth, pTable->Modes );
    }

    return pTable;
}


unsigned short
SavageGetBIOSModes( 
    SavagePtr psav,
    int iDepth,
    SavageModeEntryPtr s3vModeTable )
{
    unsigned short iModeCount = 0;
    short int *mode_list;
    pointer vbeLinear = NULL;
    vbeControllerInfoPtr vbe = NULL;
    int vbeReal;
    struct vbe_mode_info_block * vmib;

    if( !psav->pVbe )
	return 0;

    vbe = (vbeControllerInfoPtr) psav->pVbe->memory;
    
    mode_list = xf86int10Addr( psav->pInt10, L_ADD(vbe->VideoModePtr) );

    vbeLinear = xf86Int10AllocPages( psav->pInt10, 1, &vbeReal );
    vmib = (struct vbe_mode_info_block *) vbeLinear;

    while (*mode_list != -1)
    {
	/*
	 * This is a HACK to work around what I believe is a BUG in the
	 * Toshiba Satellite BIOSes in 08/2000 and 09/2000.  The BIOS
	 * table for 1024x600 says it has six refresh rates, when in fact
	 * it only has 3.  This causes the BIOS to go into an infinite
	 * loop until the user interrupts it, usually by pressing
	 * Ctrl-Alt-F1.  For now, we'll just punt everything with a VESA
	 * number greater than or equal to 0200.
	 */
	if( *mode_list >= 0x0200 )
	{
	    mode_list++;
	    continue;
	}

	SavageClearVM86Regs( psav->pInt10 );

	psav->pInt10->ax = 0x4f01;
	psav->pInt10->cx = *mode_list;
	psav->pInt10->es = SEG_ADDR(vbeReal);
	psav->pInt10->di = SEG_OFF(vbeReal);
	psav->pInt10->num = 0x10;

	xf86ExecX86int10( psav->pInt10 );

	if( 
	   (vmib->bits_per_pixel == iDepth) &&
	   (
	      (vmib->memory_model == VBE_MODEL_256) ||
	      (vmib->memory_model == VBE_MODEL_PACKED) ||
	      (vmib->memory_model == VBE_MODEL_RGB)
	   )
	)
	{
	    /* This mode is a match. */

	    iModeCount++;

	    /* If we're supposed to fetch information, do it now. */

	    if( s3vModeTable )
	    {
	        int iRefresh = 0;

		s3vModeTable->Width = vmib->x_resolution;
		s3vModeTable->Height = vmib->y_resolution;
		s3vModeTable->VesaMode = *mode_list;
		
		/* Query the refresh rates at this mode. */

		psav->pInt10->cx = *mode_list;
		psav->pInt10->dx = 0;

		do
		{
		    if( (iRefresh % 8) == 0 )
		    {
			if( s3vModeTable->RefreshRate )
			{
			    s3vModeTable->RefreshRate = (unsigned char *)
				xrealloc( 
				    s3vModeTable->RefreshRate,
				    (iRefresh+8) * sizeof(unsigned char)
				);
			}
			else
			{
			    s3vModeTable->RefreshRate = (unsigned char *)
				xcalloc( 
				    sizeof(unsigned char),
				    (iRefresh+8)
				);
			}
		    }

		    psav->pInt10->ax = 0x4f14;	/* S3 extended functions */
		    psav->pInt10->bx = 0x0201;	/* query refresh rates */
		    psav->pInt10->num = 0x10;
		    xf86ExecX86int10( psav->pInt10 );

		    s3vModeTable->RefreshRate[iRefresh++] = psav->pInt10->di;
		}
		while( psav->pInt10->dx );

		s3vModeTable->RefreshCount = iRefresh;

	    	s3vModeTable++;
	    }
	}

	mode_list++;
    }

    xf86Int10FreePages( psav->pInt10, vbeLinear, 1 );

    return iModeCount;
}
