/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/xf86int10.c,v 1.5 2000/04/04 19:25:18 dawes Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */

#include "xf86.h"
#include "xf86str.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86Pci.h"
#define _INT10_PRIVATE
#include "xf86int10.h"
#include "int10Defines.h"

xf86Int10InfoPtr Int10Current = NULL;

static int int1A_handler(xf86Int10InfoPtr pInt);
static int int42_handler(xf86Int10InfoPtr pInt);
static int intE6_handler(xf86Int10InfoPtr pInt);
static PCITAG findPci(unsigned short bx);
static CARD32 pciSlotBX(pciVideoPtr pvp);

int
int_handler(xf86Int10InfoPtr pInt)
{
    int num = pInt->num;

    switch (num) {
    case 0x10:
    case 0x42:
	if (!(int42_handler(pInt)))
	    goto bios_handler;
	else return 1;
    case 0x1A:
	if(!(int1A_handler(pInt)))
	    goto bios_handler;
	else return 1;
    case 0xe6:
	if (!(intE6_handler(pInt)))
	    goto bios_handler;
	else return 1;
    default:
	goto bios_handler;
    }
 bios_handler:
    return run_bios_int(num,pInt);
}

/*
 * The system-BIOS provides int10 ax=1200 and ax=1201 functions
 * before the video bios is installed. The int10_handler below
 * provides these functions, too. However there have been cases
 * in which disabling generic video has caused problems. Therefore
 * it has been disabled by default. To reenable it do:
 * #define DO_GENERIC_INT10
 */
static int
int42_handler(xf86Int10InfoPtr pInt)
{
#define REG pInt
#ifdef DO_GENERIC_INT10
    unsigned char c;
#endif
    int num = pInt->num;
#ifdef PRINT_INT
    ErrorF("int 0x%x: ax:0x%x bx:0x%x cx:0x%x dx:0x%x\n",num,
	   X86_EAX,X86_EBX,X86_ECX,X86_EDX);
#endif
    /*
     * video bios has modified these -
     * leave it to the video bios to do this
     */

    if (getIntVect(pInt,num) != I_S_DEFAULT_INT_VECT) {
	return 0;
    }
    
    if ((X86_EBX & 0xff) == 0x32) {
	switch (X86_EAX & 0xFFFF) {
	case 0x1200:
#ifdef PRINT_INT
	    ErrorF("enabling video\n");
#endif
#ifdef DO_GENERIC_INT10
	    c = inb(0x3cc);
	    c |= 0x02;
	    outb(0x3c2,c);
#endif
	    return 1;
	case 0x1201:
#ifdef PRINT_INT
	    ErrorF("disabling video\n");
#endif
#ifdef DO_GENERIC_INT10
	    c = inb(0x3cc);
	    c &= ~0x02;
	    outb(0x3c2,c);
#endif
	    return 1;
	default:
	    break;
	}
    }
    if (num == 0x42)
	return 1;
    else
	return 0;
}

#define SUCCESSFUL              0x00
#define DEVICE_NOT_FOUND        0x86
#define BAD_REGISTER_NUMBER     0x87

static int
int1A_handler(xf86Int10InfoPtr pInt)
{
    PCITAG tag;
    pciVideoPtr pvp;
    
    if (! (pvp = xf86GetPciInfoForEntity(pInt->entityIndex)))
	return 0; /* oops */

#ifdef PRINT_INT
    ErrorF("int 0x1a: ax=0x%x bx=0x%x cx=0x%x dx=0x%x di=0x%x es=0x%x\n",
	    X86_EAX,X86_EBX,X86_ECX,X86_EDX,X86_EDI,X86_ESI);
#endif
    switch (X86_EAX & 0xFFFF) {
    case 0xb101:
	X86_EAX  &= 0xFF00;   /* no config space/special cycle support */
	X86_EDX = 0x20494350; /* " ICP" */
	X86_EBX  = 0x0210;    /* Version 2.10 */
	X86_ECX  &= 0xFF00;
	X86_ECX |= (pciNumBuses & 0xFF);   /* Max bus number in system */
	X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
#ifdef PRINT_INT
	ErrorF("ax=0x%x dx=0x%x bx=0x%x cx=0x%x flags=0x%x\n",
		 X86_EAX,X86_EDX,X86_EBX,X86_ECX,X86_EFLAGS);
#endif
	return 1;
    case 0xb102:
	if ((X86_EDX & 0xFFFF) == pvp->vendor &&
	    (X86_ECX & 0xFFFF) ==pvp->chipType &&
	    X86_ESI == 0) {
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp); 
	}
#ifdef SHOW_ALL_DEVICES
	else if ((pvp = xf86FindPciDeviceVendor(X86_EDX,X86_ECX,X86_ESI,pvp))
		 != NULL) {
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp);
	}
#endif
	else {
	    X86_EAX = (X86_EAX & 0x00FF) | (DEVICE_NOT_FOUND << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x bx=0x%x flags=0x%x\n",
		 X86_EAX,X86_EBX,X86_EFLAGS);
#endif
	return 1;
    case 0xb103:
	if ((X86_ECX & 0xFF) == pvp->interface &&
	    ((X86_ECX & 0xFF00) >> 8) == pvp->subclass &&
	    ((X86_ECX & 0xFFFF0000) >> 16) == pvp->class) {
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EBX = pciSlotBX(pvp);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	}
#ifdef SHOW_ALL_DEVICES
	else if ((pvp = xf86FindPciClass(X86_ECX & 0xFF,
					 (X86_ECX & 0xff00) >> 8,
					 (X86_ECX & 0xffff0000) >> 16,
					 X86_ESI,pvp))!= NULL) {
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp);
	}
#endif
	else {
	    X86_EAX = (X86_EAX & 0x00FF) | (DEVICE_NOT_FOUND << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n",X86_EAX,X86_EFLAGS);
#endif
	return 1;
    case 0xb108:
	if ((tag = findPci(X86_EBX))) {
	    X86_ECX &= 0xFFFFFF00;
	    X86_ECX |= pciReadByte(tag,X86_EDI); 
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n",
		 X86_EAX,X86_ECX,X86_EFLAGS);
#endif
	return 1;
    case 0xb109:
	if ((tag = findPci(X86_EBX))) {
	    X86_ECX &= 0xFFFF0000;
	    X86_ECX |= pciReadWord(tag,X86_EDI);
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n",
		 X86_EAX,X86_ECX,X86_EFLAGS);
#endif
	return 1;
    case 0xb10a:
	if ((tag = findPci(X86_EBX))) {
	    X86_ECX &= 0;
	    X86_ECX |= pciReadLong(tag, X86_EDI);
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n",
		 X86_EAX,X86_ECX,X86_EFLAGS);
#endif
	return 1;
    case 0xb10b:
	if ((tag = findPci(X86_EBX))) {
	    pciWriteByte(tag,X86_EDI,(CARD8)X86_ECX);
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX,X86_EFLAGS);
#endif
	return 1;
    case 0xb10c:
	if ((tag = findPci(X86_EBX))) {
	    pciWriteWord(tag,X86_EDI,(CARD16)X86_ECX);
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX,X86_EFLAGS);
#endif
	return 1;
    case 0xb10d:
	if ((tag = findPci(X86_EBX))) {
	    pciWriteLong(tag,X86_EDI,(CARD32)X86_ECX);
	    X86_EAX = (X86_EAX & 0x00FF) | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = (X86_EAX & 0x00FF) | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX,X86_EFLAGS);
#endif
	return 1;
    default:
	return 0;
    }
}

static PCITAG
findPci(unsigned short bx)
{
    int bus = (bx >> 8) & 0xFF;
    int dev = (bx >> 3) & 0x1F;
    int func = bx & 0x7;
    if (xf86IsPciDevPresent(bus,dev,func))
	return pciTag(bus,dev,func);
    return 0;
}

static CARD32
pciSlotBX(pciVideoPtr pvp)
{
    return ((pvp->bus << 8) | (pvp->device << 3) | (pvp->func));
}

/*
 * handle initialization
 */
static int
intE6_handler(xf86Int10InfoPtr pInt)
{
    pciVideoPtr pvp;

    if ((pvp = xf86GetPciInfoForEntity(pInt->entityIndex))) {
	X86_AX = (CARD16)(((pvp->bus) << 8)
		      | (pvp->device << 3) | (pvp->func & 0x7));
    }
    pushw(pInt,X86_CS);
    pushw(pInt,(CARD16)X86_EIP);
    X86_CS = pInt->BIOSseg;
    X86_EIP = 0x0003;
    X86_ES = 0;                  /* standard pc es */
    return 1;
}
