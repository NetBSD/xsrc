/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sunos/sun_mouse.c,v 1.12 2005/02/06 23:48:15 dawes Exp $ */
/*
 * Copyright 1999-2005 The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "xf86.h"
#include "xf86_OSlib.h"
#include "xf86OSmouse.h"

#if defined(__SOL8__) || !defined(i386)
#define HAVE_VUID_SUPPORT
#endif

#define DEFAULT_MOUSE_PS2_DEV		"/dev/kdmouse"
#define DEFAULT_MOUSE_VUID_DEV		"/dev/mouse"

#define PS2_PROTOCOL_NAME		"PS/2"
#define VUID_PROTOCOL_NAME		"VUID"

#ifdef HAVE_VUID_SUPPORT
#include "xisb.h"
#include "mipointer.h"
#include <sys/vuid_event.h>


/* Names of protocols that are handled internally here. */

static const char *internalNames[] = {
	VUID_PROTOCOL_NAME,
	NULL
};

typedef struct _VuidMseRec {
    Firm_event event;
    unsigned char *buffer;
} VuidMseRec, *VuidMsePtr;


static int  vuidMouseProc(DeviceIntPtr pPointer, int what);
static void vuidReadInput(InputInfoPtr pInfo);

/* This function is called when the protocol is "VUID". */
static Bool
vuidPreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
    MouseDevPtr pMse = pInfo->private;
    VuidMsePtr pVuidMse;

    pVuidMse = xalloc(sizeof(VuidMseRec));
    if (pVuidMse == NULL) {
	xf86Msg(X_ERROR, "%s: cannot allocate VuidMouseRec\n", pInfo->name);
	xfree(pMse);
	pInfo->private = NULL;
	return FALSE;
    }

    pMse->protocol = protocol;
    xf86Msg(X_CONFIG, "%s: Protocol: %s\n", pInfo->name, protocol);

    /* Collect the options, and process the common options. */
    xf86CollectInputOptions(pInfo, NULL, NULL);
    xf86ProcessCommonOptions(pInfo, pInfo->options);

    /* Check if the device can be opened. */
    pInfo->fd = xf86OpenSerial(pInfo->options);
    if (pInfo->fd == -1) {
	if (xf86GetAllowMouseOpenFail())
	    xf86Msg(X_WARNING, "%s: cannot open input device\n", pInfo->name);
	else {
	    xf86Msg(X_ERROR, "%s: cannot open input device\n", pInfo->name);
	    xfree(pVuidMse);
	    xfree(pMse);
	    pInfo->private = NULL;
	    return FALSE;
	}
    }
    pVuidMse->buffer = (unsigned char *)&pVuidMse->event;
    xf86CloseSerial(pInfo->fd);
    pInfo->fd = -1;

    /* Private structure */
    pMse->mousePriv = pVuidMse;

    /* Process common mouse options (like Emulate3Buttons, etc). */
    pMse->CommonOptions(pInfo);

    /* Setup the local procs. */
    pInfo->device_control = vuidMouseProc;
    pInfo->read_input = vuidReadInput;

    pInfo->flags |= XI86_CONFIGURED;
    return TRUE;
}

static void
vuidReadInput(InputInfoPtr pInfo)
{
    MouseDevPtr pMse;
    VuidMsePtr pVuidMse;
    int buttons;
    int dx = 0, dy = 0, dz = 0, dw = 0;
    unsigned int n;
    int c; 
    unsigned char *pBuf;

    pMse = pInfo->private;
    pVuidMse = pMse->mousePriv;
    buttons = pMse->lastButtons;
    XisbBlockDuration(pMse->buffer, -1);
    pBuf = pVuidMse->buffer;
    n = 0;

    do {
	while (n < sizeof(Firm_event) && (c = XisbRead(pMse->buffer)) >= 0) {
	    pBuf[n++] = (unsigned char)c;
	}

	if (n == 0)
	    return;

	if (n != sizeof(Firm_event)) {
	    xf86Msg(X_WARNING, "%s: incomplete packet, size %d\n",
			pInfo->name, n);
	}

	if (pVuidMse->event.id >= BUT_FIRST && pVuidMse->event.id <= BUT_LAST) {
	    /* button */
	    int butnum = pVuidMse->event.id - BUT_FIRST;
	    if (butnum < 3)
		butnum = 2 - butnum;
	    if (!pVuidMse->event.value)
		buttons &= ~(1 << butnum);
	    else
		buttons |= (1 << butnum);
	} else if (pVuidMse->event.id >= VLOC_FIRST &&
		   pVuidMse->event.id <= VLOC_LAST) {
	    /* axis */
	    int delta = pVuidMse->event.value;
	    switch(pVuidMse->event.id) {
	    case LOC_X_DELTA:
		dx += delta;
		break;
	    case LOC_Y_DELTA:
		dy -= delta;
		break;
	    }
	}

	n = 0;
	if ((c = XisbRead(pMse->buffer)) >= 0) {
	    /* Another packet.  Handle it right away. */
	    pBuf[n++] = c;
	}
    } while (n != 0);

    pMse->PostEvent(pInfo, buttons, dx, dy, dz, dw);
    return;
}

#define NUMEVENTS 64

static int
vuidMouseProc(DeviceIntPtr pPointer, int what)
{
    InputInfoPtr pInfo;
    MouseDevPtr pMse;
    VuidMsePtr pVuidMse;
    unsigned char map[MSE_MAXBUTTONS + 1];
    int nbuttons;

    pInfo = pPointer->public.devicePrivate;
    pMse = pInfo->private;
    pMse->device = pPointer;
    pVuidMse = pMse->mousePriv;

    switch (what) {
    case DEVICE_INIT: 
	pPointer->public.on = FALSE;

	for (nbuttons = 0; nbuttons < MSE_MAXBUTTONS; ++nbuttons)
	    map[nbuttons + 1] = nbuttons + 1;

	InitPointerDeviceStruct((DevicePtr)pPointer, 
				map, 
				min(pMse->buttons, MSE_MAXBUTTONS),
				miPointerGetMotionEvents, 
				pMse->Ctrl,
				miPointerGetMotionBufferSize());

	/* X valuator */
	xf86InitValuatorAxisStruct(pPointer, 0, 0, -1, 1, 0, 1);
	xf86InitValuatorDefaults(pPointer, 0);
	/* Y valuator */
	xf86InitValuatorAxisStruct(pPointer, 1, 0, -1, 1, 0, 1);
	xf86InitValuatorDefaults(pPointer, 1);
	xf86MotionHistoryAllocate(pInfo);
	break;

    case DEVICE_ON:
	pInfo->fd = xf86OpenSerial(pInfo->options);
	if (pInfo->fd == -1)
	    xf86Msg(X_WARNING, "%s: cannot open input device\n", pInfo->name);
	else {
	    pMse->buffer = XisbNew(pInfo->fd,
			      NUMEVENTS * sizeof(Firm_event));
	    if (!pMse->buffer) {
		xfree(pMse);
		pInfo->private = NULL;
		xf86CloseSerial(pInfo->fd);
		pInfo->fd = -1;
	    } else {
	        int fmt = VUID_FIRM_EVENT;
		ioctl(pInfo->fd, VUIDSFORMAT, &fmt);
		xf86FlushInput(pInfo->fd);
		AddEnabledDevice(pInfo->fd);
	    }
	}
	pMse->lastButtons = 0;
	pMse->emulateState = 0;
	pPointer->public.on = TRUE;
	break;

    case DEVICE_OFF:
    case DEVICE_CLOSE:
	if (pInfo->fd != -1) {
	    RemoveEnabledDevice(pInfo->fd);
	    if (pMse->buffer) {
		XisbFree(pMse->buffer);
		pMse->buffer = NULL;
	    }
	    xf86CloseSerial(pInfo->fd);
	    pInfo->fd = -1;
	}
	pPointer->public.on = FALSE;
	usleep(300000);
	break;
    }
    return Success;
}

static Bool
sunMousePreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
    /* The protocol is guaranteed to be one of the internalNames[] */
    if (xf86NameCmp(protocol, VUID_PROTOCOL_NAME) == 0) {
	return vuidPreInit(pInfo, protocol, flags);
    }
    return TRUE;
}    

static const char **
BuiltinNames(void)
{
    return internalNames;
}

static Bool
CheckProtocol(const char *protocol)
{
    int i;

    for (i = 0; internalNames[i]; i++)
	if (xf86NameCmp(protocol, internalNames[i]) == 0)
	    return TRUE;

    return FALSE;
}
#endif

static const char *
DefaultProtocol(void)
{
    return "Auto";
}

static Bool
haveVUID(int fd, const char *dev)
{
#ifndef HAVE_VUID_SUPPORT
    return FALSE;
#else
    Bool needToClose = FALSE;
    int ret, fmt;

    if (fd == -1) {
	needToClose = TRUE;
	if (!dev)
	    dev = DEFAULT_MOUSE_VUID_DEV;
	SYSCALL (fd = open(dev, O_RDWR | O_NONBLOCK));
    }
    if (fd == -1) {
#ifdef DEBUG
	ErrorF("Cannot open %s (%s)\n", dev, strerror(errno));
#endif
	return FALSE;
    }

    ret = ioctl(fd, VUIDGFORMAT, &fmt);
    if (needToClose)
	close(fd);

    if (ret == -1) {
#ifdef DEBUG
	ErrorF("VUIDGFORMAT ioctl failed (%s)\n", strerror(errno));
#endif
	return FALSE;
    } else
	return TRUE;
#endif
}

static const char *
GuessProtocol(InputInfoPtr pInfo, int flags)
{
    const char *dev;
    char *realdev = NULL;
    struct stat sbuf;
    const char *ret = NULL;
    int i;

    dev = xf86SetStrOption(pInfo->conf_idev->commonOptions, "Device", NULL);
    if (!dev) {
#ifdef DEBUG
	ErrorF("xf86SetStrOption failed to return the device name\n");
#endif
	return NULL;
    }
    /* Can guess either VUID or PS/2. */
    if (haveVUID(-1, dev))
	return VUID_PROTOCOL_NAME;

    if (strcmp(dev, DEFAULT_MOUSE_PS2_DEV) == 0)
	return PS2_PROTOCOL_NAME;

    if (lstat(dev, &sbuf) != 0) {
#ifdef DEBUG
	ErrorF("lstat failed for %s (%s)\n", dev, strerror(errno));
#endif
	return NULL;
    }
    if (S_ISLNK(sbuf.st_mode)) {
	realdev = xalloc(PATH_MAX + 1);
	if (!realdev)
	    return NULL;
	i = readlink(dev, realdev, PATH_MAX);
	if (i <= 0) {
#ifdef DEBUG
	    ErrorF("readlink failed for %s (%s)\n", dev, strerror(errno));
#endif
	    xfree(realdev);
	    return NULL;
	}
	realdev[i] = '\0';
	/* If realdev doesn't contain a '/' then prepend "/dev/". */
	if (!strchr(realdev, '/')) {
	    char *tmp;
	    xasprintf(&tmp, "/dev/%s", realdev);
	    if (tmp) {
		xfree(realdev);
		realdev = tmp;
	    }
	}
    } else {
	realdev = xstrdup(dev);
	if (!realdev)
	    return NULL;
    }

    if (strcmp(realdev, DEFAULT_MOUSE_PS2_DEV) == 0)
	ret = PS2_PROTOCOL_NAME;
    xfree(realdev);

    return ret;
}

static const char *
SetupAuto(InputInfoPtr pInfo, int *protoPara)
{
    const char *guess;

    if (pInfo->fd == -1) {
	guess = GuessProtocol(pInfo, 0);
	if (guess)
	    return guess;
    }

    if (haveVUID(pInfo->fd, NULL))
	return VUID_PROTOCOL_NAME;
    else
	return PS2_PROTOCOL_NAME;
}

static const char *
FindDevice(InputInfoPtr pInfo, const char *protocol, int flags)
{
    const char *dev;

    if (haveVUID(-1, NULL))
	dev = DEFAULT_MOUSE_VUID_DEV;
    else
	dev = DEFAULT_MOUSE_PS2_DEV;

    pInfo->conf_idev->commonOptions =
	xf86AddNewOption(pInfo->conf_idev->commonOptions, "Device", dev);
    xf86Msg(X_INFO, "%s: Setting Device option to \"%s\".\n", pInfo->name, dev);
    return dev;
}

static int
SupportedInterfaces(void)
{
    int i;

    /* XXX This needs to be checked. */
    i = MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_AUTO | MSE_XPS2;
#ifdef HAVE_VUID_SUPPORT
    i |= MSE_MISC;
#endif
    return i;
}

OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
    OSMouseInfoPtr p;

    p = xcalloc(sizeof(OSMouseInfoRec), 1);
    if (!p)
	return NULL;
    p->SupportedInterfaces = SupportedInterfaces;
#ifdef HAVE_VUID_SUPPORT
    p->BuiltinNames = BuiltinNames;
    p->CheckProtocol = CheckProtocol;
    p->PreInit = sunMousePreInit;
#endif
    p->DefaultProtocol = DefaultProtocol;
    p->SetupAuto = SetupAuto;
    p->FindDevice = FindDevice;
    p->GuessProtocol = GuessProtocol;
    return p;
}

