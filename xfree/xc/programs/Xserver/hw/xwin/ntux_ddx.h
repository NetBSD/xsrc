/*
* Copyright 2000 (C) Peter Busch
* All rights reserved.
NO WARRANTY; NO LIABILITY FOR DAMAGES:  THE MATERIALS ARE PROVIDED "AS IS" 
WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES
OF SATISFACTORY QUALITY, MERCHANTABILITY, NONINFRINGEMENT OF THIRD-PARTY
INTELLECTUAL PROPERTY, OR FITNESS FOR ANY PARTICULAR PURPOSE.
IN NO EVENT  SHALL Peter Busch BE LIABLE FOR ANY DAMAGES WHATSOEVER
(INCLUDING, WITHOUT  LIMITATION, DIRECT OR INDIRECT DAMAGES, DAMAGES FOR
LOSS OF PROFITS, BUSINESS  INTERRUPTION, LOSS OF INFORMATION) ARISING OUT
OF THE USE OF OR INABILITY TO  USE THE MATERIALS, EVEN IF
Peter Busch HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH  DAMAGES.
Without limiting the generality of the foregoing, no warranty is made
that the enclosed software will generate computer programs with the
characteristics or specifications desired by you or that the demo
software will be error-free.
THESE DISCLAIMERS OF WARRANTY CONSTITUTE AN ESSENTIAL PART OF THIS
AGREEMENT. 
*/
/* $XFree86: xc/programs/Xserver/hw/xwin/ntux_ddx.h,v 1.1 2000/08/10 17:40:37 dawes Exp $ */

/*
* ntux_ddx.h
* public header file for the ntux_ddx frame buffer library
* Copyright 2000 (C) Peter Busch
* All rights reserved.
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* This code is BETA and will be part of the CYGWIN/XFREE Project.
* It may not be used for purposes other than to contribute to the 
* CYGWIN/XFree project without prior written notice from the
* author.
* Any commercial use of this code will be permitted
* provided you contact Peter Busch at pbusch@dfki.de
*/


/*
* for use with the gcc compiler the exported get_surface routine
* is exported with the __cdecl calling convention
*/

#ifdef MSC_VER
char *__cdecl get_surface( unsigned long size );
#else
char *get_surface( unsigned long size );
#endif


#ifdef MSC_VER
char *__cdecl map_dx_fb( char *adr, unsigned long size );
#else
char *map_dx_fb( char *adr, unsigned long size );
#endif
/*
* definitions from xfddx_escape.h 
* (didn't want include the X-Server specific headers 
* needed to to include xfddx_escape.h)
*/
#define XFDDX_MAP_VIDEO_MEM			(0x1998120E)
#define XFDDX_UNMAP_VIDEO_MEM		(0x1998120F)
#define XFDDX_KMAP_VIDEO_MEM		(0x1998120B)
#define XFDDX_KUNMAP_VIDEO_MEM		(0x1998120C)


#define FILE_DEVICE_XFMAP			(0x00008383)
#define XFMAP_IOCTL_INDEX			(0x830)


typedef enum {
	NOTMAPPED,
	XFMAP_XFMAP,
	XFMAP_XFDDX_2,
	XFMAP_DX,
} xf_mapped;
	
/* define IOCTL codes */
#define XFMAP_MAP			( ULONG) CTL_CODE( FILE_DEVICE_XFMAP,XFMAP_IOCTL_INDEX + 0,	METHOD_NEITHER,	FILE_ANY_ACCESS )

#define XFMAP_UNMAP			( ULONG) CTL_CODE( FILE_DEVICE_XFMAP, XFMAP_IOCTL_INDEX + 1,METHOD_NEITHER,	FILE_ANY_ACCESS )


#define XFMAP_PMAP			( ULONG) CTL_CODE( FILE_DEVICE_XFMAP, XFMAP_IOCTL_INDEX + 3,METHOD_NEITHER,	FILE_ANY_ACCESS )


#define XFMAP_V2P			( ULONG) CTL_CODE( FILE_DEVICE_XFMAP,XFMAP_IOCTL_INDEX + 4,	METHOD_NEITHER,	FILE_ANY_ACCESS )



#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define XFMAP_SYS					"xfmap"
#define XFMAP_IMG					"xfmap.sys"



#ifndef __BUILD_XFDDX__

#ifndef _VIDEO_SHARE_MEMORY
typedef struct _VIDEO_SHARE_MEMORY 
{
    HANDLE	ProcessHandle;
    ULONG	ViewOffset;
    ULONG	ViewSize;
    PVOID	RequestedVirtualAddress;
} VIDEO_SHARE_MEMORY, *PVIDEO_SHARE_MEMORY;
#endif

#ifndef _VIDEO_SHARE_MEMORY_INFORMATION
typedef struct _VIDEO_SHARE_MEMORY_INFORMATION 
{
    ULONG	SharedViewOffset;
    ULONG	SharedViewSize;
    PVOID	VirtualAddress;
} VIDEO_SHARE_MEMORY_INFORMATION, *PVIDEO_SHARE_MEMORY_INFORMATION;
#endif


#ifndef _VIDEO_MEMORY
typedef struct _VIDEO_MEMORY
{
    PVOID RequestedVirtualAddress;
} VIDEO_MEMORY, *PVIDEO_MEMORY;
#endif

#ifndef _VIDEO_MEMORY_INFORMATION 
typedef struct _VIDEO_MEMORY_INFORMATION 
{
    PVOID VideoRamBase;
    ULONG VideoRamLength;
    PVOID FrameBufferBase;
    ULONG FrameBufferLength;
} VIDEO_MEMORY_INFORMATION, *PVIDEO_MEMORY_INFORMATION;
#endif
#endif	/* __BUILD_XFDDX__ */

/* prototypes */
HANDLE	WINAPI load_driver( 
	char *driver_name,		/* driver name, example "xfmap" */
	char *driver_bin		/* driver binary, example "xfmap.sys" */
	);

BOOL	WINAPI unload_driver( 
	HANDLE h_driver,		/* handle to loaded driver */
	char *driver_name		/* driver name, example : "xfmap" */
	);

BOOL UnloadDeviceDriver( const char * Name );
