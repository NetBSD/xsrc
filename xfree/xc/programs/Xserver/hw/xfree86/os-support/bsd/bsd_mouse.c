/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_mouse.c,v 1.37 2005/02/10 01:37:52 dawes Exp $ */

/*
 * Copyright (c) 1999-2005 by The XFree86 Project, Inc.
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

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"
#include "xisb.h"
#include "mipointer.h"
#include <dirent.h>
#ifdef WSCONS_SUPPORT
#include <dev/wscons/wsconsio.h>
#endif
#ifdef USBMOUSE_SUPPORT
#ifdef HAS_LIB_USB_HID
#include <usbhid.h>
#else
#include "usb.h"
#endif

#include <dev/usb/usb.h>
#ifdef USB_GET_REPORT_ID
#define USB_NEW_HID
#endif

#define HUP_GENERIC_DESKTOP     0x0001
#define HUP_BUTTON              0x0009
#define HUP_CONSUMER		0x000c

#define HUG_X                   0x0030
#define HUG_Y                   0x0031
#define HUG_Z                   0x0032
#define HUG_WHEEL               0x0038

#define HUC_AC_PAN		0x0238

#define HID_USAGE2(p,u) (((p) << 16) | u)

/* The UMS mices have middle button as number 3 */
#define UMS_BUT(i) ((i) == 0 ? 2 : (i) == 1 ? 0 : (i) == 2 ? 1 : (i))
#endif /* USBMOUSE_SUPPORT */

#ifdef USBMOUSE_SUPPORT
static void usbSigioReadInput (int fd, void *closure);
#endif

#define TSTOMILLI(ts)       (((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

#define DEFAULT_MOUSE_DEV		"/dev/mouse"
#if defined(__NetBSD__)
#define DEFAULT_PS2_DEV			"/dev/pms0"
#else
#define DEFAULT_PS2_DEV			"/dev/psm0"
#endif

#if defined(__FreeBSD__)
/* These are for FreeBSD */
#define DEFAULT_SYSMOUSE_DEV		"/dev/sysmouse"
#define DEFAULT_USBMOUSE_DEV		"/dev/ums0"

static const char *mouseDevs[] = {
	DEFAULT_MOUSE_DEV,
	DEFAULT_SYSMOUSE_DEV,
	DEFAULT_PS2_DEV,
	DEFAULT_USBMOUSE_DEV,
	NULL
};

#elif defined(__OpenBSD__) || defined(__NetBSD__)

#define DEFAULT_WSMOUSE_DEV		"/dev/wsmouse"
#define DEFAULT_WSMOUSE0_DEV		"/dev/wsmouse0"

static const char *mouseDevs[] = {
	DEFAULT_MOUSE_DEV,
	DEFAULT_WSMOUSE_DEV,
	DEFAULT_WSMOUSE0_DEV,
	DEFAULT_PS2_DEV,
	NULL
};

#ifdef WSCONS_SUPPORT
static const char *wsMouseDevs[] = {
	DEFAULT_WSMOUSE_DEV,
	DEFAULT_WSMOUSE0_DEV,
	NULL
};
#endif
#endif

static int
SupportedInterfaces(void)
{
#if defined(__NetBSD__)
    return MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_AUTO;
#elif defined(__FreeBSD__)
    return MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_AUTO | MSE_MISC;
#else
    return MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_XPS2 | MSE_AUTO;
#endif
}

/* Names of protocols that are handled internally here. */
static const char *internalNames[] = {
#if defined(WSCONS_SUPPORT)
	"WSMouse",
#endif
#if defined(USBMOUSE_SUPPORT)
	"usb",
#endif
	NULL
};

/*
 * Names of MSC_MISC protocols that the OS supports.  These are decoded by
 * main "mouse" driver.
 */
static const char *miscNames[] = {
#if defined(__FreeBSD__)
	"SysMouse",
#endif
	NULL
};

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
    for (i = 0; miscNames[i]; i++)
	if (xf86NameCmp(protocol, miscNames[i]) == 0)
	    return TRUE;
    return FALSE;
}

static const char *
DefaultProtocol(void)
{
    return "Auto";
}

#if defined(__FreeBSD__) && defined(MOUSE_PROTO_SYSMOUSE)
static struct {
	int dproto;
	const char *name;
} devproto[] = {
	{ MOUSE_PROTO_MS,		"Microsoft" },
	{ MOUSE_PROTO_MSC,		"MouseSystems" },
	{ MOUSE_PROTO_LOGI,		"Logitech" },
	{ MOUSE_PROTO_MM,		"MMSeries" },
	{ MOUSE_PROTO_LOGIMOUSEMAN,	"MouseMan" },
	{ MOUSE_PROTO_BUS,		"BusMouse" },
	{ MOUSE_PROTO_INPORT,		"BusMouse" },
	{ MOUSE_PROTO_PS2,		"PS/2" },
	{ MOUSE_PROTO_HITTAB,		"MMHitTab" },
	{ MOUSE_PROTO_GLIDEPOINT,	"GlidePoint" },
	{ MOUSE_PROTO_INTELLI,		"Intellimouse" },
	{ MOUSE_PROTO_THINK,		"ThinkingMouse" },
	{ MOUSE_PROTO_SYSMOUSE,		"SysMouse" }
};
	
static const char *
SetupAuto(InputInfoPtr pInfo, int *protoPara)
{
    int i;
    mousehw_t hw;
    mousemode_t mode;

    if (pInfo->fd == -1)
	return NULL;

    /* set the driver operation level, if applicable */
    i = 1;
    ioctl(pInfo->fd, MOUSE_SETLEVEL, &i);
    
    /* interrogate the driver and get some intelligence on the device. */
    hw.iftype = MOUSE_IF_UNKNOWN;
    hw.model = MOUSE_MODEL_GENERIC;
    ioctl(pInfo->fd, MOUSE_GETHWINFO, &hw);
    xf86MsgVerb(X_INFO, 3, "%s: SetupAuto: hw.iftype is %d, hw.model is %d\n",
		pInfo->name, hw.iftype, hw.model);
    if (ioctl(pInfo->fd, MOUSE_GETMODE, &mode) == 0) {
	for (i = 0; i < sizeof(devproto)/sizeof(devproto[0]); ++i) {
	    if (mode.protocol == devproto[i].dproto) {
		/* override some parameters */
		if (protoPara) {
		    protoPara[4] = mode.packetsize;
		    protoPara[0] = mode.syncmask[0];
		    protoPara[1] = mode.syncmask[1];
		}
		xf86MsgVerb(X_INFO, 3, "%s: SetupAuto: protocol is %s\n",
			    pInfo->name, devproto[i].name);
		return devproto[i].name;
	    }
	}
    }
    return NULL;
}

static void
SetSysMouseRes(InputInfoPtr pInfo, const char *protocol, int rate, int res)
{
    mousemode_t mode;
    MouseDevPtr pMse;

    pMse = pInfo->private;

    mode.rate = rate > 0 ? rate : -1;
    mode.resolution = res > 0 ? res : -1;
    mode.accelfactor = -1;
#if defined(__FreeBSD__)
    if (pMse->autoProbe ||
	(protocol && xf86NameCmp(protocol, "SysMouse") == 0)) {
	/*
	 * As the FreeBSD sysmouse driver defaults to protocol level 0
	 * everytime it is opened we enforce protocol level 1 again at
	 * this point.
	 */
	mode.level = 1;
    } else
	mode.level = -1;
#else
    mode.level = -1;
#endif
    ioctl(pInfo->fd, MOUSE_SETMODE, &mode);
}
#endif

#if defined(__FreeBSD__)

#define MOUSED_PID_DIR "/var/run"
#define MOUSED_PID_FILE_PREFIX "moused"

/*
 * Try to check if moused is running.  DEFAULT_SYSMOUSE_DEV is useless without
 * it.  There doesn't seem to be a better way of checking.
 */
static Bool
MousedRunning(void)
{
    FILE *f = NULL;
    unsigned int pid;
    DIR *d;
    struct dirent *dp;
    char *fname;

    d = opendir(MOUSED_PID_DIR);
    if (!d)
	return FALSE;

    while ((dp = readdir(d))) {
	if (strncmp(dp->d_name, MOUSED_PID_FILE_PREFIX,
		    strlen(MOUSED_PID_FILE_PREFIX)) == 0) {
	    xasprintf(&fname, "%s/%s", MOUSED_PID_DIR, dp->d_name);
	    if (!fname)
		continue;
	    f = fopen(fname, "r");
	    xfree(fname);
	    fname = NULL;
	    if (f) {
		if (fscanf(f, "%u", &pid) == 1 && pid > 0) {
		    if (kill(pid, 0) == 0) {
			fclose(f);
			closedir(d);
			return TRUE;
		    }
		}
		fclose(f);
	    }
	}
    }
    closedir(d);
    return FALSE;
}

static const char *
FindDevice(InputInfoPtr pInfo, const char *protocol, int flags)
{
    int fd = -1;
    const char **pdev, *dev = NULL;
    Bool devMouse = FALSE;
    struct stat devMouseStat;
    struct stat sb;

    for (pdev = mouseDevs; *pdev; pdev++) {
	SYSCALL (fd = open(*pdev, O_RDWR | O_NONBLOCK));
	if (fd == -1) {
#ifdef DEBUG
	    ErrorF("Cannot open %s (%s)\n", *pdev, strerror(errno));
#endif
	} else {
	    /*
	     * /dev/mouse is held until checks for matches with other devices
	     * are done.  This is so that when it points to /dev/sysmouse,
	     * the test for whether /dev/sysmouse is usable can be made.
	     */
	    if (!strcmp(*pdev, DEFAULT_MOUSE_DEV)) {
		if (fstat(fd, &devMouseStat) == 0)
		    devMouse = TRUE;
		close(fd);
		continue;
	    } else if (!strcmp(*pdev, DEFAULT_SYSMOUSE_DEV)) {
		/* Check if /dev/mouse is the same as /dev/sysmouse. */
		if (devMouse && fstat(fd, &sb) == 0 && 
		    devMouseStat.st_dev == sb.st_dev &&
		    devMouseStat.st_ino == sb.st_ino) {
		    /* If the same, use /dev/sysmouse. */
		    devMouse = FALSE;
		}
		close(fd);
		if (MousedRunning())
		    break;
		else {
#ifdef DEBUG
	    	    ErrorF("moused isn't running\n");
#endif
		}
	    } else {
		close(fd);
		break;
	    }
	}
    }

    if (*pdev)
	dev = *pdev;
    else if (devMouse)
	dev = DEFAULT_MOUSE_DEV;

    if (dev) {
	/* Set the Device option. */
	pInfo->conf_idev->commonOptions =
	    xf86AddNewOption(pInfo->conf_idev->commonOptions, "Device", dev);
	xf86Msg(X_INFO, "%s: Setting Device option to \"%s\"\n",
		pInfo->name, dev);
    }

    return *pdev;
}
#endif

#if defined(__OpenBSD__) || defined(__NetBSD__)

static const char *
haveWSCons(void)
{
#ifndef WSCONS_SUPPORT
    return NULL;
#else
    const char **pdev;
    int fd;

    for (pdev = wsMouseDevs; *pdev; pdev++) {
	SYSCALL(fd = open(*pdev, O_RDWR | O_NONBLOCK));
	if (fd != -1) {
	    close(fd);
	    return *pdev;
	}
    }
    return NULL;
#endif
}
#if defined(__NetBSD__)

static Bool
SetupMouse(InputInfoPtr pInfo)
{
#ifdef WSCONS_SUPPORT
#ifdef WSMOUSEIO_SETVERSION
	int version = WSMOUSE_EVENT_VERSION;
	if (ioctl(pInfo->fd, WSMOUSEIO_SETVERSION, &version) == -1) {
	    xf86Msg(X_WARNING, "%s: cannot set version\n", pInfo->name);
	    return FALSE;
	}
#endif
#endif
	return TRUE;
}

#endif

static const char *
GuessProtocol(InputInfoPtr pInfo, int flags)
{
    const char *dev;
    char *realdev = NULL;
    const char *ret = NULL;
    int i;
    struct stat sbuf;

    dev = xf86SetStrOption(pInfo->conf_idev->commonOptions, "Device", NULL);
    if (!dev) {
#ifdef DEBUG
	ErrorF("xf86SetStrOption failed to return the device name\n");
#endif
	return NULL;
    }
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

    if (strncmp(realdev, DEFAULT_WSMOUSE_DEV, strlen(DEFAULT_WSMOUSE_DEV) == 0))
	ret = "wsmouse";
    else if (strcmp(realdev, DEFAULT_PS2_DEV) == 0)
	ret = "ps/2";
    xfree(realdev);
    return ret;
}

/* Only support wsmouse configuration for now */
static const char *
SetupAuto(InputInfoPtr pInfo, int *protoPara)
{
    const char *guess = NULL;

    if (pInfo->fd == -1)
	guess = GuessProtocol(pInfo, 0);
    if (!guess) {
	if (haveWSCons())
	    guess = "wsmouse";
	else
	    guess = "ps/2";
    }

    xf86MsgVerb(X_INFO, 3, "%s: SetupAuto: protocol is %s\n",
		pInfo->name, guess);
    return guess;
}

static void
SetMouseRes(InputInfoPtr pInfo, const char *protocol, int rate, int res)
{

    xf86MsgVerb(X_INFO, 3, "%s: SetMouseRes: protocol %s rate %d res %d\n",
	    pInfo->name, protocol, rate, res);
}

static const char *
FindDevice(InputInfoPtr pInfo, const char *protocol, int flags)
{
    int fd = -1;
    const char **pdev;

    for (pdev = mouseDevs; *pdev; pdev++) {
	SYSCALL(fd = open(*pdev, O_RDWR | O_NONBLOCK));
	if (fd != -1) {
	    /* Set the Device option. */
	    pInfo->conf_idev->commonOptions =
		xf86AddNewOption(pInfo->conf_idev->commonOptions, 
				 "Device", *pdev);
	    xf86Msg(X_INFO, "%s: found Device \"%s\"\n",
		    pInfo->name, *pdev);
	    close(fd);
	    break;
	}
    }
    return *pdev;
}
#endif /* __OpenBSD__ || __NetBSD__ */

#ifdef WSCONS_SUPPORT
#define NUMEVENTS 64

static void
wsconsReadInput(InputInfoPtr pInfo)
{
    MouseDevPtr pMse;
    static struct wscons_event eventList[NUMEVENTS];
    int n, c, x, y; 
    struct wscons_event *event = eventList;
    unsigned char *pBuf;

    pMse = pInfo->private;

    XisbBlockDuration(pMse->buffer, -1);
    pBuf = (unsigned char *)eventList;
    n = 0;
    while ((c = XisbRead(pMse->buffer)) >= 0 && n < sizeof(eventList)) {
	pBuf[n++] = (unsigned char)c;
    }

    if (n == 0)
	return;

    n /= sizeof(struct wscons_event);
    while( n-- ) {
	int buttons = pMse->lastButtons;
	int dx = 0, dy = 0, dz = 0, dw = 0;
	switch (event->type) {
	case WSCONS_EVENT_MOUSE_UP:
#define BUTBIT (1 << (event->value <= 2 ? 2 - event->value : event->value))
	    buttons &= ~BUTBIT;
	    break;
	case WSCONS_EVENT_MOUSE_DOWN:
	    buttons |= BUTBIT;
	    break;
	case WSCONS_EVENT_MOUSE_DELTA_X:
	    dx = event->value;
	    break;
	case WSCONS_EVENT_MOUSE_DELTA_Y:
	    dy = -event->value;
	    break;
#ifdef WSCONS_EVENT_MOUSE_DELTA_Z
	case WSCONS_EVENT_MOUSE_DELTA_Z:
	    dz = event->value;
	    break;
#endif
#ifdef WSCONS_EVENT_MOUSE_DELTA_W
	case WSCONS_EVENT_MOUSE_DELTA_W:
	    dw = event->value;
	    break;
#endif
        case WSCONS_EVENT_MOUSE_ABSOLUTE_X:
            miPointerPosition (&x, &y);
            miPointerAbsoluteCursor (event->value, y, TSTOMILLI(event->time));
            ++event;
            continue;
                            
        case WSCONS_EVENT_MOUSE_ABSOLUTE_Y:
            miPointerPosition (&x, &y);
            miPointerAbsoluteCursor (x, event->value, TSTOMILLI(event->time));
            ++event;
            continue;
                            
        case WSCONS_EVENT_MOUSE_ABSOLUTE_Z:
            ++event;
            continue;

#ifdef WSCONS_EVENT_MOUSE_ABSOLUTE_W
	case WSCONS_EVENT_MOUSE_ABSOLUTE_W:
	    ++event;
	    continue;
#endif
            
	default:
	    xf86Msg(X_WARNING, "%s: bad wsmouse event type=%d\n", pInfo->name,
		    event->type);
	    ++event;
	    continue;
	}

	pMse->PostEvent(pInfo, buttons, dx, dy, dz, dw);
	++event;
    }
    return;
}


/* This function is called when the protocol is "wsmouse". */
static Bool
wsconsPreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
    MouseDevPtr pMse = pInfo->private;

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
	    xfree(pMse);
	    pInfo->private = NULL;
	    return FALSE;
	}
    }
    xf86CloseSerial(pInfo->fd);
    pInfo->fd = -1;

    /* Process common mouse options (like Emulate3Buttons, etc). */
    pMse->CommonOptions(pInfo);

    /* Setup the local input proc. */
    pInfo->read_input = wsconsReadInput;
    pMse->xisbscale = sizeof(struct wscons_event);

    pInfo->flags |= XI86_CONFIGURED;
    return TRUE;
}
#endif

#if defined(USBMOUSE_SUPPORT)

typedef struct _UsbMseRec {
    int packetSize;
    int iid;
    hid_item_t loc_x;		/* x locator item */
    hid_item_t loc_y;		/* y locator item */
    hid_item_t loc_z;		/* z (wheel) locator item */
    hid_item_t loc_w;		/* w (pan) locator item */
    hid_item_t loc_btn[MSE_MAXBUTTONS]; /* buttons locator items */
   unsigned char *buffer;
} UsbMseRec, *UsbMsePtr;

static int
usbMouseProc(DeviceIntPtr pPointer, int what)
{
    InputInfoPtr pInfo;
    MouseDevPtr pMse;
    UsbMsePtr pUsbMse;
    unsigned char map[MSE_MAXBUTTONS + 1];
    int nbuttons;

    pInfo = pPointer->public.devicePrivate;
    pMse = pInfo->private;
    pMse->device = pPointer;
    pUsbMse = pMse->mousePriv;

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
	    pMse->buffer = XisbNew(pInfo->fd, pUsbMse->packetSize);
	    if (!pMse->buffer) {
		xfree(pMse);
		pInfo->private = NULL;
		xf86CloseSerial(pInfo->fd);
		pInfo->fd = -1;
	    } else {
		xf86FlushInput(pInfo->fd);
		if (!xf86InstallSIGIOHandler (pInfo->fd, usbSigioReadInput, 
					      pInfo))
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
	    if (pUsbMse->packetSize > 8 && pUsbMse->buffer) {
		xfree(pUsbMse->buffer);
	    }
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

static void
usbReadInput(InputInfoPtr pInfo)
{
    MouseDevPtr pMse;
    UsbMsePtr pUsbMse;
    int buttons = pMse->lastButtons;
    int dx = 0, dy = 0, dz = 0, dw = 0;
    int n, c; 
    unsigned char *pBuf;

    pMse = pInfo->private;
    pUsbMse = pMse->mousePriv;

    XisbBlockDuration(pMse->buffer, -1);
    pBuf = pUsbMse->buffer;
    n = 0;
    while ((c = XisbRead(pMse->buffer)) >= 0 && n < pUsbMse->packetSize) {
	pBuf[n++] = (unsigned char)c;
    }
    if (n == 0)
	return;
    if (n != pUsbMse->packetSize) {
	xf86Msg(X_WARNING, "%s: incomplete packet, size %d\n", pInfo->name,
		n);
    }
    /* discard packets with an id that don't match the mouse */
    /* XXX this is probably not the right thing */
    if (pUsbMse->iid != 0) {
	if (*pBuf++ != pUsbMse->iid) 
	    return;
    }
    dx = hid_get_data(pBuf, &pUsbMse->loc_x);
    dy = hid_get_data(pBuf, &pUsbMse->loc_y);
    dz = hid_get_data(pBuf, &pUsbMse->loc_z);
    dw = hid_get_data(pBuf, &pUsbMse->loc_w);

    buttons = 0;
    for (n = 0; n < pMse->buttons; n++) {
	if (hid_get_data(pBuf, &pUsbMse->loc_btn[n])) 
	    buttons |= (1 << UMS_BUT(n));
    }
    pMse->PostEvent(pInfo, buttons, dx, dy, dz, dw);
    return;
}

static void
usbSigioReadInput (int fd, void *closure)
{
    usbReadInput ((InputInfoPtr) closure);
}

/* This function is called when the protocol is "usb". */
static Bool
usbPreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
    MouseDevPtr pMse = pInfo->private;
    UsbMsePtr pUsbMse;
    report_desc_t reportDesc;
    int i;

    pUsbMse = xalloc(sizeof(UsbMseRec));
    if (pUsbMse == NULL) {
	xf86Msg(X_ERROR, "%s: cannot allocate UsbMouseRec\n", pInfo->name);
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
	    xfree(pUsbMse);
	    xfree(pMse);
	    pInfo->private = NULL;
	    return FALSE;
	}
    }
    /* Get USB informations */
    reportDesc = hid_get_report_desc(pInfo->fd);
    /* Get packet size & iid */
#ifdef USB_NEW_HID
    if (ioctl(pInfo->fd, USB_GET_REPORT_ID, &pUsbMse->iid) == -1) {
	    xf86Msg(X_ERROR, "Error ioctl USB_GET_REPORT_ID on %s : %s\n",
		    pInfo->name, strerror(errno));
	    return FALSE;
    }
    pUsbMse->packetSize = hid_report_size(reportDesc, hid_input,
					      pUsbMse->iid);
#else
    pUsbMse->packetSize = hid_report_size(reportDesc, hid_input,
					      &pUsbMse->iid);
#endif
    /* Allocate buffer */
    if (pUsbMse->packetSize <= 8) {
	pUsbMse->buffer = pMse->protoBuf;
    } else {
	pUsbMse->buffer = xalloc(pUsbMse->packetSize);
    }
    if (pUsbMse->buffer == NULL) {
	xf86Msg(X_ERROR, "%s: cannot allocate buffer\n", pInfo->name);
	xfree(pUsbMse);
	xfree(pMse);
	pInfo->private = NULL;
	xf86CloseSerial(pInfo->fd);
	return FALSE;
    }
#ifdef USB_NEW_HID
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_X),
		   hid_input, &pUsbMse->loc_x, pUsbMse->iid) < 0) {
	xf86Msg(X_WARNING, "%s: no x locator\n", pInfo->name);
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_Y),
		   hid_input, &pUsbMse->loc_y, pUsbMse->iid) < 0) {
	xf86Msg(X_WARNING, "%s: no y locator\n", pInfo->name);
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_WHEEL),
		   hid_input, &pUsbMse->loc_z, pUsbMse->iid) < 0) {
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_CONSUMER, HUC_AC_PAN),
		   hid_input, &pUsbMse->loc_w, pUsbMse->iid) < 0) {
    }
#else
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_X),
		   hid_input, &pUsbMse->loc_x) < 0) {
	xf86Msg(X_WARNING, "%s: no x locator\n", pInfo->name);
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_Y),
		   hid_input, &pUsbMse->loc_y) < 0) {
	xf86Msg(X_WARNING, "%s: no y locator\n", pInfo->name);
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_GENERIC_DESKTOP, HUG_WHEEL),
		   hid_input, &pUsbMse->loc_z) < 0) {
    }
    if (hid_locate(reportDesc, HID_USAGE2(HUP_CONSUMER, HUC_AC_PAN),
		   hid_input, &pUsbMse->loc_w) < 0) {
    }
#endif
    /* Probe for number of buttons */
    for (i = 1; i <= MSE_MAXBUTTONS; i++) {
	if (!hid_locate(reportDesc, HID_USAGE2(HUP_BUTTON, i),
			hid_input, &pUsbMse->loc_btn[i-1]
#ifdef USB_NEW_HID
			, pUsbMse->iid
#endif
			))
	    break;
    }
    pMse->buttons = i-1;

    xf86CloseSerial(pInfo->fd);
    pInfo->fd = -1;

    /* Private structure */
    pMse->mousePriv = pUsbMse;

    /* Process common mouse options (like Emulate3Buttons, etc). */
    pMse->CommonOptions(pInfo);

    /* Setup the local procs. */
    pInfo->device_control = usbMouseProc;
    pInfo->read_input = usbReadInput;

    pInfo->flags |= XI86_CONFIGURED;
    return TRUE;
}
#endif /* USBMOUSE */

static Bool
bsdMousePreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
    /* The protocol is guaranteed to be one of the internalNames[] */
#ifdef WSCONS_SUPPORT
    if (xf86NameCmp(protocol, "WSMouse") == 0) {
	return wsconsPreInit(pInfo, protocol, flags);
    }
#endif
#ifdef USBMOUSE_SUPPORT
    if (xf86NameCmp(protocol, "usb") == 0) {
	return usbPreInit(pInfo, protocol, flags);
    }
#endif
    return TRUE;
}    

OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
    OSMouseInfoPtr p;

    p = xcalloc(sizeof(OSMouseInfoRec), 1);
    if (!p)
	return NULL;
    p->SupportedInterfaces = SupportedInterfaces;
    p->BuiltinNames = BuiltinNames;
    p->DefaultProtocol = DefaultProtocol;
    p->CheckProtocol = CheckProtocol;
#if defined(__FreeBSD__) && defined(MOUSE_PROTO_SYSMOUSE)
    p->SetupAuto = SetupAuto;
    p->SetPS2Res = SetSysMouseRes;
    p->SetBMRes = SetSysMouseRes;
    p->SetMiscRes = SetSysMouseRes;
#endif
#if defined(__OpenBSD__) || defined(__NetBSD__)
    p->SetupAuto = SetupAuto;
    p->SetMiscRes = SetMouseRes;
    p->GuessProtocol = GuessProtocol;
#endif
#if defined(__NetBSD__)
    p->SetupMouse = SetupMouse;
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    p->FindDevice = FindDevice;
#endif
    p->PreInit = bsdMousePreInit;
    return p;
}
