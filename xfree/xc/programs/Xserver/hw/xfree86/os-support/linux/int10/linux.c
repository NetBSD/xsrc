/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/int10/linux.c,v 1.24 2001/05/15 10:19:42 eich Exp $ */
/*
 * linux specific part of the int10 module
 * Copyright 1999 Egbert Eich
 */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "compiler.h"
#define _INT10_PRIVATE
#include "xf86int10.h"
#include "int10Defines.h"
#ifdef __sparc__
#define DEV_MEM "/dev/fb"
#else
#define DEV_MEM "/dev/mem"
#endif
#ifndef XFree86LOADER
#include <sys/mman.h>
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif
#endif
#define ALLOC_ENTRIES(x) ((V_RAM / x) - 1)
#define REG pInt
#define SHMERRORPTR (pointer)(-1)

static int counter = 0;

static CARD8 read_b(xf86Int10InfoPtr pInt, int addr);
static CARD16 read_w(xf86Int10InfoPtr pInt, int addr);
static CARD32 read_l(xf86Int10InfoPtr pInt, int addr);
static void write_b(xf86Int10InfoPtr pInt, int addr, CARD8 val);
static void write_w(xf86Int10InfoPtr pInt, int addr, CARD16 val);
static void write_l(xf86Int10InfoPtr pInt, int addr, CARD32 val);

int10MemRec linuxMem = {
    read_b,
    read_w,
    read_l,
    write_b,
    write_w,
    write_l
};

typedef struct {
    int lowMem;
    int highMem;
    char* base;
    char* base_high;
    int screen;
    char* alloc;
} linuxInt10Priv;

xf86Int10InfoPtr
xf86InitInt10(int entityIndex)
{
    xf86Int10InfoPtr pInt = NULL;
    CARD8 *bios_base;
    int screen;
    int fd;
    static void* vidMem = NULL;
    static void* sysMem = NULL;
    void *options = NULL;
    int low_mem;
    int high_mem;
    char *base = SHMERRORPTR;
    char *base_high = SHMERRORPTR;
    int pagesize, cs;
    legacyVGARec vga;
    xf86int10BiosLocation bios;
    
    screen = (xf86FindScreenForEntity(entityIndex))->scrnIndex;

    options = xf86HandleInt10Options(xf86Screens[screen],entityIndex);

    if (int10skip(options))
	return NULL;

    if ((!vidMem) || (!sysMem)) {
	if ((fd = open(DEV_MEM, O_RDWR, 0)) >= 0) {
	    if (!sysMem) {
#ifdef DEBUG
		ErrorF("Mapping sys bios area\n");
#endif
		if ((sysMem = mmap((void *)(SYS_BIOS), BIOS_SIZE,
				   PROT_READ | PROT_WRITE | PROT_EXEC,
				   MAP_SHARED | MAP_FIXED, fd, SYS_BIOS))
		    == MAP_FAILED) {
		    xf86DrvMsg(screen, X_ERROR, "Cannot map SYS BIOS\n");
		    close(fd);
		    goto error0;
		}
	    }
	    if (!vidMem) {
#ifdef DEBUG
		ErrorF("Mapping VRAM area\n");
#endif
		if ((vidMem = mmap((void *)(V_RAM), VRAM_SIZE,
				   PROT_READ | PROT_WRITE | PROT_EXEC,
				   MAP_SHARED | MAP_FIXED, fd, V_RAM))
		    == MAP_FAILED) {
		    xf86DrvMsg(screen, X_ERROR, "Cannot map V_RAM\n");
		    close(fd);
		    goto error0;
		}
	    }
	    close(fd);
	} else {
	    xf86DrvMsg(screen, X_ERROR, "Cannot open %s\n", DEV_MEM);
	    goto error0;
	}
    }

    pInt = (xf86Int10InfoPtr)xnfcalloc(1, sizeof(xf86Int10InfoRec));
    pInt->scrnIndex = screen;
    pInt->entityIndex = entityIndex;
    if (!xf86Int10ExecSetup(pInt))
	goto error0;
    pInt->mem = &linuxMem;
    pagesize = getpagesize();
    pInt->private = (pointer)xnfcalloc(1, sizeof(linuxInt10Priv));
    ((linuxInt10Priv*)pInt->private)->screen = screen;
    ((linuxInt10Priv*)pInt->private)->alloc =
	(pointer)xnfcalloc(1, ALLOC_ENTRIES(pagesize));

#ifdef DEBUG
    ErrorF("Mapping high memory area\n");
#endif
    if ((high_mem = shmget(counter++, HIGH_MEM_SIZE,
			       IPC_CREAT | SHM_R | SHM_W)) == -1) {
	if (errno == ENOSYS)
	    xf86DrvMsg(screen, X_ERROR, "shmget error\n Please reconfigure"
		       " your kernel to include System V IPC support\n");
	goto error1;
    }
    ((linuxInt10Priv*)pInt->private)->highMem = high_mem;
#ifdef DEBUG
    ErrorF("Mapping 640kB area\n");
#endif
    if ((low_mem = shmget(counter++, V_RAM,
			      IPC_CREAT | SHM_R | SHM_W)) == -1)
	goto error2;

    ((linuxInt10Priv*)pInt->private)->lowMem = low_mem;
    base = shmat(low_mem, 0, 0);
    if (base == SHMERRORPTR) goto error4;
    ((linuxInt10Priv *)pInt->private)->base = base;
    base_high = shmat(high_mem, 0, 0);
    if (base_high == SHMERRORPTR) goto error4;
    ((linuxInt10Priv*)pInt->private)->base_high = base_high;

    MapCurrentInt10(pInt);
    Int10Current = pInt;

#ifdef DEBUG
    ErrorF("Mapping int area\n");
#endif
    if (xf86ReadBIOS(0, 0, (unsigned char *)0, LOW_PAGE_SIZE) < 0) {
	xf86DrvMsg(screen, X_ERROR, "Cannot read int vect\n");
	goto error3;
    }

    /*
     * Read in everything between V_BIOS and SYS_BIOS as some system BIOSes
     * have executable code there.  Note that xf86ReadBIOS() can only bring in
     * 64K bytes at a time.
     */
    (void)memset((pointer)V_BIOS, 0, SYS_BIOS - V_BIOS);
    for (cs = V_BIOS;  cs < SYS_BIOS;  cs += V_BIOS_SIZE)
	if (xf86ReadBIOS(cs, 0, (pointer)cs, V_BIOS_SIZE) < V_BIOS_SIZE)
	    xf86DrvMsg(screen, X_WARNING,
		"Unable to retrieve all of segment 0x%06X.\n", cs);

    xf86int10ParseBiosLocation(options,&bios);

    if (xf86IsEntityPrimary(entityIndex) 
	&& !(initPrimary(options))) {
	if (bios.bus == BUS_ISA && bios.location.legacy) {
	    xf86DrvMsg(screen, X_CONFIG,
		       "Overriding BIOS location: 0x%lx\n",
		       bios.location.legacy);
	    cs = bios.location.legacy >> 4;
	    bios_base = (unsigned char *)(cs << 4);
	    if (!int10_check_bios(screen, cs, bios_base)) {
		xf86DrvMsg(screen, X_ERROR,
			   "No V_BIOS at specified address 0x%x\n",cs << 4);
		goto error3;
	    }
	} else {
	    if (bios.bus == BUS_PCI) {
		xf86DrvMsg(screen, X_WARNING,
			   "Option BiosLocation for primary device ignored: "
			   "It points to PCI.\n");
		xf86DrvMsg(screen, X_WARNING,
			   "You must set Option InitPrimary also\n");
	    }
	    
	    cs = ((CARD16*)0)[(0x10<<1) + 1];

	    bios_base = (unsigned char *)(cs << 4);

	    if (!int10_check_bios(screen, cs, bios_base)) {
		cs = ((CARD16*)0)[(0x42 << 1) + 1];
		bios_base = (unsigned char *)(cs << 4);
		if (!int10_check_bios(screen, cs, bios_base)) {
		    cs = V_BIOS >> 4;
		    bios_base = (unsigned char *)(cs << 4);
		    if (!int10_check_bios(screen, cs, bios_base)) {
			xf86DrvMsg(screen, X_ERROR, "No V_BIOS found\n");
			goto error3;
		    }
		}
	    }
	}
	
	xf86DrvMsg(screen, X_INFO, "Primary V_BIOS segment is: 0x%x\n", cs);

	pInt->BIOSseg = cs;
	set_return_trap(pInt);
    } else {
        EntityInfoPtr pEnt = xf86GetEntityInfo(pInt->entityIndex);
	BusType location_type;

	if (bios.bus != BUS_NONE) {
	    switch (location_type = bios.bus) {
	    case BUS_PCI:
		xf86DrvMsg(screen,X_CONFIG,"Overriding bios location: "
			   "PCI:%i:%i%i\n",bios.location.pci.bus,
			   bios.location.pci.dev,bios.location.pci.func);
		break;
	    case BUS_ISA:
		if (bios.location.legacy)
		    xf86DrvMsg(screen,X_CONFIG,"Overriding bios location: "
			       "Legacy:0x%x\n",bios.location.legacy);
		else
		    xf86DrvMsg(screen,X_CONFIG,"Overriding bios location: "
			       "Legacy\n");
		break;
	    default:
		break;
	    }
	} else
	    location_type = pEnt->location.type;

	switch (location_type) {
	case BUS_PCI:
	{
	    int pci_entity;
	    
	    if (bios.bus == BUS_PCI)
		pci_entity = xf86GetPciEntity(bios.location.pci.bus,
					      bios.location.pci.dev,
					      bios.location.pci.func);
	    else 
		pci_entity = pInt->entityIndex;

	    if (!mapPciRom(pci_entity, (unsigned char *)(V_BIOS))) {
	        xf86DrvMsg(screen, X_ERROR, "Cannot read V_BIOS\n");
		goto error3;
	    }
	    pInt->BIOSseg = V_BIOS >> 4;
	    break;
	}
	case BUS_ISA:
	    if (bios.bus == BUS_ISA && bios.location.legacy) {
		cs = bios.location.legacy >> 4;
		bios_base = (unsigned char *)(cs << 4);
		if (!int10_check_bios(screen, cs, bios_base)) {
		    xf86DrvMsg(screen,X_ERROR,"No V_BIOS found "
			       "on override address 0x%x\n",bios_base);
		    goto error3;
		}
	    } else {
		cs = ((CARD16*)0)[(0x10<<1)+1];
		bios_base = (unsigned char *)(cs << 4);
		
		if (!int10_check_bios(screen, cs, bios_base)) {
		    cs = ((CARD16*)0)[(0x42<<1)+1];
		    bios_base = (unsigned char *)(cs << 4);
		    if (!int10_check_bios(screen, cs, bios_base)) {
			cs = V_BIOS >> 4;
			bios_base = (unsigned char *)(cs << 4);
			if (!int10_check_bios(screen, cs, bios_base)) {
			    xf86DrvMsg(screen,X_ERROR,"No V_BIOS found\n");
			    goto error3;
			}
		    }
		}
	    }
	    xf86DrvMsg(screen,X_INFO,"Primary V_BIOS segment is: 0x%x\n",cs);
	    pInt->BIOSseg = cs;
	    break;
	default:
	    goto error3;
	}
	xfree(pEnt);
	pInt->num = 0xe6;
	reset_int_vect(pInt);
	set_return_trap(pInt);
	LockLegacyVGA(screen, &vga);
	xf86ExecX86int10(pInt);
	UnlockLegacyVGA(screen, &vga);
    }
#ifdef DEBUG
    dprint(0xc0000, 0x20);
#endif

    return pInt;

error4:
    xf86DrvMsg(screen, X_ERROR, "shmat() call retruned errno %d\n", errno);
error3:
    shmdt(base_high);
    shmdt(base);
    shmdt(0);
    shmdt((char*)HIGH_MEM);
    shmctl(low_mem, IPC_RMID, NULL);
    Int10Current = NULL;
error2:
    shmctl(high_mem, IPC_RMID,NULL);
error1:
    xfree(((linuxInt10Priv*)pInt->private)->alloc);
    xfree(pInt->private);
error0:
    xfree(pInt);
    return NULL;
}

Bool
MapCurrentInt10(xf86Int10InfoPtr pInt)
{
    pointer addr;

    if (Int10Current) {
	shmdt(0);
	shmdt((char*)HIGH_MEM);
    }
    addr = shmat(((linuxInt10Priv*)pInt->private)->lowMem, (char*)1, SHM_RND);
    if (addr == SHMERRORPTR) {
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "Cannot shmat() low memory\n");
	return FALSE;
    }
    addr = shmat(((linuxInt10Priv*)pInt->private)->highMem, (char*)HIGH_MEM, 0);
    if (addr == SHMERRORPTR) {
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "Cannot shmat() high memory\n");
	return FALSE;
    }
    return TRUE;
}

void
xf86FreeInt10(xf86Int10InfoPtr pInt)
{
    if (!pInt)
	return;
    if (Int10Current == pInt) {
	shmdt(0);
	shmdt((char*)HIGH_MEM);
	Int10Current = NULL;
    }
    shmdt(((linuxInt10Priv*)pInt->private)->base_high);
    shmdt(((linuxInt10Priv*)pInt->private)->base);
    shmctl(((linuxInt10Priv*)pInt->private)->lowMem, IPC_RMID, NULL);
    shmctl(((linuxInt10Priv*)pInt->private)->highMem, IPC_RMID, NULL);
    xfree(((linuxInt10Priv*)pInt->private)->alloc);
    xfree(pInt->private);
    xfree(pInt->cpuRegs);
    xfree(pInt);
}

void *
xf86Int10AllocPages(xf86Int10InfoPtr pInt, int num, int *off)
{
    int pagesize = getpagesize();
    int num_pages = ALLOC_ENTRIES(pagesize);
    int i, j;

    for (i = 0; i < (num_pages - num); i++) {
	if (((linuxInt10Priv*)pInt->private)->alloc[i] == 0) {
	    for (j = i; j < (num + i); j++)
		if ((((linuxInt10Priv*)pInt->private)->alloc[j] != 0))
		    break;
	    if (j == (num + i))
		break;
	    else
		i = i + num;
	}
    }
    if (i == (num_pages - num))
	return NULL;

    for (j = i; j < (i + num); j++)
	((linuxInt10Priv*)pInt->private)->alloc[j] = 1;

    *off = (i + 1) * pagesize;

    return ((linuxInt10Priv*)pInt->private)->base + ((i + 1) * pagesize);
}

void
xf86Int10FreePages(xf86Int10InfoPtr pInt, void *pbase, int num)
{
    int pagesize = getpagesize();
    int first = (((unsigned long)pbase
		 - (unsigned long)((linuxInt10Priv*)pInt->private)->base)
	/ pagesize) - 1;
    int i;

    for (i = first; i < (first + num); i++)
	((linuxInt10Priv*)pInt->private)->alloc[i] = 0;
}

static CARD8
read_b(xf86Int10InfoPtr pInt, int addr)
{
    return *((CARD8 *)addr);
}

static CARD16
read_w(xf86Int10InfoPtr pInt, int addr)
{
    return *((CARD16 *)addr);
}

static CARD32
read_l(xf86Int10InfoPtr pInt, int addr)
{
    return *((CARD32 *)addr);
}

static void
write_b(xf86Int10InfoPtr pInt, int addr, CARD8 val)
{
    *((CARD8 *)addr) = val;
}

static void
write_w(xf86Int10InfoPtr pInt, int addr, CARD16 val)
{
    *((CARD16 *)addr) = val;
}

static
void write_l(xf86Int10InfoPtr pInt, int addr, CARD32 val)
{
    *((CARD32 *)addr) = val;
}

pointer
xf86int10Addr(xf86Int10InfoPtr pInt, CARD32 addr)
{
    if (addr < V_RAM)
	return ((linuxInt10Priv*)pInt->private)->base + addr;
    else if (addr < V_BIOS)
	return (pointer)addr;
    else if (addr < SYS_BIOS)
	return (pointer)(((linuxInt10Priv*)pInt->private)->base_high
			  - V_BIOS + addr);
    else
	return (pointer)addr;
}

#ifdef _VM86_LINUX

static int vm86_rep(struct vm86_struct *ptr);

Bool
xf86Int10ExecSetup(xf86Int10InfoPtr pInt)
{
#define VM86S ((struct vm86_struct *)pInt->cpuRegs)

    pInt->cpuRegs = (pointer)xnfcalloc(1, sizeof(struct vm86_struct));
    VM86S->flags = 0;
    VM86S->screen_bitmap = 0;
    VM86S->cpu_type = CPU_586;
    memset(&VM86S->int_revectored, 0xff, sizeof(VM86S->int_revectored));
    memset(&VM86S->int21_revectored, 0xff, sizeof(VM86S->int21_revectored));
    return TRUE;
}

/* get the linear address */
#define LIN_PREF_SI ((pref_seg << 4) + X86_SI)
#define LWECX       ((prefix66 ^ prefix67) ? X86_ECX : X86_CX)
#define LWECX_ZERO  {if (prefix66 ^ prefix67) X86_ECX = 0; else X86_CX = 0;}
#define DF (1 << 10)

/* vm86 fault handling */
static Bool
vm86_GP_fault(xf86Int10InfoPtr pInt)
{
    unsigned char *csp, *lina;
    CARD32 org_eip;
    int pref_seg;
    int done, is_rep, prefix66, prefix67;

    csp = lina = SEG_ADR((unsigned char *), X86_CS, IP);

    is_rep = 0;
    prefix66 = prefix67 = 0;
    pref_seg = -1;

    /* eat up prefixes */
    done = 0;
    do {
	switch (MEM_RB(pInt, (int)csp++)) {
	case 0x66:      /* operand prefix */  prefix66=1; break;
	case 0x67:      /* address prefix */  prefix67=1; break;
	case 0x2e:      /* CS */              pref_seg=X86_CS; break;
	case 0x3e:      /* DS */              pref_seg=X86_DS; break;
	case 0x26:      /* ES */              pref_seg=X86_ES; break;
	case 0x36:      /* SS */              pref_seg=X86_SS; break;
	case 0x65:      /* GS */              pref_seg=X86_GS; break;
	case 0x64:      /* FS */              pref_seg=X86_FS; break;
	case 0xf0:      /* lock */            break;
	case 0xf2:      /* repnz */
	case 0xf3:      /* rep */             is_rep=1; break;
	default: done=1;
	}
    } while (!done);
    csp--;   /* oops one too many */
    org_eip = X86_EIP;
    X86_IP += (csp - lina);

    switch (MEM_RB(pInt, (int)csp)) {
    case 0x6c:                    /* insb */
	/* NOTE: ES can't be overwritten; prefixes 66,67 should use esi,edi,ecx
	 * but is anyone using extended regs in real mode? */
	/* WARNING: no test for DI wrapping! */
	X86_EDI += port_rep_inb(pInt, X86_DX, SEG_EADR((CARD32), X86_ES, DI),
				X86_FLAGS & DF, is_rep ? LWECX : 1);
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6d:                  /* (rep) insw / insd */
	/* NOTE: ES can't be overwritten */
	/* WARNING: no test for _DI wrapping! */
	if (prefix66) {
	    X86_DI += port_rep_inl(pInt, X86_DX, SEG_ADR((CARD32), X86_ES, DI),
				   X86_EFLAGS & DF, is_rep ? LWECX : 1);
	}
	else {
	    X86_DI += port_rep_inw(pInt, X86_DX, SEG_ADR((CARD32), X86_ES, DI),
				   X86_FLAGS & DF, is_rep ? LWECX : 1);
	}
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6e:                  /* (rep) outsb */
	if (pref_seg < 0) pref_seg = X86_DS;
	/* WARNING: no test for _SI wrapping! */
	X86_SI += port_rep_outb(pInt, X86_DX, (CARD32)LIN_PREF_SI,
			        X86_FLAGS & DF, is_rep ? LWECX : 1);
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0x6f:                  /* (rep) outsw / outsd */
	if (pref_seg < 0) pref_seg = X86_DS;
	/* WARNING: no test for _SI wrapping! */
	if (prefix66) {
	    X86_SI += port_rep_outl(pInt, X86_DX, (CARD32)LIN_PREF_SI,
				    X86_EFLAGS & DF, is_rep ? LWECX : 1);
	}
	else {
	    X86_SI += port_rep_outw(pInt, X86_DX, (CARD32)LIN_PREF_SI,
				    X86_FLAGS & DF, is_rep ? LWECX : 1);
	}
	if (is_rep) LWECX_ZERO;
	X86_IP++;
	break;

    case 0xe5:                  /* inw xx, inl xx */
	if (prefix66) X86_EAX = x_inl(csp[1]);
	else X86_AX = x_inw(csp[1]);
	X86_IP += 2;
	break;

    case 0xe4:                  /* inb xx */
	X86_AL = x_inb(csp[1]);
	X86_IP += 2;
	break;

    case 0xed:                  /* inw dx, inl dx */
	if (prefix66) X86_EAX = x_inl(X86_DX);
	else X86_AX = x_inw(X86_DX);
	X86_IP += 1;
	break;

    case 0xec:                  /* inb dx */
	X86_AL = x_inb(X86_DX);
	X86_IP += 1;
	break;

    case 0xe7:                  /* outw xx */
	if (prefix66) x_outl(csp[1], X86_EAX);
	else x_outw(csp[1], X86_AX);
	X86_IP += 2;
	break;

    case 0xe6:                  /* outb xx */
	x_outb(csp[1], X86_AL);
	X86_IP += 2;
	break;

    case 0xef:                  /* outw dx */
	if (prefix66) x_outl(X86_DX, X86_EAX);
	else x_outw(X86_DX, X86_AX);
	X86_IP += 1;
	break;

    case 0xee:                  /* outb dx */
	x_outb(X86_DX, X86_AL);
	X86_IP += 1;
	break;

    case 0xf4:
#ifdef DEBUG
	ErrorF("hlt at %p\n", lina);
#endif
	return FALSE;

    case 0x0f:
	xf86DrvMsg(pInt->scrnIndex, X_ERROR,
	    "CPU 0x0f Trap at CS:EIP=0x%4.4x:0x%8.8x\n", X86_CS, X86_EIP);
	goto op0ferr;

    default:
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "unknown reason for exception\n");

    op0ferr:
	dump_registers(pInt);
	stack_trace(pInt);
	dump_code(pInt);
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "cannot continue\n");
	return FALSE;
    }                           /* end of switch() */
    return TRUE;
}

static int
do_vm86(xf86Int10InfoPtr pInt)
{
    int retval, signo;

    xf86InterceptSignals(&signo);
    retval = vm86_rep(VM86S);
    xf86InterceptSignals(NULL);

    if (signo >= 0) {
	xf86DrvMsg(pInt->scrnIndex, X_ERROR,
	    "vm86() syscall generated signal %d.\n", signo);
	dump_registers(pInt);
	dump_code(pInt);
	stack_trace(pInt);
	return 0;
    }

    switch (VM86_TYPE(retval)) {
    case VM86_UNKNOWN:
	if (!vm86_GP_fault(pInt)) return 0;
	break;
    case VM86_STI:
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "vm86_sti :-((\n");
	dump_registers(pInt);
	dump_code(pInt);
	stack_trace(pInt);
	return 0;
    case VM86_INTx:
	pInt->num = VM86_ARG(retval);
	if (!int_handler(pInt)) {
	    xf86DrvMsg(pInt->scrnIndex, X_ERROR,
		"Unknown vm86_int: 0x%X\n\n", VM86_ARG(retval));
	    dump_registers(pInt);
	    dump_code(pInt);
	    stack_trace(pInt);
	    return 0;
	}
	/* I'm not sure yet what to do if we can handle ints */
	break;
    case VM86_SIGNAL:
	return 1;
	/*
	 * we used to warn here and bail out - but now the sigio stuff
	 * always fires signals at us. So we just ignore them for now.
	 */
	xf86DrvMsg(pInt->scrnIndex, X_WARNING, "received signal\n");
	return 0;
    default:
	xf86DrvMsg(pInt->scrnIndex, X_ERROR, "unknown type(0x%x)=0x%x\n",
		VM86_ARG(retval), VM86_TYPE(retval));
	dump_registers(pInt);
	dump_code(pInt);
	stack_trace(pInt);
	return 0;
    }

    return 1;
}

void
xf86ExecX86int10(xf86Int10InfoPtr pInt)
{
    int sig = setup_int(pInt);

    if (int_handler(pInt))
	while(do_vm86(pInt)) {};

    finish_int(pInt, sig);
}

static int
vm86_rep(struct vm86_struct *ptr)
{
    int __res;

#ifdef __PIC__
    /* When compiling with -fPIC, we can't use asm constraint "b" because
       %ebx is already taken by gcc. */
    __asm__ __volatile__("pushl %%ebx\n\t"
			 "movl %2,%%ebx\n\t"
			 "movl %1,%%eax\n\t"
			 "int $0x80\n\t"
			 "popl %%ebx"
			 :"=a" (__res)
			 :"n" ((int)113), "r" ((struct vm86_struct *)ptr));
#else
    __asm__ __volatile__("int $0x80\n\t"
			 :"=a" (__res):"a" ((int)113),
			 "b" ((struct vm86_struct *)ptr));
#endif

	    if (__res < 0) {
		errno = -__res;
		__res = -1;
	    }
	    else errno = 0;
	    return __res;
}

#endif
