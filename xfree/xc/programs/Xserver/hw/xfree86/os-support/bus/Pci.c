/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/Pci.c,v 1.48.2.1 2001/05/21 05:00:36 tsi Exp $ */
/*
 * Pci.c - New server PCI access functions
 *
 * The XFree86 server PCI access functions have been reimplemented as a
 * framework that allows each supported platform/OS to have their own
 * platform/OS specific pci driver.
 *
 * All of the public PCI access functions exported to the other parts of
 * the server are declared in Pci.h and defined herein.  These include:
 * 	pciInit()      - Initialize PCI access functions
 * 	pciFindFirst() - Find a PCI device by dev/vend id
 * 	pciFindNext()  - Find another PCI device by dev/vend id
 * 	pciReadLong()  - Read a 32 bit value from a PCI devices cfg space
 * 	pciReadWord()  - Read a 16 bit value from a PCI devices cfg space
 * 	pciReadByte()  - Read an 8 bit value from a PCI devices cfg space
 * 	pciWriteLong() - Write a 32 bit value to a PCI devices cfg space
 * 	pciWriteWord() - Write a 16 bit value to a PCI devices cfg space
 * 	pciWriteByte() - Write an 8 bit value to a PCI devices cfg space
 *	pciSetBitsLong() - Write a 32 bit value against a mask
 *      pciSetBitsByte() - Write an 8 bit value against a mask
 *      pciLongFunc() - Return pointer to the requested low level function
 * 	pciTag()       - Return tag for a given PCI bus, device, & function
 * 	pciBusAddrToHostAddr() - Convert a PCI address to a host address
 * 	pciHostAddrToBusAddr() - Convert a host address to a PCI address
 *	pciGetBaseSize - Returns the number of bits in a PCI base addr mapping
 *	xf86MapPciMem() - Like xf86MapVidMem() expect function expects
 *                        a PCI address and PCITAG (identifies PCI domain)
 *	xf86ReadPciBIOS() - Like xf86ReadBIOS, except that it handles PCI/host
 *                          address translation and BIOS decode enabling.
 *	xf86scanpci()  - Return info about all PCI devices
 * 
 * The actual PCI backend driver is selected by the pciInit() function
 * (see below)  using either compile time definitions, run-time checks,
 * or both.
 *
 * Certain generic functions are provided that make the implementation 
 * of certain well behaved platforms (e.g. those supporting PCI config
 * mechanism 1 or some thing close to it) very easy.
 *
 * Less well behaved platforms/OS's can roll their own functions.
 *
 * To add support for another platform/OS, add a call to fooPciInit() within
 * pciInit() below under the correct compile time definition or run-time
 * conditional.
 *
 *
 * The fooPciInit() procedure must do three things:
 * 	1) Initialize the pciBusTable[] for all primary PCI buses including
 *	   the per domain PCI access functions (readLong, writeLong,
 *         addrBusToHost, and addrHostToBus).  
 *
 *	2) Add entries to pciBusTable[] for configured secondary buses.  This
 *	   step may be skipped if a platform is using the generic findFirst/
 *	   findNext functions because these procedures will automatically
 *	   discover and add secondary buses dynamically.
 *
 *      3) Overide default settings for global PCI access functions if
 *         required. These include pciFindFirstFP, pciFindNextFP,
 *         Of course, if you choose not to use one of the generic
 *         functions, you will need to provide a platform specifc replacement.
 *
 * See xc/programs/Xserver/hw/xfree86/os-support/pmax/pmax_pci.c for an example
 * of how to extend this framework to other platforms/OSes.
 *
 * Gary Barton
 * Concurrent Computer Corporation
 * garyb@gate.net
 *
 */

/*
 * Copyright 1998 by Concurrent Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Concurrent Computer
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Concurrent Computer Corporation makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 * 
 * CONCURRENT COMPUTER CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CONCURRENT COMPUTER CORPORATION BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 * Copyright 1998 by Metro Link Incorporated
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Metro Link
 * Incorporated not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Metro Link Incorporated makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 * 
 * METRO LINK INCORPORATED DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL METRO LINK INCORPORATED BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 * This software is derived from the original XFree86 PCI code
 * which includes the following copyright notices as well:
 *
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the above listed copyright holder(s)
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  The above listed
 * copyright holder(s) make(s) no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM(S) ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE 
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This code is also based heavily on the code in FreeBSD-current, which was
 * written by Wolfgang Stanglmeier, and contains the following copyright:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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
 */
#include <errno.h>
#include <signal.h>
#include "Xarch.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSproc.h"
#include "Pci.h"

#define PCI_MFDEV_SUPPORT   1 /* Include PCI multifunction device support */
#define PCI_BRIDGE_SUPPORT  1 /* Include support for PCI-to-PCI bridges */

#ifdef PC98
#define outb(port,data) _outb(port,data)
#define outl(port,data) _outl(port,data)
#define inb(port) _inb(port)
#define inl(port) _inl(port)
#endif

/*
 * Global data
 */
static int    pciInitialized = 0;

CARD32 pciDevid;            /* Requested device/vendor ID (after mask) */
CARD32 pciDevidMask;        /* Bit mask applied (AND) before comparison */
				   /* of real devid's with requested           */

int    pciBusNum;           /* Bus Number of current device */
int    pciDevNum;           /* Device number of current device */
int    pciFuncNum;          /* Function number of current device */
PCITAG pciDeviceTag;        /* Tag for current device */

pciBusFuncs_t pciNOOPFuncs = {
	pciReadLongNULL,
	pciWriteLongNULL,
	pciSetBitsLongNULL,
	pciAddrNOOP,
	pciAddrNOOP
};
		
pciBusInfo_t  *pciBusInfo[MAX_PCI_BUSES] = { NULL, };
int            pciNumBuses = 0;     /* Actual number of PCI buses */
static Bool    inProbe = FALSE;

static pciConfigPtr pci_devp[MAX_PCI_DEVICES + 1] = {NULL, };
#ifdef INCLUDE_LOCKPCI
static pciConfigPtr pci_locked_devp[MAX_PCI_DEVICES + 1] = {NULL, };
#endif

/*
 * Platform specific PCI function pointers.
 *
 * NOTE: A platform/OS specific pci init procedure can override these defaults
 *       by setting them to the appropriate platform dependent functions.
 */
PCITAG     (*pciFindFirstFP)(void) = pciGenFindFirst;
PCITAG     (*pciFindNextFP)(void) = pciGenFindNext;

/*
 * pciInit - choose correct platform/OS specific PCI init routine
 */
void
pciInit()
{
	if (pciInitialized)
		return;

	pciInitialized = 1;

	/* XXX */
#if defined(DEBUGPCI)
	if (DEBUGPCI >= xf86Verbose)
		xf86Verbose = DEBUGPCI;
#endif

	ARCH_PCI_INIT();
	if (!pciNumBuses)
#if defined(ARCH_PCI_OS_INIT)
	    ARCH_PCI_OS_INIT();
#else
	    xf86Msg(X_ERROR,"No OS PCI support available\n");
#endif
}

  
PCITAG
pciFindFirst(CARD32 id, CARD32 mask)
{
#ifdef DEBUGPCI
ErrorF("pciFindFirst(0x%lx, 0x%lx), pciInit = %d\n", id, mask, pciInitialized);
#endif
  if (!pciInitialized)
    pciInit();

  pciDevid = id & mask;
  pciDevidMask = mask;

  return((*pciFindFirstFP)());
}

PCITAG
pciFindNext(void)
{
#ifdef DEBUGPCI
ErrorF("pciFindNext(), pciInit = %d\n", pciInitialized);
#endif
  if (!pciInitialized)
    pciInit();
    
  return((*pciFindNextFP)());
}

CARD32
pciReadLong(PCITAG tag, int offset)
{
  int bus = PCI_BUS_FROM_TAG(tag);

#ifdef DEBUGPCI
ErrorF("pciReadLong(0x%lx, %d)\n", tag, offset);
#endif
  if (!pciInitialized)
    pciInit();

  if ((bus < pciNumBuses || inProbe) &&  pciBusInfo[bus] &&
	pciBusInfo[bus]->funcs.pciReadLong) {
    CARD32 rv = (*pciBusInfo[bus]->funcs.pciReadLong)(tag, offset);

    PCITRACE(1, ("pciReadLong: tag=0x%x [b=%d,d=%d,f=%d] returns 0x%08x\n",
		 tag, bus, PCI_DEV_FROM_TAG(tag), PCI_FUNC_FROM_TAG(tag), rv));
    return(rv);
   }

  return(PCI_NOT_FOUND);
}

CARD16
pciReadWord(PCITAG tag, int offset)
{
  CARD32 tmp;
  int    shift = (offset & 3) * 8;
  int    aligned_offset = offset & ~3;
  
  if (shift != 0 && shift != 16)
	  FatalError("pciReadWord: Alignment error: Cannot read 16 bits "
		     "at offset %d\n", offset);
  
  tmp = pciReadLong(tag, aligned_offset);
  
  return((CARD16)((tmp >> shift) & 0xffff));
}

CARD8
pciReadByte(PCITAG tag, int offset)
{
  CARD32 tmp;
  int    shift = (offset & 3) * 8;
  int    aligned_offset = offset & ~3;
  
  tmp = pciReadLong(tag, aligned_offset);
  
  return((CARD8)((tmp >> shift) & 0xff));
}

void
pciWriteLong(PCITAG tag, int offset, CARD32 val)
{
  int bus = PCI_BUS_FROM_TAG(tag);
	
  if (!pciInitialized)
    pciInit();
  
  if (bus < pciNumBuses && pciBusInfo[bus] &&
	pciBusInfo[bus]->funcs.pciWriteLong)
	  (*pciBusInfo[bus]->funcs.pciWriteLong)(tag, offset, val);
}
  
void
pciWriteWord(PCITAG tag, int offset, CARD16 val)
{ 
  CARD32 tmp;
  int    aligned_offset = offset & ~3;
  int    shift = (offset & 3) * 8;

  if (shift != 0 && shift != 16)
	  FatalError("pciWriteWord: Alignment Error: Cannot read 16 bits "
			"from offset %d\n", offset);
  
  tmp = pciReadLong(tag, aligned_offset);
  
  tmp &= ~(0xffffL << shift);
  tmp |= (((CARD32)val) << shift);
  
  pciWriteLong(tag, aligned_offset, tmp);
}

void
pciWriteByte(PCITAG tag, int offset, CARD8 val)
{ 
  CARD32 tmp;
  int    aligned_offset = offset & ~3;
  int    shift = (offset & 3) *8 ;
  
  tmp = pciReadLong(tag, aligned_offset);
  
  tmp &= ~(0xffL << shift);
  tmp |= (((CARD32)val) << shift);
  
  pciWriteLong(tag, aligned_offset, tmp);
} 
  
void
pciSetBitsLong(PCITAG tag, int offset, CARD32 mask, CARD32 val)
{
    int bus = PCI_BUS_FROM_TAG(tag);

#ifdef DEBUGPCI
    ErrorF("pciReadLong(0x%lx, %d)\n", tag, offset);
#endif
    if (!pciInitialized)
	pciInit();

    if (bus < pciNumBuses && pciBusInfo[bus] &&
	pciBusInfo[bus]->funcs.pciReadLong) {
	(*pciBusInfo[bus]->funcs.pciSetBitsLong)(tag, offset, mask, val);
    }
}

void
pciSetBitsByte(PCITAG tag, int offset, CARD8 mask, CARD8 val)
{ 
  CARD32 tmp_mask, tmp_val;
  int    aligned_offset = offset & ~3;
  int    shift = (offset & 3) *8 ;

  
  tmp_mask = mask << shift;
  tmp_val = val << shift;
  pciSetBitsLong(tag, aligned_offset, tmp_mask, tmp_val);
}

pointer
pciLongFunc(PCITAG tag, pciFunc func)
{
    int bus = PCI_BUS_FROM_TAG(tag);

    if (!pciInitialized)
	pciInit();

    if (bus > pciNumBuses || !pciBusInfo[bus] ||
	!pciBusInfo[bus]->funcs.pciReadLong) return NULL;
	
    switch (func) {
    case WRITE:
	return 	(void *)pciBusInfo[bus]->funcs.pciWriteLong;
    case READ:
	return 	(void *)pciBusInfo[bus]->funcs.pciReadLong;
    case SET_BITS:
	return 	(void *)pciBusInfo[bus]->funcs.pciSetBitsLong;
    }
    return NULL;
}

ADDRESS
pciBusAddrToHostAddr(PCITAG tag, PciAddrType type, ADDRESS addr)
{
  int bus = PCI_BUS_FROM_TAG(tag);
	
  if (!pciInitialized)
    pciInit();
  
  if (bus < pciNumBuses && pciBusInfo[bus] &&
	pciBusInfo[bus]->funcs.pciAddrBusToHost)
	  return (*pciBusInfo[bus]->funcs.pciAddrBusToHost)(tag, type, addr);
  else
	  return(addr);
}

ADDRESS
pciHostAddrToBusAddr(PCITAG tag, PciAddrType type, ADDRESS addr)
{
  int bus = PCI_BUS_FROM_TAG(tag);
	
  if (!pciInitialized)
    pciInit();
  
  if (bus < pciNumBuses && pciBusInfo[bus] &&
	pciBusInfo[bus]->funcs.pciAddrHostToBus)
	  return (*pciBusInfo[bus]->funcs.pciAddrHostToBus)(tag, type, addr);
  else
	  return(addr);
}


/*
 * pciGetBaseSize() returns the size of a PCI base address mapping in bits.
 * The index identifies the base register: 0-5 are the six standard registers,
 * and 6 is the ROM base register.  If destructive is TRUE, it will write
 * to the base address register to get an accurate result.  Otherwise it
 * makes a conservative guess based on the alignment of the already allocated
 * address.  If the result is accurate (ie, not an over-estimate), this is
 * indicated by setting *min to TRUE (when min is non-NULL).  This currently
 * only happens when the destructive flag is set, but in future it may be
 * possible to get the information from the OS when supported.
 */

int
pciGetBaseSize(PCITAG tag, int index, Bool destructive, Bool *min)
{
  int offset;
  CARD32 addr1;
  CARD32 addr2;
  CARD32 mask1;
  CARD32 mask2;
  int bits = 0;
 
  /*
   * Eventually a function for this should be added to pciBusFuncs_t, but for
   * now we'll just use a simple method based on the alignment of the already
   * allocated address.
   */

  /*
   * silently ignore bogus index values.  Valid values are 0-6.  0-5 are
   * the 6 base address registers, and 6 is the ROM base address register.
   */
  if (index < 0 || index > 6)
    return 0;

  if (!pciInitialized)
    pciInit();

  if (xf86GetPciSizeFromOS(tag, index, &bits)) {
      if (min)
	  *min = TRUE;
      return bits;
  }
  
  if (min)
    *min = destructive;

  /* Get the PCI offset */
  if (index == 6) 
    offset = PCI_MAP_ROM_REG;
  else
    offset = PCI_MAP_REG_START + (index << 2);

  addr1 = pciReadLong(tag, offset);
  /*
   * Check if this is the second part of a 64 bit address.
   * XXX need to check how endianness affects 64 bit addresses.
   */
  if (index > 0 && index < 6) {
    addr2 = pciReadLong(tag, offset - 4);
    if (PCI_MAP_IS_MEM(addr2) && PCI_MAP_IS64BITMEM(addr2))
      return 0;
  }

  if (destructive) {
    pciWriteLong(tag, offset, 0xffffffff);
    mask1 = pciReadLong(tag, offset);
    pciWriteLong(tag, offset, addr1);
  } else {
    mask1 = addr1;
  }

  /* Check if this is the first part of a 64 bit address. */
  if (index < 5 && PCI_MAP_IS_MEM(mask1) && PCI_MAP_IS64BITMEM(mask1)) {
    if (PCIGETMEMORY(mask1) == 0) {
      addr2 = pciReadLong(tag, offset + 4);
      if (destructive) {
	pciWriteLong(tag, offset + 4, 0xffffffff);
	mask2 = pciReadLong(tag, offset + 4);
	pciWriteLong(tag, offset + 4, addr2);
      } else {
	mask2 = addr2;
      }
      if (mask2 == 0)
	return 0;
      bits = 32;
      while ((mask2 & 1) == 0) {
	bits++;
	mask2 >>= 1;
      }
      if (bits > 32)
	  return bits;
    }
  }
  if (index < 6)
    if (PCI_MAP_IS_MEM(mask1))
      mask1 = PCIGETMEMORY(mask1);
    else
      mask1 = PCIGETIO(mask1);
  else
    mask1 = PCIGETROM(mask1);
  if (mask1 == 0)
    return 0;
  bits = 0;
  while ((mask1 & 1) == 0) {
    bits++;
    mask1 >>= 1;
  }
  /* I/O maps can be no larger than 8 bits */

#if defined(Lynx) && defined(__powerpc__)
  if (PCI_MAP_IS_IO(addr1) && bits > 8)
#else
  if ((index < 6) && PCI_MAP_IS_IO(addr1) && bits > 8)
#endif
    bits = 8;
  /* ROM maps can be no larger than 24 bits */
  if (index == 6 && bits > 24)
    bits = 24;
  return bits;
}

PCITAG
pciTag(int busnum, int devnum, int funcnum)
{
	return(PCI_MAKE_TAG(busnum,devnum,funcnum));
}

Bool
pciMfDev(int busnum, int devnum)
{
    PCITAG tag0, tag1;
    unsigned long id0, id1;
 
    /* Detect a multi-function device that complies to the PCI 2.0 spec */
 
    tag0 = PCI_MAKE_TAG(busnum, devnum, 0);
    id0  = pciReadLong(tag0, PCI_ID_REG);
    if (id0 == 0xffffffff)
        return FALSE;

    if (pciReadLong(tag0, PCI_HEADER_MISC) & PCI_HEADER_MULTIFUNCTION)
        return TRUE;
 
    /*
     * Now, to find non-compliant devices...
     * If there is a valid ID for function 1 and the ID for func 0 and 1
     * are different, or the base0 values of func 0 and 1 are differend,
     * then assume there is a multi-function device.
     */
    tag1 = PCI_MAKE_TAG(busnum, devnum, 1);
    id1  = pciReadLong(tag1, PCI_ID_REG);
    if (id1 == 0xffffffff || id1 == 0x00000000)
	return FALSE;
 
    if ((id0 != id1) || 
        (pciReadLong(tag0, PCI_MAP_REG_START) !=
	 pciReadLong(tag1, PCI_MAP_REG_START)))
        return TRUE;

    return FALSE;
}

/*
 * Generic find/read/write functions
 */
PCITAG
pciGenFindNext(void)
{
    unsigned long devid, tmp;
    unsigned char base_class, sub_class, sec_bus, pri_bus;
    Bool speculativeProbe = FALSE;
  
#ifdef DEBUGPCI
    ErrorF("pciGenFindNext\n");
#endif

    for (;;) {

#ifdef DEBUGPCI
	ErrorF("pciGenFindNext: pciBusNum %d\n", pciBusNum);
#endif
	if (pciBusNum == -1) {
	    /*
	     * Start at top of the order
	     */
	    if (pciNumBuses == 0)
		return(PCI_NOT_FOUND);

	    pciBusNum = 0;
	    pciFuncNum = 0;
	    pciDevNum = 0;
	} else {
#ifdef PCI_MFDEV_SUPPORT
#ifdef DEBUGPCI
	    ErrorF("pciGenFindNext: pciFuncNum %d\n", pciFuncNum);
#endif
	    /*
	     * Somewhere in middle of order.  Determine who's
	     * next up
	     */
	    if (pciFuncNum == 0) {
		/*
		 * Is current dev a multifunction device?
		 */
		if (pciMfDev(pciBusNum, pciDevNum))
		    /* Probe for other functions */
		    pciFuncNum = 1;
		else
		    /*
		     * No more functions this device. Next
		     * device please
		     */
		    pciDevNum ++;
	    } else if (++pciFuncNum >= 8) {
		/* No more functions for this device. Next device please */
		pciFuncNum = 0;
		pciDevNum ++;
	    }
#else
	    pciDevNum ++;
#endif
	    if (pciDevNum >= 32 ||
		!pciBusInfo[pciBusNum] ||
		pciDevNum >= pciBusInfo[pciBusNum]->numDevices) {
#ifdef DEBUGPCI
		ErrorF("pciGenFindNext: next bus\n");
#endif
		/*
		 * No more devices for this bus. Next bus please
		 */
		if (speculativeProbe) {
		    xfree(pciBusInfo[pciBusNum]);
		    pciBusInfo[pciBusNum] = NULL;
		}
     
	 
		if (++pciBusNum >= MAX_PCI_BUSES) {
#ifdef DEBUGPCI
		    ErrorF("pciGenFindNext: out of buses\n");
#endif
		    /* No more buses.  All done for now */
		    return(PCI_NOT_FOUND);
		}
		    
		pciDevNum = 0;
	    }
	}

#ifdef DEBUGPCI
	ErrorF("pciGenFindNext: pciBusInfo[%d] = 0x%lx\n", pciBusNum, pciBusInfo[pciBusNum]);
#endif
	if (!pciBusInfo[pciBusNum]) {
	    pciBusInfo[pciBusNum] = xnfalloc(sizeof(pciBusInfo_t));
	    *pciBusInfo[pciBusNum] = *pciBusInfo[0];
     
	    speculativeProbe = TRUE;
	}
    
	/*
	 * At this point, pciBusNum, pciDevNum, and pciFuncNum have been
	 * advanced to the next device.  Compute the tag, and read the
	 * device/vendor ID field.
	 */
#ifdef DEBUGPCI
	ErrorF("pciGenFindNext: [%d, %d, %d]\n", pciBusNum, pciDevNum, pciFuncNum);
#endif
	pciDeviceTag = PCI_MAKE_TAG(pciBusNum, pciDevNum, pciFuncNum);
	inProbe = TRUE;
	devid = pciReadLong(pciDeviceTag, 0);
	inProbe = FALSE;
#ifdef DEBUGPCI
	ErrorF("pciGenFindNext: pciDeviceTag = 0x%lx, devid = 0x%lx\n", pciDeviceTag, devid);
#endif
	if (devid == 0xffffffff)
	    continue; /* Nobody home.  Next device please */

	if (speculativeProbe && (pciNumBuses <= pciBusNum))
	    pciNumBuses = pciBusNum + 1;
    
	speculativeProbe = FALSE;
    
	/*
	 * Before checking for a specific devid, look for enabled
	 * PCI to PCI bridge devices.  If one is found, create and
	 * initialize a bus info record (if one does not already exist).
	 */
#ifdef PCI_BRIDGE_SUPPORT    
	tmp = pciReadLong(pciDeviceTag, PCI_CLASS_REG);
	base_class = PCI_CLASS_EXTRACT(tmp);
	sub_class = PCI_SUBCLASS_EXTRACT(tmp); 
	if ((base_class == PCI_CLASS_BRIDGE) &&
	    (sub_class == PCI_SUBCLASS_BRIDGE_PCI)) {
	    tmp = pciReadLong(pciDeviceTag, PCI_PCI_BRIDGE_BUS_REG);
	    sec_bus = PCI_SECONDARY_BUS_EXTRACT(tmp);
	    pri_bus = PCI_PRIMARY_BUS_EXTRACT(tmp);
#ifdef DEBUGPCI
	    ErrorF("pciGenFindNext: pri_bus %d sec_bus %d\n", pri_bus, sec_bus);
#endif
	    if (sec_bus > 0 && sec_bus < MAX_PCI_BUSES && pciBusInfo[pri_bus]) {
		/*
		 * Found a secondary PCI bus
		 */
		if (!pciBusInfo[sec_bus])
		    pciBusInfo[sec_bus] = xnfalloc(sizeof(pciBusInfo_t));

		/* Copy parents settings... */
		*pciBusInfo[sec_bus] = *pciBusInfo[pri_bus];

		/* ...but not everything same as parent */
		pciBusInfo[sec_bus]->primary_bus = pri_bus;
		pciBusInfo[sec_bus]->secondary = TRUE;
		pciBusInfo[sec_bus]->numDevices = 32;

		if (pciNumBuses <= sec_bus)
		    pciNumBuses = sec_bus+1;
	    }
	}
#endif
    
	/*
	 * Does this device match the requested devid after
	 * applying mask?
	 */
#ifdef DEBUGPCI
	ErrorF("pciGenFindNext: pciDevidMask = 0x%lx, pciDevid = 0x%lx\n", pciDevidMask, pciDevid);
#endif
	if ((devid & pciDevidMask) == pciDevid)
	    /* Yes - Return it.  Otherwise, next device */
	    return(pciDeviceTag); /* got a match */

    } /* for */
    /*NOTREACHED*/
}

PCITAG
pciGenFindFirst(void)
{
  /* Reset PCI bus number to start from top */	
  pciBusNum = -1;
  
  return pciGenFindNext();
}

#if defined (__powerpc__)
static int buserr_detected;

static 
void buserr(int sig)
{
	buserr_detected = 1;
}
#endif

CARD32
pciCfgMech1Read(PCITAG tag, int offset)
{
  unsigned long rv = 0xffffffff;

#if defined(__powerpc__)
  signal(SIGBUS, buserr);
  buserr_detected = 0;
#endif

  outl(0xCF8, PCI_EN | tag | (offset & 0xfc));
  rv = inl(0xCFC);

#if defined(__powerpc__)
  signal(SIGBUS,SIG_DFL);
  if (buserr_detected)
    return(0xffffffff);
  else
#endif
    return(rv);
}

void
pciCfgMech1Write(PCITAG tag, int offset, CARD32 val)
{
#if defined(__powerpc__)
  signal(SIGBUS, SIG_IGN);
#endif

  outl(0xCF8, PCI_EN | tag | (offset & 0xfc));
#if defined(Lynx) && defined(__powerpc__)
  outb(0x80, 0x00);	/* wo this the next access fails 
                         * on my Powerstack system when we use
                         * assembler inlines for outl */
#endif
  outl(0xCFC, val);

#if defined(__powerpc__)
  signal(SIGBUS, SIG_DFL);
#endif
}

void
pciCfgMech1SetBits(PCITAG tag, int offset, CARD32 mask, CARD32 val)
{
    unsigned long rv = 0xffffffff;

#if defined(__powerpc__)
    signal(SIGBUS, buserr);
#endif

    outl(0xCF8, PCI_EN | tag | (offset & 0xfc));
    rv = inl(0xCFC);
    rv = (rv & ~mask) | val;
    outl(0xCFC, rv);

#if defined(__powerpc__)
    signal(SIGBUS,SIG_DFL);
#endif
}

CARD32
pciByteSwap(CARD32 u)
{
#if X_BYTE_ORDER == X_BIG_ENDIAN
# if defined(__powerpc__) && defined(PowerMAX_OS)
  CARD32 tmp;

  __inst_stwbrx(u, &tmp, 0);

  return(tmp);
	
# else /* !PowerMAX_OS */

  return lswapl(u);

# endif /* !PowerMAX_OS */

#else /* !BIG_ENDIAN */
  
  return(u);
  
#endif
}

/*
 * Dummy functions to noop the PCI services
 */
CARD32
pciReadLongNULL(PCITAG tag, int offset)
{
	return(0xffffffff);
}

void
pciWriteLongNULL(PCITAG tag, int offset, CARD32 val)
{
}

void
pciSetBitsLongNULL(PCITAG tag, int offset, CARD32 mask, CARD32 val)
{
}

ADDRESS
pciAddrNOOP(PCITAG tag, PciAddrType type, ADDRESS addr)
{
	return(addr);
}

pciConfigPtr *
xf86scanpci(int flags)
{
    int idx = 0;
    PCITAG tag;

    if (pci_devp[0])
	return pci_devp;

    pciInit();

    tag = pciFindFirst(0,0);  /* 0 mask means match any valid device */
    /* Check if no devices, return now */
    if (tag == PCI_NOT_FOUND)
	return NULL;

#ifdef DEBUGPCI
ErrorF("xf86scanpci: tag = 0x%lx\n", tag);
#endif
#ifndef OLD_FORMAT
    xf86MsgVerb(X_INFO, 2, "PCI: PCI scan (all values are in hex)\n");
#endif
    while (idx < MAX_PCI_DEVICES && tag != PCI_NOT_FOUND) {
	    pciConfigPtr devp;
	    int          i;
	    
	    devp = xalloc(sizeof(pciDevice));
	    if (!devp) {
		    xf86Msg(X_ERROR,
			"xf86scanpci: Out of memory after %d devices!!\n",
			idx);
		    return (pciConfigPtr *)NULL;
	    }

	    /* Identify pci device by bus, dev, func, and tag */
	    devp->tag = tag;
	    devp->busnum = PCI_BUS_FROM_TAG(tag);
	    devp->devnum = PCI_DEV_FROM_TAG(tag);
	    devp->funcnum = PCI_FUNC_FROM_TAG(tag);
	    
	    /* Read config space for this device */
	    for (i = 0; i < 17; i++)  /* PCI hdr plus 1st dev spec dword */
		    devp->cfgspc.dwords[i] =
				pciReadLong(tag, i * sizeof(CARD32));

	    /* Get base address sizes for type 0 headers */
	    if ((devp->pci_header_type & 0x7f) == 0)
		for (i = 0; i < 7; i++)
		    devp->basesize[i] = pciGetBaseSize(tag, i, FALSE, 
						       &devp->minBasesize);
	    devp->listed_class = 0;

#ifdef OLD_FORMAT
	    xf86MsgVerb(X_INFO, 2, "PCI: BusID 0x%02x,0x%02x,0x%1x "
			"ID 0x%04x,0x%04x Rev 0x%02x Class 0x%02x,0x%02x\n",
			devp->busnum, devp->devnum, devp->funcnum,
			devp->pci_vendor, devp->pci_device, devp->pci_rev_id,
			devp->pci_base_class, devp->pci_sub_class);
#else
	    xf86MsgVerb(X_INFO, 2, "PCI: %02x:%02x:%1x: chip %04x,%04x"
			" card %04x,%04x rev %02x class %02x,%02x,%02x"
			" hdr %02x\n",
			devp->busnum, devp->devnum, devp->funcnum,
			devp->pci_vendor, devp->pci_device,
			devp->pci_subsys_vendor, devp->pci_subsys_card,
			devp->pci_rev_id, devp->pci_base_class,
			devp->pci_sub_class, devp->pci_prog_if,
			devp->pci_header_type);
#endif

	    pci_devp[idx++] = devp;
	    tag = pciFindNext();
#ifdef DEBUGPCI
ErrorF("xf86scanpci: tag = pciFindNext = 0x%lx\n", tag);
#endif
    }
#ifndef OLD_FORMAT
    xf86MsgVerb(X_INFO, 2, "PCI: End of PCI scan\n");
#endif
    
    return pci_devp;
}

CARD32
pciCheckForBrokenBase(PCITAG Tag,int basereg)
{
    pciWriteLong(Tag, PCI_MAP_REG_START + (basereg << 2), 0xffffffff);
    return pciReadLong(Tag, PCI_MAP_REG_START + (basereg << 2));
}

#if defined(INCLUDE_XF86_MAP_PCI_MEM)

pointer
xf86MapPciMem(int ScreenNum, int Flags, PCITAG Tag, ADDRESS Base,
		unsigned long Size)
{
	ADDRESS hostbase = pciBusAddrToHostAddr(Tag, PCI_MEM,Base);
	pointer base;
	CARD32 save = 0;

	/*
	 * If there are possible read side-effects, disable memory while
	 * doing the mapping.
	 */
	if (Flags & VIDMEM_READSIDEEFFECT) {
		save = pciReadLong(Tag, PCI_CMD_STAT_REG);
		pciWriteLong(Tag, PCI_CMD_STAT_REG,
			     save & ~PCI_CMD_MEM_ENABLE);
	}
	base = xf86MapVidMem(ScreenNum, Flags, hostbase, Size);
	if (!base)	{
		FatalError("xf86MapPciMem: Could not mmap PCI memory "
			   "[base=0x%x,hostbase=0x%x,size=%x] (%s)\n",
			   Base, hostbase, Size, strerror(errno));
	}
	/*
	 * If read side-effects, do whatever might be needed to prevent
	 * unintended reads, then restore PCI_CMD_STAT_REG.
	 */
	if (Flags & VIDMEM_READSIDEEFFECT) {
		xf86MapReadSideEffects(ScreenNum, Flags, base, Size);
		pciWriteLong(Tag, PCI_CMD_STAT_REG, save);
	}
	return((pointer)base);
}

static int
readPciBIOS(unsigned long Offset, PCITAG Tag, int basereg,
		unsigned char *Buf, int Len)
{
    CARD32 romsave = 0;
    int i;
    romBaseSource b_reg;
    ADDRESS hostbase;
    CARD8 tmp[64];
    CARD8 *image;

    unsigned long offset;
    int ret, length, len, rlength;

    romsave = pciReadLong(Tag, PCI_MAP_ROM_REG);

    for (i = ROM_BASE_PRESET; i <= ROM_BASE_FIND; i++) {
        memType savebase = 0, newbase, romaddr;

        if (i == ROM_BASE_PRESET) {
	    /* Does the driver have a preference? */
	    if (basereg > ROM_BASE_PRESET && basereg <= ROM_BASE_FIND)
	        b_reg =  basereg;
	    else 
	        b_reg = ++i;
	} else
	    b_reg = i;

	if (!(newbase = getValidBIOSBase(Tag, b_reg)))
	    continue;  /* no valid address found */

	romaddr = PCIGETROM(newbase);

	/* if we use a mem base save it and move it out of the way */
	if (b_reg >= 0 && b_reg <= 5) {
	    savebase = pciReadLong(Tag, PCI_MAP_REG_START+(basereg<<2));
	    xf86MsgVerb(X_INFO,5,"xf86ReadPciBios: modifying membase[%i]"
			" for device %i:%i:%i\n", basereg,
			PCI_BUS_FROM_TAG(Tag), PCI_DEV_FROM_TAG(Tag),
			PCI_FUNC_FROM_TAG(Tag));
	    pciWriteLong(Tag, PCI_MAP_REG_START + (b_reg << 2),
			 (CARD32)~0);
	}
	/* Set ROM base address and enable ROM address decoding */
	pciWriteLong(Tag, PCI_MAP_ROM_REG, romaddr 
		     | PCI_MAP_ROM_DECODE_ENABLE);

	hostbase = pciBusAddrToHostAddr(Tag, PCI_MEM, PCIGETROM(romaddr));
	
	if ((xf86ReadBIOS(hostbase, 0, tmp, sizeof(tmp)) != sizeof(tmp)) 
	    || (tmp[0] != 0x55) 
	    || (tmp[1] != 0xaa) 
	    || !tmp[2] ) { 
	  /* Restore the base register if it was changed. */
	    if (savebase) pciWriteLong(Tag, PCI_MAP_REG_START + (b_reg << 2),
				       (CARD32) savebase);
	    /* No BIOS found: try another address */
	    continue;
	}

#if 0
	/* 
	 * Currently this is only good for PC style BIOSes.
	 * This code needs to be revistited after 4.1 is out.
	 * We need to pass an argument for the BIOS type to
	 * look for. Then we can pick the correct BIOS.
	 * Combine this with the code in int10/pci.c.
	 */
	if ((Offset) > (tmp[2] << 9)) {
	    xf86Msg(X_WARNING,"xf86ReadPciBios: requesting data past "
		    "end of BIOS %i > %i\n",(Offset) , (tmp[2] << 9));
	} else {
	  if ((Offset + Len) > (tmp[2] << 9)) {
	    Len = (tmp[2] << 9) - Offset;
	    xf86Msg(X_INFO,"Truncating PCI BIOS Length to %i\n",Len);
	  }
	}
#endif

	/* Read BIOS in 64kB chunks */
	ret = 0;
	offset = Offset;
	image = Buf;
	len = Len;
	while ((length = len) > 0) {
	  if (length > 0x10000) length = 0x10000;
	  rlength = xf86ReadBIOS(hostbase, offset, image, length);
	  if (rlength < 0) {
	    ret = rlength;
	    break;
	  }
	  ret += rlength;
	  if (rlength < length) break;
	  offset += length;
	  image += length;
	  len -= length;
	}
	/* Restore the base register if it was changed. */
	if (savebase) pciWriteLong(Tag, PCI_MAP_REG_START + (b_reg << 2),
				   (CARD32) savebase);
	/* Restore ROM address decoding */
	pciWriteLong(Tag, PCI_MAP_ROM_REG, romsave);
	return ret;
	
    }
    /* Restore ROM address decoding */
    pciWriteLong(Tag, PCI_MAP_ROM_REG, romsave);
    return 0;
}

#if 0
static int
readPciBIOS(unsigned long Offset, PCITAG Tag, int basereg,
		unsigned char *Buf, int Len)
{
    ADDRESS hostbase;
    CARD8 *image = Buf;
    unsigned long offset;
    CARD32 romaddr, savebase = 0, romsave = 0, newbase = 0;
    int ret, tmpLen, length, rlength, n;
    /* XXX This assumes that memory access is enabled */

    /*
     * Check if the rom base address is assigned.  If it isn't, and if
     * a basereg was supplied, temporarily map the rom at that base
     * address.
     */
    romsave = pciReadLong(Tag, PCI_MAP_ROM_REG);
    romaddr = PCIGETROM(romsave);
    if ((newbase = getValidBIOSBase(Tag, &basereg)) != romaddr) {
RetryWithBase:
	romaddr = PCIGETROM(newbase);
	if (romaddr) {
	  /* move mem base out of the way if in conflicts with ROM */
	  if ((basereg >= 0) && (basereg <= 5)) {
	      if (!savebase)
	          savebase = pciReadLong(Tag, PCI_MAP_REG_START+(basereg<<2));
	      if (PCIGETROM(savebase) == romaddr) {
	          xf86MsgVerb(X_INFO,5,"xf86ReadPciBIOS: modifying membase[%i]"
			    " for device %i:%i:%i\n", basereg,
			    PCI_BUS_FROM_TAG(Tag), PCI_DEV_FROM_TAG(Tag),
			    PCI_FUNC_FROM_TAG(Tag));
		pciWriteLong(Tag, PCI_MAP_REG_START + (basereg << 2),
		    (CARD32)~0);
	    }
	  }
	}
    }


    if (romaddr == 0) {
	xf86Msg(X_WARNING, "xf86ReadPciBIOS: cannot locate a BIOS address\n");
	return -1;
    } 
    xf86MsgVerb(X_INFO, 5,
	"xf86ReadPciBIOS: found ValidBIOSBase for %i:%i:%i: %x\n",
	PCI_BUS_FROM_TAG(Tag), PCI_DEV_FROM_TAG(Tag), PCI_FUNC_FROM_TAG(Tag),
	newbase);

    hostbase = pciBusAddrToHostAddr(Tag, PCI_MEM, PCIGETROM(romaddr));
    xf86MsgVerb(X_INFO, 5, "ReadPciBIOS: base = 0x%x\n",romaddr);
    /* Enable ROM address decoding */
    pciWriteLong(Tag, PCI_MAP_ROM_REG, romaddr | PCI_MAP_ROM_DECODE_ENABLE);

    /* Check to see if we really have a PCI BIOS image */
    rlength = xf86ReadBIOS(hostbase, 0, tmp_buf, sizeof(tmp_buf));
    if (rlength < 0) return rlength;
    /* If we found a BIOS image we read the requested data */
    if ((rlength == sizeof(tmp_buf)) && (tmp_buf[0] == 0x55) 
	 && (tmp_buf[1] == 0xaa) && tmp_buf[2] ) {
    
        /* Read BIOS in 64kB chunks */
        ret = 0;
	offset = Offset;
	tmpLen = Len;
	image = Buf;
	
	while ((length = tmpLen) > 0) {
	    if (length > 0x10000) length = 0x10000;
	    rlength = xf86ReadBIOS(hostbase, offset, image, length);
	    if (rlength < 0) {
	        ret = rlength;
		break;
	    }
	    ret += rlength;
	    if (rlength < length) break;
	    offset += length;
	    image += length;
	    tmpLen -= length;
	}
    } else {
        /* If we don't have a PCI BIOS image we look further */
	n = 0;
	if ((basereg >= 0) && (basereg <= 5) && xf86PciVideoInfo) do {
	    pciVideoPtr pvp;

	    if (!(pvp = xf86PciVideoInfo[n++])) break;
	    if (pciTag(pvp->bus, pvp->device, pvp->func) == Tag) {
		if (newbase == pvp->memBase[basereg]) break;
		newbase = pvp->memBase[basereg];
		goto RetryWithBase;
	    }
	} while (1);
    }

    /* Restore ROM address decoding */
    pciWriteLong(Tag, PCI_MAP_ROM_REG, romsave);
    /* Restore the base register if it was changed. */
    if (savebase)
	pciWriteLong(Tag, PCI_MAP_REG_START + (basereg << 2), savebase);

    return ret;
}
#endif

typedef CARD32 (*ReadProcPtr)(PCITAG, int);
typedef void (*WriteProcPtr)(PCITAG, int, CARD32);

int
xf86ReadPciBIOS(unsigned long Offset, PCITAG Tag, int basereg,
		unsigned char *Buf, int Len)
{
  int size, num;
  CARD32 Acc1, Acc2;
  PCITAG *pTag;
  int i;

  size = readPciBIOS(Offset,Tag,basereg,Buf,Len);
  
  if ((size == Len) && (Buf[0] == 0x55) && (Buf[1] == 0xaa) && Buf[2] &&
      (Len >= (Buf[2] << 9)))
    return size;

  num = pciTestMultiDeviceCard(PCI_BUS_FROM_TAG(Tag),
			       PCI_DEV_FROM_TAG(Tag),
			       PCI_FUNC_FROM_TAG(Tag),&pTag);
  
  if (!num) return size;

#define PCI_ENA (PCI_CMD_MEM_ENABLE | PCI_CMD_IO_ENABLE)
  Acc1 = ((ReadProcPtr)(pciLongFunc(Tag,READ)))(Tag,PCI_CMD_STAT_REG);
  ((WriteProcPtr)(pciLongFunc(Tag,WRITE)))(Tag,
					   PCI_CMD_STAT_REG,(Acc1 & ~PCI_ENA));
  
  for (i = 0; i < num; i++) {
    Acc2 = ((ReadProcPtr)(pciLongFunc(pTag[i],READ)))(pTag[i],PCI_CMD_STAT_REG);
    ((WriteProcPtr)(pciLongFunc(pTag[i],WRITE)))(pTag[i],
					     PCI_CMD_STAT_REG,(Acc2 | PCI_ENA));
    size = readPciBIOS(Offset,pTag[i],0,Buf,Len);
    ((WriteProcPtr)(pciLongFunc(pTag[i],WRITE)))(pTag[i],PCI_CMD_STAT_REG,Acc2);
    if ((size == Len) && (Buf[0] == 0x55) && (Buf[1] == 0xaa) && Buf[2] &&
	(Len >= (Buf[2] << 9)))
      break;
  }
  ((WriteProcPtr)(pciLongFunc(Tag,WRITE)))(Tag,PCI_CMD_STAT_REG,Acc1);
  return size;
}  

#endif /* INCLUDE_XF86_MAP_PCI_MEM */

