/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/helper_exec.c,v 1.11 2000/12/06 15:35:26 eich Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 *
 *   Part of this is based on code taken form DOSEMU
 *   (C) Copyright 1992, ..., 1999 the "DOSEMU-Development-Team"
 */   

/*
 * To debug port accesses define PRINT_PORT.
 * Note! You also have to comment out ioperm()
 * in xf86EnableIO(). Otherwise we won't trap
 * on PIO.
 */
#include "xf86.h"
#include "xf86str.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86Pci.h"
#define _INT10_PRIVATE
#include "int10Defines.h"
#include "xf86int10.h"

#if !defined (_PC) && !defined (_PC_PCI)
static int pciCfg1in(CARD16 addr, CARD32 *val);
static int pciCfg1out(CARD16 addr, CARD32 val);
#endif

#define REG pInt

int
setup_int(xf86Int10InfoPtr pInt)
{
    if (pInt != Int10Current) {
	MapCurrentInt10(pInt);
	Int10Current = pInt;
    }
    X86_EAX = (CARD32) pInt->ax;
    X86_EBX = (CARD32) pInt->bx;
    X86_ECX = (CARD32) pInt->cx;
    X86_EDX = (CARD32) pInt->dx;
    X86_ESI = (CARD32) pInt->si;
    X86_EDI = (CARD32) pInt->di;
    X86_ES  = (CARD32) pInt->es;
    X86_EBP = (CARD32) pInt->bp;
    X86_EIP = 0;
    X86_CS = 0x60;               /* address of 'hlt' */               
    X86_ESP = 0x100;
    X86_SS = 0x30;               /* This is the standard pc bios stack */
    X86_DS = 0x40;               /* standard pc ds */  
    X86_FS = 0;
    X86_GS = 0;
    X86_EFLAGS = (X86_IF_MASK | X86_IOPL_MASK);
   
    return xf86BlockSIGIO();
}

void
finish_int(xf86Int10InfoPtr pInt, int sig)
{
    xf86UnblockSIGIO(sig);
    pInt->ax = (CARD16) X86_EAX;
    pInt->bx = (CARD16) X86_EBX;
    pInt->cx = (CARD16) X86_ECX;
    pInt->dx = (CARD16) X86_EDX;
    pInt->si = (CARD16) X86_ESI;
    pInt->di = (CARD16) X86_EDI;
    pInt->bp = (CARD16) X86_EBP;
    pInt->flags = (CARD16) X86_FLAGS;
}

#define SEG_ADR(type, seg, reg)  type((seg << 4) \
				      + (X86_##reg))
#define SEG_EADR(type, seg, reg)  type((seg << 4) \
				      + (X86_E##reg))
#ifndef _X86EMU
/* get the linear address */
#define LIN_PREF_SI  ((pref_seg << 4) + X86_SI)
#define LWECX	    (prefix66 ^ prefix67 ? X86_ECX : X86_CX)
#define LWECX_ZERO  {if (prefix66 ^ prefix67) X86_ECX = 0; else X86_CX = 0;}
#define DF (1 << 10)


/* vm86 fault handling */
Bool
vm86_GP_fault(xf86Int10InfoPtr pInt)
{
    unsigned char *csp, *lina;
    CARD32 org_eip;
    int pref_seg;
    int done,is_rep,prefix66,prefix67;


    csp = lina = SEG_ADR((unsigned char *), X86_CS, IP);

    is_rep = 0;
    prefix66 = prefix67 = 0;
    pref_seg = -1;

    /* eat up prefixes */
    done = 0;
    do {
	switch (MEM_RB(pInt,(int)csp++)) {
	case 0x66:      /* operand prefix */  prefix66=1; break;
	case 0x67:      /* address prefix */  prefix67=1; break;
	case 0x2e:      /* CS */              pref_seg=X86_CS; break;
	case 0x3e:      /* DS */              pref_seg=X86_DS; break;
	case 0x26:      /* ES */              pref_seg=X86_ES; break;
	case 0x36:      /* SS */              pref_seg=X86_SS; break;
	case 0x65:      /* GS */              pref_seg=X86_GS; break;
	case 0x64:      /* FS */              pref_seg=X86_FS; break;
	case 0xf2:      /* repnz */
	case 0xf3:      /* rep */             is_rep=1; break;
	default: done=1;
	}
    } while (!done);
    csp--;   /* oops one too many */
    org_eip = X86_EIP;
    X86_IP += (csp - lina);

    switch (MEM_RB(pInt,(int)csp)) {
    case 0x6c:                    /* insb */
	/* NOTE: ES can't be overwritten; prefixes 66,67 should use esi,edi,ecx
	 * but is anyone using extended regs in real mode? */
	/* WARNING: no test for DI wrapping! */
	X86_EDI += port_rep_inb(pInt,X86_DX,SEG_EADR((CARD32),X86_ES,DI),
				X86_FLAGS & DF, (is_rep? LWECX:1));
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6d:			/* (rep) insw / insd */
	/* NOTE: ES can't be overwritten */
	/* WARNING: no test for _DI wrapping! */
	if (prefix66) {
	    X86_DI += port_rep_inl(pInt,X86_DX,SEG_ADR((CARD32),X86_ES,DI),
				   X86_EFLAGS & DF, (is_rep? LWECX:1));
	}
	else {
	    X86_DI += port_rep_inw(pInt,X86_DX,SEG_ADR((CARD32),X86_ES,DI),
				   X86_FLAGS & DF, (is_rep? LWECX:1));
	}
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6e:			/* (rep) outsb */
	if (pref_seg < 0) pref_seg = X86_DS;
	/* WARNING: no test for _SI wrapping! */
	X86_SI += port_rep_outb(pInt,X86_DX,(CARD32)LIN_PREF_SI, X86_FLAGS&DF,
			    (is_rep? LWECX:1));
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6f:			/* (rep) outsw / outsd */
	if (pref_seg < 0) pref_seg = X86_DS;
	/* WARNING: no test for _SI wrapping! */
	if (prefix66) {
	    X86_SI += port_rep_outl(pInt,X86_DX,(CARD32)LIN_PREF_SI,
				    X86_EFLAGS&DF, (is_rep? LWECX:1));
	}
	else {
	    X86_SI += port_rep_outw(pInt,X86_DX,(CARD32)LIN_PREF_SI,
				    X86_FLAGS & DF, (is_rep? LWECX:1));
	} 
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0xe5:			/* inw xx, inl xx */
	if (prefix66) X86_EAX = p_inl((int) MEM_RB(pInt,(int)(csp+1)));
	else X86_AX = p_inw((int) (int)(csp[1]));  
	X86_IP += 2;
	break;
    case 0xe4:			/* inb xx */
	X86_AX &= ~(CARD32)0xff;
	X86_AL |= p_inb((int) MEM_RB(pInt,(int)(csp+1)));
	X86_IP += 2;
	break;
    case 0xed:			/* inw dx, inl dx */
	if (prefix66) X86_EAX = p_inl(X86_EDX); 
	else X86_AX = p_inw(X86_DX);
	X86_IP += 1;
	break;
    case 0xec:			/* inb dx */
	X86_AX &= ~(CARD32)0xff;
	X86_AL |= p_inb(X86_DX);
	X86_IP += 1;
	break;

    case 0xe7:			/* outw xx */
	if (prefix66) p_outl((int)MEM_RB(pInt,(int)(csp+1)), X86_EAX);
	else p_outw((int)MEM_RB(pInt,(int)(csp+1)), X86_AX);
	X86_IP += 2;
	break;
    case 0xe6:			/* outb xx */
	p_outb((int) MEM_RB(pInt,(int)(csp+1)), X86_AL);
	X86_IP += 2;
	break;
    case 0xef:			/* outw dx */
	if (prefix66) p_outl(X86_DX, X86_EAX);
	else p_outw(X86_DX, X86_AX);
	X86_IP += 1;
	break;
    case 0xee:			/* outb dx */
	p_outb(X86_DX, X86_AL);
	X86_IP += 1;
	break;

    case 0xf4:
#ifdef DEBUG
	ErrorF("hlt at %p\n", lina);
#endif
	return FALSE;

    case 0x0f: 
	xf86DrvMsg(pInt->scrnIndex,
		X_ERROR,"CPU 0x0f Trap at eip=0x%lx\n",X86_EIP);
	goto op0ferr; 
	break;

    case 0xf0:			/* lock */
    default:
	xf86DrvMsg(pInt->scrnIndex,X_ERROR,"unknown reason for exception\n");
	dump_registers(pInt);
	stack_trace(pInt);

    op0ferr:
	dump_code(pInt);
	xf86DrvMsg(pInt->scrnIndex,X_ERROR,"cannot continue\n");
	return FALSE;
    }				/* end of switch() */
    return TRUE;
}
#endif

/* general software interrupt handler */
CARD32
getIntVect(xf86Int10InfoPtr pInt,int num)
{
    return (MEM_RW(pInt,(num << 2)) + (MEM_RW(pInt,((num << 2) + 2)) << 4));
}

void
pushw(xf86Int10InfoPtr pInt, CARD16 val)
{
    X86_ESP -= 2;
    MEM_WW(pInt,((CARD32) X86_SS << 4) + X86_SP,val);
}

int
run_bios_int(int num, xf86Int10InfoPtr pInt)
{
    CARD32 eflags;
#ifndef _PC
    /* check if bios vector is initialized */
    if (MEM_RW(pInt,(num<<2)+2) == (SYS_BIOS >> 4)) { /* SYS_BIOS_SEG ?*/
#ifdef PRINT_INT
        ErrorF("card BIOS not loaded\n");
#endif
        return 0;
    }
#endif
#ifdef PRINT_INT
    ErrorF("calling card BIOS at: ");
#endif
    eflags = X86_EFLAGS;
#if 0
    eflags = eflags | IF_MASK;
    X86_EFLAGS = X86_EFLAGS  & ~(VIF_MASK | TF_MASK | IF_MASK | NT_MASK);
#endif
    pushw(pInt, eflags);
    pushw(pInt, X86_CS);
    pushw(pInt, (CARD16)X86_EIP);
    X86_CS = MEM_RW(pInt,((num << 2) + 2));
    X86_EIP = (X86_EIP & 0xFFFF0000) | MEM_RW(pInt,(num << 2));
#ifdef PRINT_INT
    ErrorF("0x%x:%lx\n",X86_CS,X86_EIP);
#endif
    return 1;
}

/* Debugging stuff */
void
dump_code(xf86Int10InfoPtr pInt)
{
    int i;
    CARD32 lina = SEG_ADR((CARD32), X86_CS, IP);

    ErrorF("code at 0x%8.8lx: ",lina);
    for (i=0; i<0x10; i++) 
	ErrorF("%2.2x ",MEM_RB(pInt,lina + i));
    ErrorF("\n                    ");
    for (; i<0x20; i++) 
	ErrorF("%2.2x ",MEM_RB(pInt,lina + i));
    ErrorF("\n");
}

#define PRINT(x) ErrorF(#x":%4.4x ",x)
#define PRINT_FLAGS(x) ErrorF(#x":%8.8x ",x)
void
dump_registers(xf86Int10InfoPtr pInt)
{
    PRINT(X86_AX);
    PRINT(X86_BX);
    PRINT(X86_CX);
    PRINT(X86_DX);
    ErrorF("\n");
    PRINT(X86_IP);
    PRINT(X86_SI);
    PRINT(X86_DI);
    PRINT(X86_BP);
    PRINT(X86_SP);
    ErrorF("\n");
    PRINT(X86_CS);
    PRINT(X86_SS);
    PRINT(X86_ES);
    PRINT(X86_DS);
    PRINT(X86_FS);
    PRINT(X86_GS);
    ErrorF("\n");
    PRINT_FLAGS(X86_EFLAGS);
    ErrorF("\n");
}

void
stack_trace(xf86Int10InfoPtr pInt)
{
    int i;
    CARD32 stack = SEG_ADR((CARD32), X86_SS, SP);    

    ErrorF("stack at 0x%8.8lx:\n",stack);
    for (i=0; i < 0x10; i++) 
	ErrorF("%2.2x ",MEM_RB(pInt,stack + i));
    ErrorF("\n");
}

int
port_rep_inb(xf86Int10InfoPtr pInt,
	     CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD32 dst = base;
#ifdef PRINT_PORT
    ErrorF(" rep_insb(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	MEM_WB(pInt,dst,inb(port));
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_inw(xf86Int10InfoPtr pInt,
	     CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -2 : 2;
    CARD32 dst = base;
#ifdef PRINT_PORT    
    ErrorF(" rep_insw(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	MEM_WW(pInt,dst,inw(port));
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_inl(xf86Int10InfoPtr pInt,
	     CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -4 : 4;
    CARD32 dst = base;
#ifdef PRINT_PORT    
    ErrorF(" rep_insl(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	MEM_WL(pInt,dst,inl(port));
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outb(xf86Int10InfoPtr pInt,
	      CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -1 : 1;
    CARD32 dst = base;
#ifdef PRINT_PORT    
    ErrorF(" rep_outb(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	outb(port,MEM_RB(pInt,dst));
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outw(xf86Int10InfoPtr pInt,
	      CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -2 : 2;
    CARD32 dst = base;
#ifdef PRINT_PORT    
    ErrorF(" rep_outw(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	outw(port,MEM_RW(pInt,dst));
	dst += inc;
    }
    return (dst-base);
}

int
port_rep_outl(xf86Int10InfoPtr pInt,
	      CARD16 port, CARD32 base, int d_f, CARD32 count)
{
    register int inc = d_f ? -4 : 4;
    CARD32 dst = base;
#ifdef PRINT_PORT    
    ErrorF(" rep_outl(%#x) %d bytes at %p %s\n",
	     port, count, base, d_f?"up":"down");
#endif
    while (count--) {
	outl(port,MEM_RL(pInt,dst));
	dst += inc;
    }
    return (dst-base);
}

#if defined(PRINT_PORT) || (!defined(_PC) && !defined(_PC_IO))
CARD8
x_inb(CARD16 port)
{
    CARD8 val;

    val = inb(port);
#ifdef PRINT_PORT    
    ErrorF(" inb(%#x) = %2.2x\n",port,val);
#endif
    return val;
}

CARD16
x_inw(CARD16 port)
{
    CARD16 val;

    val = inw(port);
#ifdef PRINT_PORT    
    ErrorF(" inw(%#x) = %4.4x\n",port,val);
#endif
    return val;
}

void
x_outb(CARD16 port, CARD8 val)
{
#ifdef PRINT_PORT    
    ErrorF(" outb(%#x, %2.2x)\n",port,val);
#endif
    outb(port,val);
}

void
x_outw(CARD16 port, CARD16 val)
{
#ifdef PRINT_PORT    
    ErrorF(" outw(%#x, %4.4x)\n",port,val);
#endif
    outw(port,val);
}

CARD32
x_inl(CARD16 port)
{
    CARD32 val;

#if !defined  (_PC) && !defined (_PC_PCI)
    if (!pciCfg1in(port,&val))
#endif
    val = inl(port);

#ifdef PRINT_PORT
    ErrorF(" inl(%#x) = %8.8x\n",port,val);
#endif
    return val;
}

void
x_outl(CARD16 port, CARD32 val)
{
#ifdef PRINT_PORT
    ErrorF(" outl(%#x, %8.8x)\n",port,val);
#endif
#if !defined  (_PC) && !defined (_PC_PCI)
            if (!pciCfg1out(port,val))
#endif
	    outl(port,val);
}
#endif

CARD8
Mem_rb(int addr)
{
    return Int10Current->mem->rb(Int10Current,addr);
}

CARD16
Mem_rw(int addr)
{
    return Int10Current->mem->rw(Int10Current,addr);
}

CARD32
Mem_rl(int addr)
{
    return Int10Current->mem->rl(Int10Current,addr);
}

void
Mem_wb(int addr,CARD8 val)
{
    Int10Current->mem->wb(Int10Current,addr,val);
}

void
Mem_ww(int addr,CARD16 val)
{
    Int10Current->mem->ww(Int10Current,addr,val);
}

void
Mem_wl(int addr,CARD32 val)
{
    Int10Current->mem->wl(Int10Current,addr,val);
}

#if !defined  (_PC) && !defined (_PC_PCI)
static CARD32 PciCfg1Addr = 0;

#define TAG(Cfg1Addr) (Cfg1Addr & 0xffff00)
#define OFFSET(Cfg1Addr) (Cfg1Addr & 0xff)

static int
pciCfg1in(CARD16 addr, CARD32 *val)
{
    if (addr == 0xCF8) {
	*val = PciCfg1Addr;
	return 1;
    }
    else if (addr == 0xCFC) {
	*val = pciReadLong(TAG(PciCfg1Addr), OFFSET(PciCfg1Addr));
	return 1;
    }
    return 0;
}

static int
pciCfg1out(CARD16 addr, CARD32 val)
{
    if (addr == 0xCF8) {
	PciCfg1Addr = val;
	return 1;
    }
    else if (addr == 0xCFC) {
	pciWriteLong(TAG(PciCfg1Addr), OFFSET(PciCfg1Addr),val);
	return 1;
    }
    return 0;
}
#endif

CARD8
bios_checksum(CARD8 *start, int size)
{
    CARD8 sum = 0;

    while (size-- > 0)
	sum += *start++;
    return sum;
}

/*
 * Lock/Unlock legacy VGA. Some Bioses try to be very clever and make
 * an attempt to detect a legacy ISA card. If they find one they might
 * act very strange: for example they might configure the card as a
 * monochrome card. This might cause some drivers to choke.
 * To avoid this we attempt legacy VGA by writing to all know VGA
 * disable registers before we call the BIOS initialization and
 * restore the original values afterwards. In beween we hold our
 * breath. To get to a (possibly exising) ISA card need to disable
 * our current PCI card. 
 */
/*
 * This is just for booting: we just want to catch pure
 * legacy vga therefore we don't worry about mmio etc.
 * This stuff should really go into vgaHW.c. However then
 * the driver would have to load the vga-module prior to
 * doing int10.
 */
void
LockLegacyVGA(int screenIndex,legacyVGAPtr vga)
{
    xf86SetCurrentAccess(FALSE,xf86Screens[screenIndex]);
    vga->save_msr = inb(0x3CC);
    vga->save_vse = inb(0x3C3);
    vga->save_46e8 = inb(0x46e8);
    vga->save_pos102 = inb(0x102);
    outb(0x3C2,~(CARD8)0x03 & vga->save_msr);
    outb(0x3C3,~(CARD8)0x01 & vga->save_vse);
    outb(0x46e8, ~(CARD8)0x08 & vga->save_46e8);
    outb(0x102, ~(CARD8)0x01 & vga->save_pos102);
    xf86SetCurrentAccess(TRUE,xf86Screens[screenIndex]);
}

void
UnlockLegacyVGA(int screenIndex, legacyVGAPtr vga)
{
    xf86SetCurrentAccess(FALSE,xf86Screens[screenIndex]);
    outb(0x102, vga->save_pos102);
    outb(0x46e8, vga->save_46e8);
    outb(0x3C3, vga->save_vse);
    outb(0x3C2, vga->save_msr);
    xf86SetCurrentAccess(TRUE,xf86Screens[screenIndex]);
}
