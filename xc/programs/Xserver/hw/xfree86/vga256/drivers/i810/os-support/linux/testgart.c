/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/os-support/linux/testgart.c,v 1.1.2.2 1999/11/18 19:06:20 hohndel Exp $ */
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "agpgart.h"

unsigned char *gart;
int gartfd;

int usec( void ) {
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday( &tv, &tz );
  return (tv.tv_sec & 2047) * 1000000 + tv.tv_usec;
}

int MemoryBenchmark( void *buffer, int dwords ) {
  int             i;
  int             start, end;
  int             mb;
  int             *base;
  
  base = (int *)buffer;
  start = usec();
  for ( i = 0 ; i < dwords ; i += 8 ) {
    base[i] =
      base[i+1] =
      base[i+2] =
      base[i+3] =
      base[i+4] =
      base[i+5] =
      base[i+6] =
      base[i+7] = 0x15151515;         /* dmapad nops */
  }
  end = usec();
  mb = ( (float)dwords / 0x40000 ) * 1000000 / (end - start);
  printf("MemoryBenchmark: %i mb/s\n", mb );
  return mb;
}



void insert_gart(int page)
{
  if (ioctl(gartfd, GARTIOCINSERT, &page) != 0)
    {
      perror("ioctl(GARTIOCINSERT)");
      exit(1);
    }
}

void remove_gart(int page)
{
  if (ioctl(gartfd, GARTIOCREMOVE, &page) != 0)
    {
      perror("ioctl(GARTIOCREMOVE)");
      exit(1);
    }
}

void BenchMark()
{
  int i, worked = 1;

/*    i = MemoryBenchmark(gart, 1024 * 1024 * 4) + */
/*      MemoryBenchmark(gart, 1024 * 1024 * 4) + */
/*      MemoryBenchmark(gart, 1024 * 1024 * 4); */
  
/*    printf("Average speed: %i mb/s\n", i /3); */
  
  printf("Testing data integrity : ");
  fflush(stdout);
  
  for (i=0; i < 32 * 1024 * 4; i++)
    {
      gart[i] = i % 255;	/* not 256, which is a factor of page size */
    }
  
  for (i=0; i < 32 * 1024 * 4; i++)
    {
       if (gart[i] != i % 255) {
	  printf("i: %d wanted %d got %d\n", i, i % 255, gart[i]);
	  worked = 0;
       }
    }
  
  if (!worked)
    printf("failed!\n");
  else
    printf("passed.\n");
}

int main()
{
  struct gart_info gi;
  int i;
  
  gartfd = open("/dev/agpgart", O_RDWR);
  if (gartfd == -1)
    {	
      perror("open");
      exit(1);
    }
  
  
  if (ioctl(gartfd, GARTIOCINFO, &gi) != 0)
    {
      perror("ioctl");
      exit(1);
    }

  printf("gart:\n");
  printf("  physical: 0x%x\n", gi.physical);
  printf("  size (mb): %d\n", gi.size);
  printf("  normal pages: %d\n", gi.num_of_slots - gi.num_dcache_slots);
  printf("  dcache pages: %d\n", gi.num_dcache_slots);
  printf("  agpmode: %d\n", gi.agpmode);
  
  gart = mmap(NULL, gi.size * 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED, gartfd, 0);
  
  if (gart == (unsigned char *) 0xffffffff)
    {
      perror("mmap");
      close(gartfd);
      exit(1);
    }	
  
     // allocate 128k
     for (i=0; i<32; i++)
	insert_gart(i);

  printf("Allocated 128k of GART memory\n");

  BenchMark();  
/*    printf("Switching to agp x2 mode\n"); */
/*    i = 2; */
/*    ioctl(gartfd, GARTIOCMODE, &i); */
/*    BenchMark(); */
  
     for (i=0; i<32; i++)
	remove_gart(i);  
  
  close(gartfd);
  
  return 0;
}

