/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/generic.c,v 1.13 2000/11/21 23:10:38 tsi Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */
#include "xf86.h"
#include "xf86str.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "compiler.h"
#define _INT10_PRIVATE
#include "xf86int10.h"
#include "int10Defines.h"

#define ALLOC_ENTRIES(x) ((V_RAM / x) - 1)

static CARD8 read_b(xf86Int10InfoPtr pInt,int addr);
static CARD16 read_w(xf86Int10InfoPtr pInt,int addr);
static CARD32 read_l(xf86Int10InfoPtr pInt,int addr);
static void write_b(xf86Int10InfoPtr pInt,int addr, CARD8 val);
static void write_w(xf86Int10InfoPtr pInt,int addr, CARD16 val);
static void write_l(xf86Int10InfoPtr pInt,int addr, CARD32 val);

/*
 * the emulator cannot pass a pointer to the current xf86Int10InfoRec
 * to the memory access functions therefore store it here.
 */

typedef struct {
    int shift;
    int entries;
    void* base;
    void* vRam;
    void* sysMem;
    char* alloc;
} genericInt10Priv;

#define INTPriv(x) ((genericInt10Priv*)x->private)

int10MemRec genericMem = {
    read_b,
    read_w,
    read_l,
    write_b,
    write_w,
    write_l
};

static void MapVRam(xf86Int10InfoPtr pInt);
static void UnmapVRam(xf86Int10InfoPtr pInt);

static void *sysMem = NULL;

xf86Int10InfoPtr
xf86InitInt10(int entityIndex)
{
    xf86Int10InfoPtr pInt;
    int screen, cs;
    void* base = 0;
    void* vbiosMem = 0;
    legacyVGARec vga;
    
    screen = (xf86FindScreenForEntity(entityIndex))->scrnIndex;
    
    if (int10skip(xf86Screens[screen],entityIndex))
	return NULL;

    pInt = (xf86Int10InfoPtr)xnfcalloc(1,sizeof(xf86Int10InfoRec));
    pInt->entityIndex = entityIndex;
    if (!xf86Int10ExecSetup(pInt))
	goto error0;
    pInt->mem = &genericMem;
    pInt->private = (pointer)xnfcalloc(1,sizeof(genericInt10Priv));
    INTPriv(pInt)->alloc = 
	(pointer)xnfcalloc(1,ALLOC_ENTRIES(getpagesize()));
    pInt->scrnIndex = screen;
    base = INTPriv(pInt)->base = xnfalloc(SYS_BIOS);

    /*
     * we need to map video RAM MMIO as some chipsets map mmio
     * registers into this range.
     */
    MapVRam(pInt);
#ifdef _PC
    if (!sysMem)
	sysMem = xf86MapVidMem(screen,VIDMEM_FRAMEBUFFER,SYS_BIOS,BIOS_SIZE);
    INTPriv(pInt)->sysMem = sysMem;
    
    if (xf86ReadBIOS(0,0,(unsigned char *)base,LOW_PAGE_SIZE) < 0) {
	xf86DrvMsg(screen,X_ERROR,"Cannot read int vect\n");
	goto error1;
    }

    /*
     * Retrieve everything between V_BIOS and SYS_BIOS as some system BIOSes
     * have executable code there.  Note that xf86ReadBIOS() can only read in
     * 64kB at a time.
     */
    (void)memset((char *)base + V_BIOS, 0, SYS_BIOS - V_BIOS);
    for (cs = V_BIOS;  cs < SYS_BIOS;  cs += V_BIOS_SIZE)
	if (xf86ReadBIOS(cs, 0, (unsigned char *)base + cs, V_BIOS_SIZE) <
		V_BIOS_SIZE)
	    xf86DrvMsg(screen, X_WARNING,
		"Unable to retrieve all of segment 0x%06X.\n", cs);

    if (xf86IsEntityPrimary(entityIndex)) {
	cs = MEM_RW(pInt,((0x10<<2)+2));

	vbiosMem = (unsigned char *)base + (cs << 4);
	if (!int10_check_bios(screen, cs, vbiosMem)) {
	    cs = MEM_RW(pInt,((0x42<<2)+2));
	    vbiosMem = (unsigned char *)base + (cs << 4);
	    if (!int10_check_bios(screen, cs, vbiosMem)) {
		cs = V_BIOS >> 4;
		vbiosMem = (unsigned char *)base + (cs << 4);
		if (!int10_check_bios(screen, cs, vbiosMem)) {
		    xf86DrvMsg(screen,X_ERROR,"No V_BIOS found\n");
		    goto error1;
		}
	    }
	}
	xf86DrvMsg(screen,X_INFO,"Primary V_BIOS segment is: 0x%x\n",cs);
	
	set_return_trap(pInt);
	pInt->BIOSseg = cs;
    } else {
        reset_int_vect(pInt);
	set_return_trap(pInt);
	vbiosMem = (unsigned char *)base + V_BIOS;
	if (!mapPciRom(pInt,(unsigned char *)(vbiosMem))) {
	    xf86DrvMsg(screen,X_ERROR,"Cannot read V_BIOS (3)\n");
	    goto error1;
	}
	pInt->BIOSseg = V_BIOS >> 4;
	pInt->num = 0xe6;
	LockLegacyVGA(screen, &vga); 
	xf86ExecX86int10(pInt);
	UnlockLegacyVGA(screen, &vga);
    }
#else
    if (!sysMem) {
	sysMem = xnfalloc(BIOS_SIZE);
	setup_system_bios((memType)sysMem);
    }
    INTPriv(pInt)->sysMem = sysMem;
    setup_int_vect(pInt);
    set_return_trap(pInt);
    vbiosMem = (unsigned char *)base + V_BIOS;
    {
        EntityInfoPtr pEnt = xf86GetEntityInfo(pInt->entityIndex);
	switch (pEnt->location.type) {
	case BUS_PCI:
	    if (!mapPciRom(pInt,(unsigned char *)(vbiosMem))) {
	      xf86DrvMsg(screen,X_ERROR,"Cannot read V_BIOS (4)\n");
	      goto error1;
	    }
	    break;
	case BUS_ISA:  
	    (void)memset(vbiosMem, 0, V_BIOS_SIZE);
	    if (xf86ReadBIOS(V_BIOS, 0, vbiosMem, V_BIOS_SIZE) < V_BIOS_SIZE)
		xf86DrvMsg(screen, X_WARNING,
		    "Unable to retrieve all of segment 0x0C0000.\n");
	    if (!int10_check_bios(screen, V_BIOS >> 4, vbiosMem)) {
	        xf86DrvMsg(screen,X_ERROR,"Cannot read V_BIOS (5)\n");
		goto error1;
	    }
	    break;
	default:
	    goto error1;
	}
    }
    pInt->BIOSseg = V_BIOS >> 4;
    pInt->num = 0xe6;
    LockLegacyVGA(screen, &vga);	   
    xf86ExecX86int10(pInt);
    UnlockLegacyVGA(screen, &vga);
#endif
    return pInt;

 error1:
    xfree(base);
    UnmapVRam(pInt);
    xfree(INTPriv(pInt)->alloc);
    xfree(pInt->private);
 error0:
    xfree(pInt);

    return NULL;
}

static void
MapVRam(xf86Int10InfoPtr pInt)
{
    int screen = pInt->scrnIndex;
    int pagesize = getpagesize();
    int size = ((VRAM_SIZE + pagesize - 1)/pagesize) * pagesize;

    INTPriv(pInt)->vRam = xf86MapVidMem(screen,VIDMEM_MMIO,V_RAM,size);
}

static void 
UnmapVRam(xf86Int10InfoPtr pInt)
{
    int screen = pInt->scrnIndex;
    int pagesize = getpagesize();
    int size = ((VRAM_SIZE + pagesize - 1)/pagesize) * pagesize;

    xf86UnMapVidMem(screen,INTPriv(pInt)->vRam,size);
}

void
MapCurrentInt10(xf86Int10InfoPtr pInt)
{
  /* nothing to do here */
}

void
xf86FreeInt10(xf86Int10InfoPtr pInt)
{
    if (!pInt)
      return;
    if (Int10Current == pInt) 
	Int10Current = NULL;
    xfree(INTPriv(pInt)->base);
    UnmapVRam(pInt);
    xfree(INTPriv(pInt)->alloc);
    xfree(pInt->private);
    xfree(pInt);
}

void *
xf86Int10AllocPages(xf86Int10InfoPtr pInt,int num, int *off)
{
    int pagesize = getpagesize();
    int num_pages = ALLOC_ENTRIES(pagesize);
    int i,j;

    for (i=0;i<num_pages - num;i++) {
	if (INTPriv(pInt)->alloc[i] == 0) {
	    for (j=i;j < num + i;j++)
		if (INTPriv(pInt)->alloc[j] != 0)
		    break;
	    if (j == num + i)
		break;
	    else
		i = i + num;
	}
    }
    if (i == num_pages - num)
	return NULL;
    
    for (j = i; j < i + num; j++)
	INTPriv(pInt)->alloc[j] = 1;

    *off = (i + 1) * pagesize;
    
    return (void *)
	((char*)INTPriv(pInt)->base + (i + 1) * pagesize);
}

void
xf86Int10FreePages(xf86Int10InfoPtr pInt, void *pbase, int num)
{
    int pagesize = getpagesize();
    int first = ((unsigned long)pbase
		 - (unsigned long)INTPriv(pInt)->base)
	/ pagesize - 1;
    int i;

    for (i = first; i < first + num; i++)
	INTPriv(pInt)->alloc[i] = 0;
}

#define OFF(addr) ((addr) & 0xffff)
#define SYS(addr) ((addr) >= SYS_BIOS)
#define V_ADDR(addr) \
          (SYS(addr) ? ((char*)INTPriv(pInt)->sysMem) + (addr - SYS_BIOS) \
           : ((char*)(INTPriv(pInt)->base) + addr))
#define VRAM_ADDR(addr) (addr - V_RAM)
#define VRAM_BASE (INTPriv(pInt)->vRam)

#define VRAM(addr) ((addr >= V_RAM) && (addr < (V_RAM + VRAM_SIZE)))
#define V_ADDR_RB(addr) \
        (VRAM(addr)) ? MMIO_IN8((CARD8*)VRAM_BASE,VRAM_ADDR(addr)) \
           : *(CARD8*) V_ADDR(addr)
#define V_ADDR_RW(addr) \
        (VRAM(addr)) ? MMIO_IN16((CARD16*)VRAM_BASE,VRAM_ADDR(addr)) \
           : ldw_u((pointer)V_ADDR(addr))
#define V_ADDR_RL(addr) \
        (VRAM(addr)) ? MMIO_IN32((CARD32*)VRAM_BASE,VRAM_ADDR(addr)) \
           : ldl_u((pointer)V_ADDR(addr))

#define V_ADDR_WB(addr,val) \
        if(VRAM(addr)) \
            MMIO_OUT8((CARD8*)VRAM_BASE,VRAM_ADDR(addr),val); \
        else \
            *(CARD8*) V_ADDR(addr) = val;
#define V_ADDR_WW(addr,val) \
        if(VRAM(addr)) \
            MMIO_OUT16((CARD16*)VRAM_BASE,VRAM_ADDR(addr),val); \
        else \
            stw_u((val),(pointer)(V_ADDR(addr)));

#define V_ADDR_WL(addr,val) \
        if (VRAM(addr)) \
            MMIO_OUT32((CARD32*)VRAM_BASE,VRAM_ADDR(addr),val); \
        else \
            stl_u(val,(pointer)(V_ADDR(addr)));

static CARD8
read_b(xf86Int10InfoPtr pInt, int addr)
{
    return V_ADDR_RB(addr);
}

static CARD16
read_w(xf86Int10InfoPtr pInt, int addr)
{
#if X_BYTE_ORDER == X_BIG_ENDIAN
    return ((V_ADDR_RB(addr))
	    || ((V_ADDR_RB(addr + 1)) << 8));
#else
    if (OFF(addr + 1) > 0) {
	return V_ADDR_RW(addr);
    } else
	return ((V_ADDR_RB(addr + 1))
		|| ((V_ADDR_RB(addr)) << 8));

#endif
}

static CARD32
read_l(xf86Int10InfoPtr pInt, int addr)
{
#if X_BYTE_ORDER == X_BIG_ENDIAN
    return ((V_ADDR_RB(addr))
	    || ((V_ADDR_RB(addr + 1)) << 8)
	    || ((V_ADDR_RB(addr + 2)) << 16)
	    || ((V_ADDR_RB(addr + 3)) << 24));
#else
    if (OFF(addr + 3)  > 2) {
	return V_ADDR_RL(addr);
    } else {
	return ((V_ADDR_RB(addr + 3))
		|| ((V_ADDR_RB(addr + 2)) << 8)
		|| ((V_ADDR_RB(addr + 1)) << 16)
		|| ((V_ADDR_RB(addr)) << 24));
    }
#endif
}

static void
write_b(xf86Int10InfoPtr pInt, int addr, CARD8 val)
{
    V_ADDR_WB(addr,val);
}

static void
write_w(xf86Int10InfoPtr pInt, int addr, CARD16 val)
{
#if X_BYTE_ORDER == X_BIG_ENDIAN
    V_ADDR_WB(addr,val);
    V_ADDR_WB(addr + 1,val >> 8);
#else
    if (OFF(addr + 1) > 0) {
	V_ADDR_WW(addr,val);
    } else {
	V_ADDR_WB(addr + 1,val);
	V_ADDR_WB(addr,val >> 8);
    }
#endif
}

static void
write_l(xf86Int10InfoPtr pInt, int addr, CARD32 val)
{
#if X_BYTE_ORDER == X_BIG_ENDIAN
    V_ADDR_WB(addr,val);
    V_ADDR_WB(addr + 1, val >> 8);
    V_ADDR_WB(addr + 2, val >> 16);
    V_ADDR_WB(addr + 3, val >> 24);
#else
    if (OFF(addr + 3) > 2) {
	V_ADDR_WL(addr,val);
    } else {
	V_ADDR_WB(addr + 3, val);
	V_ADDR_WB(addr + 2, val >> 8);
	V_ADDR_WB(addr + 1, val >> 16);
	V_ADDR_WB(addr, val >> 24);
    }
#endif
}

pointer
xf86int10Addr(xf86Int10InfoPtr pInt, CARD32 addr)
{
    return (pointer) V_ADDR(addr);
}








