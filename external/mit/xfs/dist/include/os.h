/*
Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef	_OS_H_
#define	_OS_H_

typedef struct _FontPathRec *FontPathPtr;
typedef struct _alt_server *AlternateServerPtr;
typedef struct _auth *AuthPtr;

#include <X11/fonts/FSproto.h>
#include <X11/Xfuncproto.h>
#include "client.h"
#include "misc.h"

#if __has_attribute(alloc_size)
#define XFS_ATTRIBUTE_ALLOC_SIZE(ARGS) __attribute__ ((alloc_size ARGS))
#else
#define XFS_ATTRIBUTE_ALLOC_SIZE(ARGS)
#endif

#if __has_attribute(malloc)
# if defined(__clang__) || (defined(__GNUC__) && __GNUC__ < 11)
/* Clang or older gcc do not support the optional deallocator argument */
#  define XFS_ATTRIBUTE_MALLOC(ARGS) __attribute__((malloc))
# else
#  define XFS_ATTRIBUTE_MALLOC(ARGS) __attribute__((malloc ARGS))
# endif
#else
# define XFS_ATTRIBUTE_MALLOC(ARGS)
#endif

#define XFS_ALLOCATOR(DEALLOC, SIZE) \
    XFS_ATTRIBUTE_MALLOC(DEALLOC) XFS_ATTRIBUTE_ALLOC_SIZE(SIZE)

typedef pointer FID;

#define ALLOCATE_LOCAL_FALLBACK(_size) FSalloc((unsigned long)_size)
#define DEALLOCATE_LOCAL_FALLBACK(_ptr) FSfree((pointer)_ptr)

#include "X11/Xalloca.h"

#define	MAX_REQUEST_SIZE	8192

extern int  ListenPort;
extern Bool UseSyslog;
extern Bool CloneSelf;
extern char ErrorFile[];

struct _osComm;	/* FIXME: osCommPtr */

/* os/config.c */
extern	int	ReadConfigFile(const char *filename);

/* os/connection.c */
extern	void	AttendClient(ClientPtr client);
extern	void	CheckConnections(void);
extern	void	CloseDownConnection(ClientPtr client);
extern	void	IgnoreClient(ClientPtr client);
extern	void	MakeNewConnections(void);
extern	void	ReapAnyOldClients(void);
extern	void	ResetSockets(void);
extern	void	CloseSockets(void);
extern	void	StopListening(void);

/* os/daemon.c */
extern	void	BecomeDaemon(void);
extern	void	DetachStdio(void);

/* os/error.c */
extern void	Error(const char *str);
extern void	InitErrors(void);
extern void	CloseErrors(void);
extern void	NoticeF(const char *f, ...) _X_ATTRIBUTE_PRINTF(1, 2);
extern void	ErrorF(const char * f, ...) _X_ATTRIBUTE_PRINTF(1, 2);
extern void	FatalError(const char* f, ...) _X_ATTRIBUTE_PRINTF(1, 2)
    _X_NORETURN;

/* os/io.c */
extern	Bool	InsertFakeRequest(ClientPtr client, char *data, int count);
extern	int	FlushClient(ClientPtr client, struct _osComm *oc, char *extraBuf, int extraCount, int padsize);
extern	int	ReadRequest(ClientPtr client);
extern	void	FlushAllOutput(void);
extern	void	FreeOsBuffers(struct _osComm *oc);
extern	void	ResetCurrentRequest(ClientPtr client);
extern	void	ResetOsBuffers(void);
extern	void	WriteToClient(ClientPtr client, int count, char *buf);
extern	void	WriteToClientUnpadded(ClientPtr client, int count, char *buf);

/* os/osglue.c */
extern int 	ListCatalogues(const char *pattern, int patlen, int maxnames, char **catalogues, int *len);
extern int 	ValidateCatalogues(int *num, char *cats);
extern int 	SetAlternateServers(char *list);
extern int 	ListAlternateServers(AlternateServerPtr *svrs);
extern int 	CloneMyself(void);

/* os/osinit.c */
extern	void	OsInit(void);

/* os/utils.c */
extern	void	AutoResetServer (int n);
extern	void	CleanupChild (int n);
extern	void	GiveUp (int n);
extern	void	ServerCacheFlush (int n);
extern	void	ServerReconfig (int n);
extern	unsigned int GetTimeInMillis (void);
extern	void	FSfree(pointer);
extern	pointer	FSalloc(unsigned long)
    XFS_ATTRIBUTE_ALLOC_SIZE((1)) XFS_ATTRIBUTE_MALLOC((FSfree));
extern	pointer	FSallocarray(unsigned long, unsigned long)
    XFS_ATTRIBUTE_ALLOC_SIZE((1,2)) XFS_ATTRIBUTE_MALLOC((FSfree));
extern	pointer	FScalloc (unsigned long, unsigned long)
     XFS_ATTRIBUTE_ALLOC_SIZE((1,2)) XFS_ATTRIBUTE_MALLOC((FSfree));
extern	pointer	FSrealloc(pointer, unsigned long)
    XFS_ATTRIBUTE_ALLOC_SIZE((2));
extern	pointer FSreallocarray (pointer, unsigned long, unsigned long)
    XFS_ATTRIBUTE_ALLOC_SIZE((2,3));
extern	void	ProcessCmdLine (int argc, char **argv);
extern	void	ProcessLSoption (char *str);
extern	void	SetUserId(void);
extern	void	SetDaemonState(void);

/* os/waitfor.c */
extern	int	WaitForSomething(int *pClientsReady);

extern void	SetConfigValues(void);


#endif				/* _OS_H_ */
