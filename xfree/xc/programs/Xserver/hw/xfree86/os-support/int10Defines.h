/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/int10Defines.h,v 1.1 2000/01/23 04:44:35 dawes Exp $ */
#ifdef _VM86_LINUX

#include <asm/vm86.h>

#define CPU_R(type,name) \
        (*((type *)&(((struct vm86_struct *)REG->cpuRegs)->regs.##name)))
#define CPU_RD(name) CPU_R(CARD32,name)
#define CPU_RW(name) CPU_R(CARD16,name)
#define CPU_RB(name) CPU_R(CARD8,name)
	   
#define X86_EAX CPU_RD(eax)
#define X86_EBX CPU_RD(ebx)
#define X86_ECX CPU_RD(ecx)
#define X86_EDX CPU_RD(edx)
#define X86_ESI CPU_RD(esi)
#define X86_EDI CPU_RD(edi)
#define X86_EBP CPU_RD(ebp)
#define X86_EIP CPU_RD(eip)
#define X86_ESP CPU_RD(esp)
#define X86_EFLAGS CPU_RD(eflags)

#define X86_FLAGS CPU_RW(eflags)
#define X86_AX CPU_RW(eax)
#define X86_BX CPU_RW(ebx)
#define X86_CX CPU_RW(ecx)
#define X86_DX CPU_RW(edx)
#define X86_SI CPU_RW(esi)
#define X86_DI CPU_RW(edi)
#define X86_BP CPU_RW(ebp)
#define X86_IP CPU_RW(eip)
#define X86_SP CPU_RW(esp)
#define X86_CS CPU_RW(cs)
#define X86_DS CPU_RW(ds)
#define X86_ES CPU_RW(es)
#define X86_SS CPU_RW(ss)
#define X86_FS CPU_RW(fs)
#define X86_GS CPU_RW(gs)

#define X86_AL CPU_RB(eax)
#define X86_BL CPU_RB(ebx)
#define X86_CL CPU_RB(ecx)
#define X86_DL CPU_RB(edx)

#elif defined(_X86EMU)

#include "xf86x86emu.h"

#endif

