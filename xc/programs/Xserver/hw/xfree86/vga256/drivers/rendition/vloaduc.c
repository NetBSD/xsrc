/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vloaduc.c,v 1.1.2.2 1998/08/07 06:40:25 hohndel Exp $ */
/*
 * includes
 */

#include "v1kregs.h"
#include "vloaduc.h"
#include "vos.h"
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*
 * defines 
 */

#ifdef LITTLE_ENDIAN

/* maybe swap word */
#define SW32(x) swapl(x)
#define SW16(x) swaps(x)
#else /* BIG_ENDIAN */
#define SW32(x) (x)
#define SW16(x) (x)
#endif /* #ifdef LITTLE_ENDIAN */



/*
 * local function prototypes 
 */

#ifdef LITTLE_ENDIAN 
static vu32 swapl(vu32 x);
static vu16 swaps(vu16 x);
#endif
void loadSection2board(struct v_board_t *board, int fd, Elf32_Shdr *shdr);
void loadSegment2board(struct v_board_t *board, int fd, Elf32_Phdr *phdr);
static int seek_and_read_hdr(int fd, void *ptr, long int offset, int size, 
                             int cnt);
static void mmve(struct v_board_t *board, vu32 size, vu8 *data, vu32 phys_addr);



/*
 * functions
 */

/* 
 * int v_load_ucfile(struct v_board_t *board, char *file_name)
 *
 * Loads verite elf file microcode file in |name| onto the board.
 * NOTE: Assumes the ucode loader is already running on the board!
 * 
 * Returns the program's entry point, on error -1;
 */
int v_load_ucfile(struct v_board_t *board, char *file_name)
{
  int num;
  int sz;
  int fd;
  Elf32_Phdr *pphdr, *orig_pphdr=NULL;
  Elf32_Shdr *pshdr, *orig_pshdr=NULL;
  Elf32_Ehdr ehdr ;

#ifdef DEBUG
  fprintf(stderr, "RENDITION: Loading microcode %s\n", file_name); 
#endif

  /* open file and read ELF-header */
  if (-1 == (fd=open(file_name, O_RDONLY))) {
    fprintf(stderr, "RENDITION: Cannot open microcode %s\n", file_name); 
    return -1;
  }

  if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
    fprintf(stderr, "RENDITION: Cannot read microcode %s\n", file_name); 
    return -1;
  }
  if (0 != strncmp((char *)&ehdr.e_ident[1], "ELF", 3)) {
    fprintf(stderr, "RENDITION: Microcode %s(%s) is corrupt\n", 
               file_name, &ehdr.e_ident[1]); 
    return -1;
  }

  /* read in the program header(s) */
  sz=SW16(ehdr.e_phentsize);
  num=SW16(ehdr.e_phnum);
  if (0!=sz && 0!=num) {
	orig_pphdr=pphdr=(Elf32_Phdr *)malloc(sz*num);
	if (!pphdr) {
      fprintf(stderr, "RENDITION: Cannot allocate global memory (1)\n"); 
      close(fd);
      return -1;
    }

	if (seek_and_read_hdr(fd, pphdr, SW32(ehdr.e_phoff), sz, num)) {
      fprintf(stderr, "RENDITION: Error reading microkernel (1)\n");
      close(fd);
      return -1;
    }

	orig_pshdr=pshdr=(Elf32_Shdr *)0;
  }
  else {
	orig_pphdr=pphdr=(Elf32_Phdr *)0;

    /* read in the section header(s) */
    sz=SW16(ehdr.e_shentsize);
    num=SW16(ehdr.e_shnum);
    if (0!=sz && 0!=num) {
      orig_pshdr=pshdr=(Elf32_Shdr *)malloc(sz*num);
      if (!pshdr) {
        fprintf(stderr, "RENDITION: Cannot allocate global memory (2)\n"); 
        close(fd);
        return -1;
      }

      if (seek_and_read_hdr(fd, pshdr, SW32(ehdr.e_shoff), sz, num)) {
        fprintf(stderr, "RENDITION: Error reading microcode (2)\n");
        close(fd);
        return -1;
      }
    }
    else
      pshdr=(Elf32_Shdr *)0;
  }

  if (pphdr) {
    do {
	  if (SW32(pphdr->p_type) == PT_LOAD) 
        loadSegment2board(board, fd, pphdr);
        pphdr=(Elf32_Phdr *)(((char *)pphdr)+sz);
      } while (--num);
      free(orig_pphdr);
  }    
  else {
    do {
      if (SW32(pshdr->sh_size) && (SW32(pshdr->sh_flags) & SHF_ALLOC)
          && ((SW32(pshdr->sh_type)==SHT_PROGBITS) 
               || (SW32(pshdr->sh_type)==SHT_NOBITS))) 
        loadSection2board(board, fd, pshdr);
	  pshdr=(Elf32_Shdr *)(((char *)pshdr)+sz);
	} while (--num) ;
	free(orig_pshdr);
  }

  close(fd);

  return SW32(ehdr.e_entry);
}



/*
 * local functions
 */

#ifdef LITTLE_ENDIAN 
static vu32 swapl(vu32 x)
{
  vu32 tmp;

  tmp=x>>8;
  tmp&=0xFF00FF;
  x&=0xFF00FF;
  x<<=8;
  x|=tmp;
  tmp=x>>16;
  return (x<<16)|tmp;
}



static vu16 swaps(vu16 x)
{
  return (x<<8)+(x>>8);
}
#endif /* #ifdef LITTLE_ENDIAN */



void loadSection2board(struct v_board_t *board, int fd, Elf32_Shdr *shdr)
{
  fprintf(stderr, "vlib: loadSection2board not implemented yet!\n");
}



void loadSegment2board(struct v_board_t *board, int fd, Elf32_Phdr *phdr)
{
  vu8 *data;
  vu32 offset=SW32(phdr->p_offset);
  vu32 size=SW32(phdr->p_filesz);
  vu32 physAddr=SW32(phdr->p_paddr);
    
  if (lseek(fd, offset, SEEK_SET) != offset) {
	fprintf(stderr, "RENDITION: Failure in loadSegmentToBoard, offset %x\n", offset);
    abort();
  }

  data=(vu8 *)malloc(size);
  if (NULL == data)
	fprintf(stderr, "RENDITION: GlobalAllocPtr couldn't allocate %x bytes", size);

  if (read(fd, data, size) != size)
	fprintf(stderr, "RENDITION: v_readfile Failure, couldn't read %x bytes ", size);

#ifdef DEBUG
  fprintf(stderr, "RENDITION: Writing %d bytes to 0x%x\n", size, physAddr);
#endif
  mmve(board, size, data, physAddr);

  free(data);
}



static int seek_and_read_hdr(int fd, void *ptr, long int offset, int size, 
                             int cnt)
{
  if (lseek(fd, offset, SEEK_SET) != offset)
	return 1 ;

  if (size*cnt != read(fd, ptr, size*cnt))
	return 2 ;

  return 0 ;
}



static void mmve(struct v_board_t *board, vu32 size, vu8 *data, vu32 phys_addr)
{
  vu32 out;
  vu32 *dataout;
  vu8 *vmb=board->vmem_base;

  /* swap bytes 3<>0, 2<>1 */
  v_out8(board->io_base+MEMENDIAN, MEMENDIAN_END);

  out=phys_addr/4;
  dataout=(vu32 *)data;

  while (size > 0) {
    v_write_memory32(vmb, out, *dataout);
	out++;
	dataout++;
	size-=4;
  }
}



/*
 * end of file vloaduc.c
 */
