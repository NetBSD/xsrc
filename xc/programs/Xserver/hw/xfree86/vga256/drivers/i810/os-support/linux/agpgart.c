/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/os-support/linux/agpgart.c,v 1.1.2.2 1999/11/18 19:06:20 hohndel Exp $ */
/*
 * AGPGART Hardware Device Driver
 * Copyright (C) 1999 Jeff Hartmann
 * Copyright (C) 1999 Precision Insight
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JEFF HARTMANN, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    Jeff Hartmann <jhartmann@precisioninsight.com>
 */

/*
 * - added support for VIA MVP3 <A.Borrmann@tu-bs.de>
 *   (introduced "chipset" struct)
 *
 * - added support for i810, changed to use a scratch page
 *   for empty entries. <keithw@precisioninsight.com>
 *
 * - for initial i810 release, removed all other chipset support as
 *   the i810 changes haven't been tested on those motherboards.
 *   <keithw@precisioninsight.com>
 */

/* mknod /dev/gart c 10 175 */

#include <linux/config.h>
#include <linux/version.h>

#ifdef MODULE
#include <linux/module.h>
#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#else
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/malloc.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include "agpgart.h"

#ifndef HAVE_MTRR 
#undef CONFIG_MTRR
#endif 

/* The dc100 doesn't like MTRR...  But we don't program the
 * dcache from the cpu...
 */
/*  #undef CONFIG_MTRR */

#ifdef CONFIG_MTRR 
#include <asm/mtrr.h>
#endif

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#define TRUE 1
#define FALSE 0

#define AGPGART_MINOR 175

MODULE_AUTHOR("Jeff Hartmann <slicer@ionet.net>");
MODULE_PARM(gart_agpmode, "1i");

static int gart_agpmode __initdata = 1; /* default agp mode = x1 ! */
static struct miscdevice gart_miscdev;

enum chipset_type { NOT_SUPPORTED,  INTEL_810};


static struct chipset_functions
{
  enum chipset_type type;
  char cache_flush_req;
  struct pci_dev *dev;
  struct pci_dev *agp_bridge;
   
  int  (*initialize)(void);
  int  (*fetch_size)(void);
  int  (*configure)(int);
  void (*cleanup)(void);
  int (*set_mode)(int);
  void (*tlb_flush)(void);
  void (*write_entry)(int, unsigned long);
  void (*conf_agpbridge)(int);
} chipset;

struct gart_table_init
{
   int size;
   int num_entries;
   int page_order;
   short int intel_size_values;
   int via_size_values;
   int num_dcache_entries;	/* defaults to zero below. */
};

static struct gart_table_init gart_init_values[] =
{
     /*  size, 	number of entries, 	page order */
        {4,   	1024,		        0,		63, 252},
        {8,   	2048,  			1,		62, 248},
	{16,  	4096,  			2,		60, 240},
	{32,  	8192,  			3,		56, 224},
	{64,  	16384, 			4,		48, 192},
	{128, 	32768, 			5,		32, 128},
	{256, 	65536, 			6,		 0, 0}
};

/* Use a copy of the entry so we can change stuff.
 */
static struct gart_table_init gart_init;
static unsigned long table_bus_addr;
static unsigned long gart_map_phys;
static unsigned long gart_mask;

#if defined(CONFIG_MTRR)
static int gart_mtrr;
#endif

static unsigned long *gart_table;
static unsigned long scratch_page = 0;

#ifdef __SMP__
static atomic_t cpus_waiting;
#endif

static void flush_cache(void)
{
   asm volatile("wbinvd" : : : "memory" );
}

#ifdef __SMP__
static void ipi_handler(void *info)
{
   flush_cache();
   atomic_dec(&cpus_waiting);
   while( atomic_read(&cpus_waiting) > 0) barrier();
}

static void smp_flush_cache(void)
{
   atomic_set(&cpus_waiting, smp_num_cpus - 1);
   if(smp_call_function(ipi_handler, NULL, 1, 0) != 0)
     panic("agpgart: timed out waiting for the other CPUs\n");
   flush_cache();
   while (atomic_read (&cpus_waiting) > 0) barrier();
}
#endif


#ifdef __SMP__
#define FLUSH_CACHE()	 smp_flush_cache()
#else
#define FLUSH_CACHE()	 flush_cache()
#endif


#define PGE_EMPTY(p) (!(p) || (p) == scratch_page)

static void *gart_alloc_page(void)
{
   void *pt;

   pt = (void *) __get_free_page(GFP_KERNEL);
   if (pt == NULL)
     {
	return(NULL);
     }
   
   set_bit(PG_reserved, &mem_map[MAP_NR(pt)].flags);
   
   return(pt);
}

static void gart_destroy_page(void *pt)
{
   if(pt == NULL)
     return;
   
   clear_bit(PG_reserved, &mem_map[MAP_NR(pt)].flags);
   
   free_page((unsigned long) pt);
}

static int gart_insert_page(int gart_index)
{
void *tmp_ptr;
unsigned long tmp_value;
   
   if(gart_index < 0 || 
      gart_index >= gart_init.num_entries)
     {
	return(-EINVAL);
     }
   
   tmp_value = gart_table[gart_index];
   
   if(!PGE_EMPTY(tmp_value))
     {
	return(-EBUSY);
     }
   
   tmp_ptr = gart_alloc_page();
   
   if(tmp_ptr == NULL)
     {
	return(-ENOMEM);
     }
   
   tmp_value = virt_to_bus(tmp_ptr);
   tmp_value |= gart_mask;
   chipset.write_entry(gart_index, tmp_value);

   if (	chipset.cache_flush_req ) 
      FLUSH_CACHE();

   chipset.tlb_flush();
   return(0);
}



static int gart_remove_page(int gart_index, int flush)
{
   unsigned long tmp_value;

   if (gart_index < 0 || 
       gart_index >= gart_init.num_entries)
   {
      return(-EINVAL);
   }
   
   if(scratch_page == 0) {
      void *tmp_ptr = gart_alloc_page();

      if(tmp_ptr == NULL)
	 return(-ENOMEM);

      scratch_page = virt_to_bus(tmp_ptr);
      scratch_page |= gart_mask;
   }	

   tmp_value = gart_table[gart_index];
   chipset.write_entry(gart_index, scratch_page);

   if (flush) {
      if (chipset.cache_flush_req) 
	 FLUSH_CACHE();

      chipset.tlb_flush();
   }

   if(!PGE_EMPTY(tmp_value))
   {
      void *tmp_ptr;
      tmp_value &= ~(gart_mask);
      tmp_ptr = (void *) bus_to_virt(tmp_value);
      gart_destroy_page(tmp_ptr);
   }

   return(0);
}



static int gart_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		      unsigned long arg)
{
int gart_index, mode;
struct gart_info user_info;
struct gart_pge_info user_pge_info; 
   
   switch(cmd)
     {
      case GARTIOCMODE:	     
	if(!(file->f_mode&FMODE_WRITE))
	   return -EPERM;

	if(get_user(mode, (int *)arg))
	  {
	     return(-EFAULT);
	  }
        printk(KERN_INFO "request to set agp x%d mode\n", mode);
        if ((mode==1) || (mode==2))
  		return(chipset.set_mode(mode));
       	else
	 	return(-EINVAL);
	break;
       
      case GARTIOCINSERT:
	if(!(file->f_mode&FMODE_WRITE))
	   return -EPERM;

	if(get_user(gart_index, (int *)arg))
	  {
	     return(-EFAULT);
	  }
	return(gart_insert_page(gart_index));
	break;
  	
     case GARTIOCREMOVE:
	if(!(file->f_mode&FMODE_WRITE))
	   return -EPERM;

	if(get_user(gart_index, (int *) arg))
	  {
	     return(-EFAULT);
	  }
	return(gart_remove_page(gart_index, 1));
	break;

      case GARTIOCPGINFO:
	if(get_user(gart_index, &((struct gart_pge_info *) arg)->index) )
	  {
	     return(-EFAULT);
	  }

	if (gart_index < 0 || 
	    gart_index >= gart_init.num_entries ||
	    PGE_EMPTY(gart_table[gart_index])) {
	   printk("index: %x max: %x entry: %x empty: %d\n",
		  gart_index, gart_init.num_entries, gart_table[gart_index],
		  PGE_EMPTY(gart_table[gart_index]));
	   return -EINVAL;
	}

	user_pge_info.index = gart_index;
	user_pge_info.physical = gart_table[gart_index] & ~gart_mask;

	if(copy_to_user((void *)arg, &user_pge_info, 
			sizeof(struct gart_pge_info)))
	  {
	     return(-EFAULT);
	  }
	return 0;
	break;

      case GARTIOCINFO:
	user_info.size = gart_init.size;
	user_info.num_of_slots = gart_init.num_entries;
	user_info.num_dcache_slots = gart_init.num_dcache_entries;
	user_info.physical = gart_map_phys;
	user_info.agpmode = gart_agpmode;

	if(copy_to_user((void *)arg, &user_info, sizeof(struct gart_info)))
	  {
	     return(-EFAULT);
	  }
	return 0;
     }
   return(-ENOTTY);
}

static int gart_create_table(int size /* in megs */)
{
   /* 4   megs - one page	  - 4096   bytes - 1024  entries - 0 order */
   /* 8   megs - two pages 	  - 8192   bytes - 2048  entries - 1 order */
   /* 16  megs - four pages 	  - 16384  bytes - 4096  entries - 2 order */
   /* 32  megs - eight pages 	  - 32768  bytes - 8192  entries - 3 order */
   /* 64  megs - sixteen pages 	  - 65536  bytes - 16384 entries - 4 order */
   /* 128 megs - thirty-two pages - 131072 bytes - 32768 entries - 5 order */
   /* 256 megs - sixty-four pages - 262144 bytes - 65536 entries - 6 order */
      
   char *table;
   char *table_end;
   struct gart_table_init *gart_init_p;
   int i;
   
   for(i = 0, gart_init_p = NULL; i < 7 && gart_init_p == NULL; i++)
     {
	if(size == gart_init_values[i].size) 
	  {
	     gart_init_p = gart_init_values + i;
	  }
     }
   
   if(gart_init_p == NULL)
     {
	return(-EINVAL);
     }

   gart_init = *gart_init_p;

   table = (char *) __get_free_pages(GFP_KERNEL, gart_init.page_order);
   
   if(table == NULL)
     {
	return(-ENOMEM);
     }
   
   table_bus_addr = virt_to_bus(table);
   table_end = table + ((PAGE_SIZE * (1 << gart_init.page_order)) - 1);

   for(i = MAP_NR(table); i < MAP_NR(table_end); i++)
     {
	set_bit(PG_reserved, &mem_map[i].flags);
     }
   
   memset(table, 0, (PAGE_SIZE * (1 << gart_init.page_order)));
   gart_table = (unsigned long *) table;
   return(0);
}


/* Normal behaviour, overridden in the i810.
 */
static int gart_get_gart(void)
{
   int ret_val;
   int size, old_size;

   old_size = size = chipset.fetch_size();
   ret_val = -1;
   
   while(size >= 4 && ret_val != 0)
     {
	ret_val = gart_create_table(size);
	size = size / 2;
     }
   
   if(ret_val != 0)
     {
	printk( "Unable to get memory for the GART table.\n");
        misc_deregister(&gart_miscdev);
	return(-ENOMEM);
     }

   return 0;
}

/* Normal behaviour, overridden in the i810.
 */
void gart_write_entry(int entry, unsigned long value) 
{
   gart_table[entry] = value;
}



/* ------------------------------------------------------------------------
 * Intel 810 
 *    - need to map the mmio region to talk to the GTT.
 */

#define I810_GMADDR 0x10	/* first pci addr range */
#define I810_MMADDR 0x14	/* second pci addr range */

#define I810_PTE_BASE          0x10000
#define I810_PTE_MAIN_UNCACHED   0x00000000
#define I810_PTE_LOCAL           0x00000002
#define I810_PTE_VALID           0x00000001

#define I810_SMRAM_MISCC       0x70
#define I810_GFX_MEM_WIN_SIZE    0x00010000
#define I810_GFX_MEM_WIN_32M     0x00010000
#define I810_GMS                 0x000000c0
#define I810_GMS_DISABLE         0x00000000

#define I810_PGETBL_CTL        0x2020
#define I810_PGETBL_ENABLED      0x00000001

#define I810_DRAM_CTL          0x3000
#define I810_DRAM_ROW_0          0x00000001
#define I810_DRAM_ROW_0_SDRAM    0x00000001


#ifndef PCI_DEVICE_ID_INTEL_810_0
#define PCI_DEVICE_ID_INTEL_810_0       0x7120
#define PCI_DEVICE_ID_INTEL_810_DC100_0 0x7122
#define PCI_DEVICE_ID_INTEL_810_E_0     0x7124

#define PCI_DEVICE_ID_INTEL_810_1       0x7121
#define PCI_DEVICE_ID_INTEL_810_DC100_1 0x7123
#define PCI_DEVICE_ID_INTEL_810_E_1     0x7125
#endif

static volatile unsigned char *mmvirt = 0;

#define OUTREG(addr, val)   *(volatile unsigned int *)(mmvirt + (addr)) = (val)
#define INREG(addr)         *(volatile unsigned int *)(mmvirt + (addr))


/* The i810 doesn't support multiple size gtt tables - there is only
 * the one option, it appears.
 */
static int i810_get_gart(void)
{
   int ret_val = -1;
   int size;
   int smram_miscc;
   unsigned int mmbase;

   /* Find and map the mmio region for the graphics chip.  This is just
    * the second base address in the graphics chip pci conf area.
    */ 
   pci_read_config_dword(chipset.agp_bridge, I810_MMADDR, (u32 *) &mmbase);   
   mmbase &= 0xfff80000;
   
   /* Map the mmio region (must be used to program the gtt)
    */   
   mmvirt = ioremap( mmbase, 512 * 1024 );

   /* Figure out the GTT size
    */
   pci_read_config_dword(chipset.dev, I810_SMRAM_MISCC, &smram_miscc );

   if ( (smram_miscc & I810_GMS) == I810_GMS_DISABLE ) {
      printk("i810 is disabled\n");
      return -ENOMEM;
   }
   
   if ((smram_miscc & I810_GFX_MEM_WIN_SIZE) == I810_GFX_MEM_WIN_32M) 
      size = 32;
   else
      size = 64;

   /* Initialize gart_init.
    */
   if (gart_create_table(size) != 0)
   {
      printk( "Unable to get memory for the GART table.\n");
      misc_deregister(&gart_miscdev);
      return(-ENOMEM);
   }
   
   if ((INREG(I810_DRAM_CTL) & I810_DRAM_ROW_0) == I810_DRAM_ROW_0_SDRAM) 
   {
      printk( "agpgart (i810): detected 4MB dedicated video ram\n");

      gart_init.num_dcache_entries = 1024;
      gart_init.num_entries -= 1024;
   }
 
   return 0;
}


/* Have to read via the gart_table, but write via mmio - very complex...
 */
static void i810_write_entry( int entry, unsigned long val )
{
   OUTREG(I810_PTE_BASE + (entry * 4), val);
}



/* Agp is hidden or not implemented on the 810 (the graphics device is
 * integrated to the pci bridge).
 */
static void i810_configure_agpbridge(int size)
{
}

static int i810_setmode(int agp_mode)
{
   return 0;
}

/* Writing via mmio already flushes all tlb's on the graphics device.
 */
static void i810_tlbflush( void )
{
}


static int i810_configure(int agp_mode)
{
   unsigned long temp;
   int i;

   /* gmaddr - physical address to remap too (a pci range of this device) 
    */
   pci_read_config_dword(chipset.agp_bridge, I810_GMADDR, (u32 *) (&temp));
   gart_map_phys = (temp & PCI_BASE_ADDRESS_MEM_MASK);

   OUTREG(I810_PGETBL_CTL, table_bus_addr | I810_PGETBL_ENABLED);


   if (chipset.cache_flush_req) 
      FLUSH_CACHE();

   /* Load the dcache, if present.
    */
   for (i = gart_init.num_entries ; 
	i < gart_init.num_entries + gart_init.num_dcache_entries ;
	i++) {
      OUTREG(I810_PTE_BASE + i * 4, 
	     (i*4096) | I810_PTE_LOCAL | I810_PTE_VALID);
   }

   printk("i810_configure");

   return 0;
}

static void i810_cleanup(void)
{
   OUTREG(I810_PGETBL_CTL, 0);
   printk("i810_cleanup");
}






/* KW - removed (not-tested) code for all GART's execept the i810.
 */
static void * gart_find_supported(void)
{
   struct pci_dev *dev = NULL;
   struct pci_dev *agpb_dev = NULL;

   chipset.type = NOT_SUPPORTED;
   

   dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			 PCI_DEVICE_ID_INTEL_810_0,
			 NULL);
   agpb_dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			      PCI_DEVICE_ID_INTEL_810_1,
			      NULL);
   if((dev != NULL) && (agpb_dev != NULL))
     {
	     chipset.type	= INTEL_810;
	     chipset.dev	= dev;
	     chipset.agp_bridge = agpb_dev;
     }


   dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			 PCI_DEVICE_ID_INTEL_810_DC100_0,
			 NULL);
   agpb_dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			      PCI_DEVICE_ID_INTEL_810_DC100_1,
			      NULL);
   if((dev != NULL) && (agpb_dev != NULL))
     {
	     chipset.type	= INTEL_810;
	     chipset.dev	= dev;
	     chipset.agp_bridge = agpb_dev;
     }


   dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			 PCI_DEVICE_ID_INTEL_810_E_0,
			 NULL);
   agpb_dev = pci_find_device(PCI_VENDOR_ID_INTEL,
			      PCI_DEVICE_ID_INTEL_810_E_1,
			      NULL);
   if((dev != NULL) && (agpb_dev != NULL))
     {
	     chipset.type	= INTEL_810;
	     chipset.dev	= dev;
	     chipset.agp_bridge = agpb_dev;
     }



   switch(chipset.type)
     {
     case INTEL_810:
	/* Will need a different mask for local (dedicated) video ram.
	 */
	gart_mask 		= I810_PTE_MAIN_UNCACHED | I810_PTE_VALID;
	chipset.cache_flush_req = TRUE;

	chipset.initialize      = i810_get_gart;
	chipset.configure  	= i810_configure;
	chipset.fetch_size 	= 0;
	chipset.cleanup    	= i810_cleanup;
	chipset.set_mode   	= i810_setmode;
	chipset.tlb_flush  	= i810_tlbflush;
	chipset.write_entry  	= i810_write_entry;
	chipset.conf_agpbridge 	= i810_configure_agpbridge;

	return(chipset.dev);
	break;
     }
   
   return(NULL);
}


static ssize_t gart_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
   return -EINVAL;
}

static ssize_t gart_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
   return -EINVAL;
}

static int gart_mmap(struct file *file, struct vm_area_struct *vma)
{
   int size;
   
   size = vma->vm_end - vma->vm_start;
   if(size != (gart_init.size * 0x100000))
     {
	return(-EINVAL);
     }
   if(remap_page_range(vma->vm_start, gart_map_phys, 
		       size, vma->vm_page_prot))
     {
	return(-EAGAIN);
     }
   return(0);
}

static int gart_release(struct inode *inode, struct file *file)
{
   MOD_DEC_USE_COUNT;
   return 0;
}

static long long gart_lseek(struct file *file, long long offset, int origin)
{
   return -ESPIPE;
}					 

static int gart_open(struct inode *inode, struct file *file)
{
#if 0
   int minor = MINOR(inode->i_rdev);
   
   if(minor != 0)
     return(-ENXIO);
#endif
   MOD_INC_USE_COUNT;
   return(0);
}

static struct file_operations gart_fops =
{
   gart_lseek,
   gart_read,
   gart_write,
   NULL,
   NULL,
   gart_ioctl,
   gart_mmap,
   gart_open,
   NULL,
   gart_release
};

static struct miscdevice gart_miscdev =
{
   AGPGART_MINOR,
   "agpgart",
   &gart_fops
};

void gart_free_table(void)
{
   int i;
   char *table, *table_end;
   void *tmp_ptr;
   unsigned long tmp_value;
   
   
   for(i = 0; i < gart_init.num_entries; i++)
     {
	if(gart_table[i] != 0)
	  {
	     tmp_value = gart_table[i];
	     gart_table[i] = 0;
	     tmp_value &= ~(gart_mask);
	     tmp_ptr = (void *) bus_to_virt(tmp_value);
	     gart_destroy_page(tmp_ptr);
	  }
     }
   
   table = (char *)gart_table;
   table_end = table + ((PAGE_SIZE * (1 << gart_init.page_order)) - 1);
     
   for(i = MAP_NR(table); i < MAP_NR(table_end); i++)
     {
	clear_bit(PG_reserved, &mem_map[i].flags);
     }   
   
   free_pages((unsigned long) gart_table, gart_init.page_order);
}

/* 
 * Main Initialization Function 
 */
static int gart_initialize(void)
{
int old_size=0;
int retval, i; 

   printk( "Linux AGP GART interface v0.03 (c) Jeff Hartmann\n");
   printk( "-- Experimental distribution, only i810 enabled\n");

   if(misc_register(&gart_miscdev))
     {
	printk("agp_gart: unable to get misc device: %d\n", AGPGART_MINOR);
	return(-EIO);
     }
   
   if (!gart_find_supported())
     {
	printk("agp_gart: no supported devices found\n");
        misc_deregister(&gart_miscdev);
	return(-EINVAL);
     }

   retval = chipset.initialize();
   
   if (retval != 0)
     {
	printk("agp_gart: initialization failed\n");
        misc_deregister(&gart_miscdev);
	return retval;
     }
   

   printk("Got pages for a %i megabyte mapping for the agp aperature.\n",
	   gart_init.size);
 
   chipset.configure(gart_agpmode);
   chipset.conf_agpbridge(old_size);
   
#ifdef CONFIG_MTRR
   gart_mtrr = mtrr_add(gart_map_phys, (gart_init.size * 0x100000), 
			MTRR_TYPE_WRCOMB, 1);
#endif	

   for (i = 0 ; i < gart_init.num_entries-1 ; i++) 
      gart_remove_page( i, 0 );      

   if (	chipset.cache_flush_req ) 
      FLUSH_CACHE();

   chipset.tlb_flush();

   return(0);
}

#ifdef MODULE
int init_module(void)
{
   return gart_initialize();
}

void cleanup_module(void)
{
#ifdef CONFIG_MTRR
   mtrr_del(gart_mtrr, gart_map_phys, (gart_init.size * 0x100000));
#endif
   chipset.cleanup();

   gart_free_table();

   misc_deregister(&gart_miscdev);
}
#else
void __init gart_setup(char *str, int *ints)
{
          int i;
          if (!strcmp(str,"gart_agpmode"))
                    agp_mode = ints[1];
}
#endif

