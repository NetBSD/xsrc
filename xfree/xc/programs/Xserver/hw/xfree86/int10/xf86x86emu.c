/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/xf86x86emu.c,v 1.9 2000/12/06 15:35:26 eich Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */
#include <x86emu.h>
#include "xf86.h"
#include "xf86str.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86_OSproc.h"
#include "xf86Pci.h"
#include "xf86_libc.h"
#define _INT10_PRIVATE
#define _INT10_NO_INOUT_MACROS
#include "xf86int10.h"
#include "int10Defines.h"

#define M            _X86EMU_env

#if defined(PRINT_PORT) || (!defined(_PC) && !defined(_PC_IO))
# define p_inb x_inb
# define p_inw x_inw
# define p_outb x_outb
# define p_outw x_outw
# define p_inl x_inl
# define p_outl x_outl
#else
# define p_inb f_inb
# define p_inw f_inw
# define p_outb f_outb
# define p_outw f_outw
# define p_inl f_inl
# define p_outl f_outl
#endif

/*
 * inb/outb, etc are not available as functions (compler.h) on all
 * platforms (eg SVR4.0 with cc).  This provides versions that are guaranteed
 * to be functions.
 */

static CARD8
f_inb(CARD16 port)
{
    return inb(port);
}

static CARD16
f_inw(CARD16 port)
{
    return inw(port);
}

static CARD32
f_inl(CARD16 port)
{
    return inl(port);
}

static void
f_outb(CARD16 port, CARD8 val)
{
    outb(port, val);
}

static void
f_outw(CARD16 port, CARD16 val)
{
    outw(port,val);
}

static void
f_outl(CARD16 port, CARD32 val)
{
    outl(port,val);
}

static void
x86emu_do_int(int num)
{
    Int10Current->num = num;

    if (!int_handler(Int10Current)) {
	xf86DrvMsg(Int10Current->scrnIndex,
		X_ERROR,"\nUnknown vm86_int: %X\n\n",num);
	X86EMU_halt_sys();
    }
    return;
}
    
void
xf86ExecX86int10(xf86Int10InfoPtr pInt)
{
    int sig = setup_int(pInt);

    if (int_handler(pInt)) {
	X86EMU_exec();	
    }
    
    finish_int(pInt, sig);
}
    
Bool
xf86Int10ExecSetup(xf86Int10InfoPtr pInt)
{
    int i;
    X86EMU_intrFuncs intFuncs[256];
    X86EMU_pioFuncs pioFuncs = {
	(u8(*)(u16))p_inb,
	(u16(*)(u16))p_inw,
	(u32(*)(u16))p_inl,
	(void(*)(u16,u8))p_outb,
	(void(*)(u16,u16))p_outw,
	(void(*)(u16,u32))p_outl
    };
    
    X86EMU_memFuncs memFuncs = {
	(u8(*)(u32))Mem_rb,
	(u16(*)(u32))Mem_rw,
	(u32(*)(u32))Mem_rl,
	(void(*)(u32,u8))Mem_wb,
	(void(*)(u32,u16))Mem_ww,
	(void(*)(u32,u32))Mem_wl
    };

    X86EMU_setupMemFuncs(&memFuncs);
    
    pInt->cpuRegs =  &M;
    M.mem_base = 0;
    M.mem_size = 1024*1024 + 1024;
    X86EMU_setupPioFuncs(&pioFuncs);

    for (i=0;i<256;i++)
	intFuncs[i] = x86emu_do_int;
    X86EMU_setupIntrFuncs(intFuncs);
    return TRUE;
}

void
printk(const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    VErrorF(fmt, argptr);
    va_end(argptr);
}


