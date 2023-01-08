/* $XFree86: xc/lib/Xxf86dga/XF86DGA.c,v 3.23tsi Exp $ */
/*

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995,1996  The XFree86 Project, Inc

*/

/* THIS IS NOT AN X CONSORTIUM STANDARD */


#if defined(linux)
#define HAS_MMAP_ANON
#include <sys/types.h>
#include <sys/mman.h>
/* kernel header doesn't work with -ansi */
/* #include <asm/page.h> */   /* PAGE_SIZE */
#define HAS_SC_PAGESIZE /* _SC_PAGESIZE may be an enum for Linux */
#define HAS_GETPAGESIZE
#endif /* linux */

#if defined(CSRG_BASED)
#define HAS_MMAP_ANON
#define HAS_GETPAGESIZE
#include <sys/types.h>
#include <sys/mman.h>
#endif /* CSRG_BASED */

#if defined(SVR4)
#define MMAP_DEV_ZERO
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#endif /* SVR4 */

#ifdef XNO_SYSCONF
#undef _SC_PAGESIZE
#endif

#include <X11/Xlibint.h>
#include <X11/extensions/Xxf86dga.h>
#include <X11/extensions/xf86dgaproto.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>

extern XExtDisplayInfo* xdga_find_display(Display*);
extern const char *xdga_extension_name;

#define XF86DGACheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, xdga_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *		    public XFree86-DGA Extension routines                    *
 *                                                                           *
 *****************************************************************************/

Bool XF86DGAQueryExtension (
    Display *dpy,
    int *event_basep,
    int *error_basep
){
    return XDGAQueryExtension(dpy, event_basep, error_basep);
}

Bool XF86DGAQueryVersion(
    Display* dpy,
    int* majorVersion,
    int* minorVersion
){
    return XDGAQueryVersion(dpy, majorVersion, minorVersion);
}

Bool XF86DGAGetVideoLL(
    Display* dpy,
    int screen,
    unsigned int *offset,
    int *width,
    int *bank_size,
    int *ram_size
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAGetVideoLLReply rep;
    xXF86DGAGetVideoLLReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAGetVideoLL, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAGetVideoLL;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    *offset = rep.offset;
    *width = rep.width;
    *bank_size = rep.bank_size;
    *ram_size = rep.ram_size;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


Bool XF86DGADirectVideoLL(
    Display* dpy,
    int screen,
    int enable
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGADirectVideoReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGADirectVideo, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGADirectVideo;
    req->screen = screen;
    req->enable = enable;
    UnlockDisplay(dpy);
    SyncHandle();
    XSync(dpy,False);
    return True;
}

Bool XF86DGAGetViewPortSize(
    Display* dpy,
    int screen,
    int *width,
    int *height
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAGetViewPortSizeReply rep;
    xXF86DGAGetViewPortSizeReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAGetViewPortSize, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAGetViewPortSize;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    *width = rep.width;
    *height = rep.height;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


Bool XF86DGASetViewPort(
    Display* dpy,
    int screen,
    int x,
    int y
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGASetViewPortReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGASetViewPort, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGASetViewPort;
    req->screen = screen;
    req->x = x;
    req->y = y;
    UnlockDisplay(dpy);
    SyncHandle();
    XSync(dpy,False);
    return True;
}


Bool XF86DGAGetVidPage(
    Display* dpy,
    int screen,
    int *vpage
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAGetVidPageReply rep;
    xXF86DGAGetVidPageReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAGetVidPage, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAGetVidPage;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    *vpage = rep.vpage;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


Bool XF86DGASetVidPage(
    Display* dpy,
    int screen,
    int vpage
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGASetVidPageReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGASetVidPage, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGASetVidPage;
    req->screen = screen;
    req->vpage = vpage;
    UnlockDisplay(dpy);
    SyncHandle();
    XSync(dpy,False);
    return True;
}

Bool XF86DGAInstallColormap(
    Display* dpy,
    int screen,
    Colormap cmap
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAInstallColormapReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAInstallColormap, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAInstallColormap;
    req->screen = screen;
    req->id = cmap;
    UnlockDisplay(dpy);
    SyncHandle();
    XSync(dpy,False);
    return True;
}

Bool XF86DGAQueryDirectVideo(
    Display *dpy,
    int screen,
    int *flags
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAQueryDirectVideoReply rep;
    xXF86DGAQueryDirectVideoReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAQueryDirectVideo, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAQueryDirectVideo;
    req->screen = screen;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *flags = rep.flags;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XF86DGAViewPortChanged(
    Display *dpy,
    int screen,
    int n
){
    XExtDisplayInfo *info = xdga_find_display (dpy);
    xXF86DGAViewPortChangedReply rep;
    xXF86DGAViewPortChangedReq *req;

    XF86DGACheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(XF86DGAViewPortChanged, req);
    req->reqType = info->codes->major_opcode;
    req->dgaReqType = X_XF86DGAViewPortChanged;
    req->screen = screen;
    req->n = n;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return rep.result;
}



/* Helper functions */

#include <X11/Xmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#if defined(SVR4) && !defined(sun)
#define DEV_MEM "/dev/pmem"
#elif defined(SVR4) && defined(sun)
#define DEV_MEM "/dev/xsvc"
#elif defined(HAS_APERTURE_DRV)
#define DEV_MEM "/dev/xf86"
#else
#define DEV_MEM "/dev/mem"
#endif

typedef struct {
    unsigned long physaddr;	/* actual requested physical address */
    unsigned long size;		/* actual requested map size */
    unsigned long delta;	/* delta to account for page alignment */
    void *	  vaddr;	/* mapped address, without the delta */
    int		  refcount;	/* reference count */
} MapRec, *MapPtr;

typedef struct {
    Display *	display;
    int		screen;
    MapPtr	map;
} ScrRec, *ScrPtr;

static int mapFd = -1;
static int numMaps = 0;
static int numScrs = 0;
static MapPtr *mapList = NULL;
static ScrPtr *scrList = NULL;

static MapPtr
AddMap(void)
{
    MapPtr *old;

    old = mapList;
    mapList = realloc(mapList, sizeof(MapPtr) * (numMaps + 1));
    if (!mapList) {
	mapList = old;
	return NULL;
    }
    mapList[numMaps] = malloc(sizeof(MapRec));
    if (!mapList[numMaps])
	return NULL;
    return mapList[numMaps++];
}

static ScrPtr
AddScr(void)
{
    ScrPtr *old;

    old = scrList;
    scrList = realloc(scrList, sizeof(ScrPtr) * (numScrs + 1));
    if (!scrList) {
	scrList = old;
	return NULL;
    }
    scrList[numScrs] = malloc(sizeof(ScrRec));
    if (!scrList[numScrs])
	return NULL;
    return scrList[numScrs++];
}

static MapPtr
FindMap(unsigned long address, unsigned long size)
{
    int i;

    for (i = 0; i < numMaps; i++) {
	if (mapList[i]->physaddr == address &&
	    mapList[i]->size == size)
	    return mapList[i];
    }
    return NULL;
}

static ScrPtr
FindScr(Display *display, int screen)
{
    int i;

    for (i = 0; i < numScrs; i++) {
	if (scrList[i]->display == display &&
	    scrList[i]->screen == screen)
	    return scrList[i];
    }
    return NULL;
}

static void *
MapPhysAddress(unsigned long address, unsigned long size)
{
    unsigned long offset, delta;
    int pagesize = -1;
    void *vaddr;
    MapPtr mp;

    if ((mp = FindMap(address, size))) {
	mp->refcount++;
	return (void *)((unsigned long)mp->vaddr + mp->delta);
    }

#if defined(_SC_PAGESIZE) && defined(HAS_SC_PAGESIZE)
    pagesize = sysconf(_SC_PAGESIZE);
#endif
#ifdef _SC_PAGE_SIZE
    if (pagesize == -1)
	pagesize = sysconf(_SC_PAGE_SIZE);
#endif
#ifdef HAS_GETPAGESIZE
    if (pagesize == -1)
	pagesize = getpagesize();
#endif
#ifdef PAGE_SIZE
    if (pagesize == -1)
	pagesize = PAGE_SIZE;
#endif
    if (pagesize == -1)
	pagesize = 4096;

   delta = address % pagesize;
   offset = address - delta;
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
    if (mapFd < 0) {
	if ((mapFd = open(DEV_MEM, O_RDWR)) < 0)
	    return NULL;
    }
    vaddr = (void *)mmap(NULL, size + delta, PROT_READ | PROT_WRITE,
			 MAP_FILE | MAP_SHARED, mapFd, (off_t)offset);
    if (vaddr == (void *)-1)
	return NULL;

    if (!vaddr) {
	if (!(mp = AddMap()))
	    return NULL;
	mp->physaddr = address;
	mp->size = size;
	mp->delta = delta;
	mp->vaddr = vaddr;
	mp->refcount = 1;
    }
    return (void *)((unsigned long)vaddr + delta);
}

/*
 * Still need to find a clean way of detecting the death of a DGA app
 * and returning things to normal - Jon
 * This is here to help debugging without rebooting... Also C-A-BS
 * should restore text mode.
 */

int
XF86DGAForkApp(int screen)
{
    pid_t pid;
    int status;
    int i;

     /* fork the app, parent hangs around to clean up */
    if ((pid = fork()) > 0) {
	ScrPtr sp;

	waitpid(pid, &status, 0);
	for (i = 0; i < numScrs; i++) {
	    sp = scrList[i];
	    XF86DGADirectVideoLL(sp->display, sp->screen, 0);
	    XSync(sp->display, False);
	}
        if (WIFEXITED(status))
	    _exit(0);
	else
	    _exit(-1);
    }
    return pid;
}


Bool
XF86DGADirectVideo(
    Display *dis,
    int screen,
    int enable
){
    ScrPtr sp;
    MapPtr mp = NULL;

    if ((sp = FindScr(dis, screen)))
	mp = sp->map;

    if (enable & XF86DGADirectGraphics) {
	if (mp && mp->vaddr)
	    mprotect(mp->vaddr, mp->size + mp->delta, PROT_READ | PROT_WRITE);
    } else {
	if (mp && mp->vaddr)
	    mprotect(mp->vaddr, mp->size + mp->delta, PROT_READ);
    }

    XF86DGADirectVideoLL(dis, screen, enable);
    return 1;
}


static void
XF86cleanup(int sig)
{
    ScrPtr sp;
    int i;
    static char beenhere = 0;

    if (beenhere)
	_exit(3);
    beenhere = 1;

    for (i = 0; i < numScrs; i++) {
	sp = scrList[i];
	XF86DGADirectVideo(sp->display, sp->screen, 0);
	XSync(sp->display, False);
    }
    _exit(3);
}

Bool
XF86DGAGetVideo(
    Display *dis,
    int screen,
    char **addr,
    int *width,
    int *bank,
    int *ram
){
    unsigned int offset;
    static int beenHere = 0;
    ScrPtr sp;
    MapPtr mp;

    if (!(sp = FindScr(dis, screen))) {
	if (!(sp = AddScr())) {
	    fprintf(stderr, "XF86DGAGetVideo: malloc failure\n");
	    exit(-2);
	}
	sp->display = dis;
	sp->screen = screen;
	sp->map = NULL;
    }

    XF86DGAGetVideoLL(dis, screen , &offset, width, bank, ram);

    *addr = MapPhysAddress(offset, *bank);
    if (*addr == NULL) {
	fprintf(stderr, "XF86DGAGetVideo: failed to map video memory (%s)\n",
		strerror(errno));
	exit(-2);
    }

    if ((mp = FindMap(offset, *bank)))
	sp->map = mp;

    if (!beenHere) {
	beenHere = 1;
	atexit((void(*)(void))XF86cleanup);
	/* one shot XF86cleanup attempts */
	signal(SIGSEGV, XF86cleanup);
#ifdef SIGBUS
	signal(SIGBUS, XF86cleanup);
#endif
	signal(SIGHUP, XF86cleanup);
	signal(SIGFPE, XF86cleanup);
    }

    return 1;
}
