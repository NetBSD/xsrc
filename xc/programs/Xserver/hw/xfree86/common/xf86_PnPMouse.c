/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_PnPMouse.c,v 1.1.2.6 1999/07/29 09:22:51 hohndel Exp $ */

/*
 * Copyright 1998 by Kazutaka YOKOTA <yokota@zodiac.mech.utsunomiya-u.ac.jp>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kazutaka YOKOTA not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Kazutaka YOKOTA makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * KAZUTAKA YOKOTA DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KAZUTAKA YOKOTA BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

#ifdef ISC
#define TIOCMGET	0x5415
#define TIOCMBIS	0x5416
#define TIOCMSET	0x5418
#define	TIOCM_DTR	0x002
#define	TIOCM_RTS	0x004
#endif

#if defined(__QNX__) || defined(__QNXNTO__)
#define	TIOCM_DTR	0x002
#define	TIOCM_RTS	0x004
#endif

/* serial PnP ID string */
typedef struct {
    int revision;	/* PnP revision, 100 for 1.00 */
    char *eisaid;	/* EISA ID including mfr ID and product ID */
    char *serial;	/* serial No, optional */
    char *class;	/* device class, optional */
    char *compat;	/* list of compatible drivers, optional */
    char *description;	/* product description, optional */
    int neisaid;	/* length of the above fields... */
    int nserial;
    int nclass;
    int ncompat;
    int ndescription;
} pnpid_t;

/* symbol table entry */
typedef struct {
    char *name;
    int val;
} symtab_t;

/* PnP EISA/product IDs */
static symtab_t pnpprod[] = {
    { "KML0001",	P_THINKING },	/* Kensignton ThinkingMouse */
    { "MSH0001",	P_IMSERIAL },	/* MS IntelliMouse */
    { "MSH0004",	P_IMSERIAL },	/* MS IntelliMouse TrackBall */
    { "KYEEZ00",	P_MS },		/* Genius EZScroll */
    { "KYE0001",	P_MS },		/* Genius PnP Mouse */
    { "KYE0003",	P_IMSERIAL },	/* Genius NetMouse */
    { "LGI800C",	P_IMSERIAL },	/* Logitech MouseMan (4 button model) */
    { "LGI8050",	P_IMSERIAL },	/* Logitech MouseMan+ */
    { "LGI8051",	P_IMSERIAL },	/* Logitech FirstMouse+ */
    { "LGI8001",	P_LOGIMAN },	/* Logitech serial */

    { "PNP0F00",	P_BM },		/* MS bus */
    { "PNP0F01",	P_MS },		/* MS serial */
    { "PNP0F02",	P_BM },		/* MS InPort */
    { "PNP0F03",	P_PS2 },	/* MS PS/2 */
    /*
     * EzScroll returns PNP0F04 in the compatible device field; but it
     * doesn't look compatible... XXX
     */
    { "PNP0F04",	P_MSC },	/* MouseSystems */ 
    { "PNP0F05",	P_MSC },	/* MouseSystems */ 
#if notyet
    { "PNP0F06",	P_??? },	/* Genius Mouse */ 
    { "PNP0F07",	P_??? },	/* Genius Mouse */ 
#endif
    { "PNP0F08",	P_LOGIMAN },	/* Logitech serial */
    { "PNP0F09",	P_MS },		/* MS BallPoint serial */
    { "PNP0F0A",	P_MS },		/* MS PnP serial */
    { "PNP0F0B",	P_MS },		/* MS PnP BallPoint serial */
    { "PNP0F0C",	P_MS },		/* MS serial comatible */
    { "PNP0F0D",	P_BM },		/* MS InPort comatible */
    { "PNP0F0E",	P_PS2 },	/* MS PS/2 comatible */
    { "PNP0F0F",	P_MS },		/* MS BallPoint comatible */
#if notyet
    { "PNP0F10",	P_??? },	/* TI QuickPort */
#endif
    { "PNP0F11",	P_BM },		/* MS bus comatible */
    { "PNP0F12",	P_PS2 },	/* Logitech PS/2 */
    { "PNP0F13",	P_PS2 },	/* PS/2 */
#if notyet
    { "PNP0F14",	P_??? },	/* MS Kids Mouse */
#endif
    { "PNP0F15",	P_BM },		/* Logitech bus */ 
#if notyet
    { "PNP0F16",	P_??? },	/* Logitech SWIFT */
#endif
    { "PNP0F17",	P_LOGIMAN },	/* Logitech serial compat */
    { "PNP0F18",	P_BM },		/* Logitech bus compatible */
    { "PNP0F19",	P_PS2 },	/* Logitech PS/2 compatible */
#if notyet
    { "PNP0F1A",	P_??? },	/* Logitech SWIFT compatible */
    { "PNP0F1B",	P_??? },	/* HP Omnibook */
    { "PNP0F1C",	P_??? },	/* Compaq LTE TrackBall PS/2 */
    { "PNP0F1D",	P_??? },	/* Compaq LTE TrackBall serial */
    { "PNP0F1E",	P_??? },	/* MS Kidts Trackball */
#endif
    { NULL,		-1 },
};

static int
pnpgets(
#if NeedFunctionPrototypes
    MouseDevPtr,
    char *
#endif
);

static int
pnpparse(
#if NeedFunctionPrototypes
    pnpid_t *,
    char *,
    int
#endif
);

static symtab_t *
pnpproto(
#if NeedFunctionPrototypes
    pnpid_t *
#endif
);

static symtab_t *
gettoken(
#if NeedFunctionPrototypes
    symtab_t *,
    char *,
    int
#endif
);

int
xf86GetPnPMouseProtocol(mouse)
MouseDevPtr mouse;
{
    char buf[256];	/* PnP ID string may be up to 256 bytes long */
    pnpid_t pnpid;
    symtab_t *t;
    int len;

    if (((len = pnpgets(mouse, buf)) <= 0) || !pnpparse(&pnpid, buf, len))
	return -1;
    if ((t = pnpproto(&pnpid)) == NULL)
	return -1;
    ErrorF("Mouse: protocol: %d\n", t->val);
    return (t->val);
}

/*
 * Try to elicit a PnP ID as described in 
 * Microsoft, Hayes: "Plug and Play External COM Device Specification, 
 * rev 1.00", 1995.
 *
 * The routine does not fully implement the COM Enumerator as par Section
 * 2.1 of the document.  In particular, we don't have idle state in which
 * the driver software monitors the com port for dynamic connection or 
 * removal of a device at the port, because `moused' simply quits if no 
 * device is found.
 *
 * In addition, as PnP COM device enumeration procedure slightly has 
 * changed since its first publication, devices which follow earlier
 * revisions of the above spec. may fail to respond if the rev 1.0 
 * procedure is used. XXX
 */
static int
pnpgets(mouse, buf)
MouseDevPtr mouse;
char *buf;
{
    struct timeval timeout;
    fd_set fds;
    int i;
    char c;

#if 0
    /* 
     * This is the procedure described in rev 1.0 of PnP COM device spec.
     * Unfortunately, some devices which comform to earlier revisions of
     * the spec gets confused and do not return the ID string...
     */

    /* port initialization (2.1.2) */
    ioctl(mouse->mseFd, TIOCMGET, &i);
    i |= TIOCM_DTR;		/* DTR = 1 */
    i &= ~TIOCM_RTS;		/* RTS = 0 */
    ioctl(mouse->mseFd, TIOCMSET, &i);
    usleep(200000);
    if ((ioctl(mouse->mseFd, TIOCMGET, &i) == -1) || ((i & TIOCM_DSR) == 0))
	goto disconnect_idle;

    /* port setup, 1st phase (2.1.3) */
    xf86SetMouseSpeed(mouse, 1200, 1200, (CS7 | CREAD | CLOCAL | HUPCL));
    i = TIOCM_DTR | TIOCM_RTS;	/* DTR = 0, RTS = 0 */
    ioctl(mouse->mseFd, TIOCMBIC, &i);
    usleep(200000);
    i = TIOCM_DTR;		/* DTR = 1, RTS = 0 */
    ioctl(mouse->mseFd, TIOCMBIS, &i);
    usleep(200000);

    /* wait for response, 1st phase (2.1.4) */
    xf86FlushInput(mouse->mseFd);
    i = TIOCM_RTS;		/* DTR = 1, RTS = 1 */
    ioctl(mouse->mseFd, TIOCMBIS, &i);

    /* try to read something */
    FD_ZERO(&fds);
    FD_SET(mouse->mseFd, &fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    if (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) <= 0) {

	/* port setup, 2nd phase (2.1.5) */
        i = TIOCM_DTR | TIOCM_RTS;	/* DTR = 0, RTS = 0 */
        ioctl(mouse->mseFd, TIOCMBIC, &i);
        usleep(200000);

	/* wait for respose, 2nd phase (2.1.6) */
	xf86FlushInput(mouse->mseFd);
        i = TIOCM_DTR | TIOCM_RTS;	/* DTR = 1, RTS = 1 */
        ioctl(mouse->mseFd, TIOCMBIS, &i);

        /* try to read something */
        FD_ZERO(&fds);
        FD_SET(mouse->mseFd, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        if (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) <= 0)
	    goto connect_idle;
    }
#else
    /*
     * This is a simplified procedure; it simply toggles RTS.
     */
    xf86SetMouseSpeed(mouse, 1200, 1200, (CS7 | CREAD | CLOCAL | HUPCL));

    ioctl(mouse->mseFd, TIOCMGET, &i);
    i |= TIOCM_DTR;		/* DTR = 1 */
    i &= ~TIOCM_RTS;		/* RTS = 0 */
    ioctl(mouse->mseFd, TIOCMSET, &i);
    usleep(200000);

    /* wait for response */
    xf86FlushInput(mouse->mseFd);
    i = TIOCM_DTR | TIOCM_RTS;	/* DTR = 1, RTS = 1 */
    ioctl(mouse->mseFd, TIOCMBIS, &i);

    /* try to read something */
    FD_ZERO(&fds);
    FD_SET(mouse->mseFd, &fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    if (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) <= 0)
        goto connect_idle;
#endif

    /* collect PnP COM device ID (2.1.7) */
    i = 0;
    usleep(200000);	/* the mouse must send `Begin ID' within 200msec */
    while (read(mouse->mseFd, &c, 1) == 1) {
	/* we may see "M", or "M3..." before `Begin ID' */
        if ((c == 0x08) || (c == 0x28)) {	/* Begin ID */
	    buf[i++] = c;
	    break;
        }
    }
    if (i <= 0) {
	/* we haven't seen `Begin ID' in time... */
	goto connect_idle;
    }

    ++c;			/* make it `End ID' */
    for (;;) {
        FD_ZERO(&fds);
        FD_SET(mouse->mseFd, &fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        if (select(FD_SETSIZE, &fds, NULL, NULL, &timeout) <= 0)
	    break;

	read(mouse->mseFd, &buf[i], 1);
        if (buf[i++] == c)	/* End ID */
	    break;
	if (i >= 256)
	    break;
    }
    if (buf[i - 1] != c)
	goto connect_idle;
    return i;

    /*
     * According to PnP spec, we should set DTR = 1 and RTS = 0 while 
     * in idle state.  But, `moused' shall set DTR = RTS = 1 and proceed, 
     * assuming there is something at the port even if it didn't 
     * respond to the PnP enumeration procedure.
     */
disconnect_idle:
    i = TIOCM_DTR | TIOCM_RTS;		/* DTR = 1, RTS = 1 */
    ioctl(mouse->mseFd, TIOCMBIS, &i);
connect_idle:
    return 0;
}

static int
pnpparse(id, buf, len)
pnpid_t *id;
char *buf;
int len;
{
    char s[3];
    int offset;
    int sum = 0;
    int i, j;

    id->revision = 0;
    id->eisaid = NULL;
    id->serial = NULL;
    id->class = NULL;
    id->compat = NULL;
    id->description = NULL;
    id->neisaid = 0;
    id->nserial = 0;
    id->nclass = 0;
    id->ncompat = 0;
    id->ndescription = 0;

    offset = 0x28 - buf[0];

    /* calculate checksum */
    for (i = 0; i < len - 3; ++i) {
	sum += buf[i];
	buf[i] += offset;
    }
    sum += buf[len - 1];
    for (; i < len; ++i)
	buf[i] += offset;
    ErrorF("Mouse: PnP ID string: '%*.*s'\n", len, len, buf);

    /* revision */
    buf[1] -= offset;
    buf[2] -= offset;
    id->revision = ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f);
    ErrorF("Mouse: PnP rev %d.%02d\n", id->revision / 100, id->revision % 100);

    /* EISA vender and product ID */
    id->eisaid = &buf[3];
    id->neisaid = 7;

    /* option strings */
    i = 10;
    if (buf[i] == '\\') {
        /* device serial # */
        for (j = ++i; i < len; ++i) {
            if (buf[i] == '\\')
		break;
        }
	if (i >= len)
	    i -= 3;
	if (i - j == 8) {
            id->serial = &buf[j];
            id->nserial = 8;
	}
    }
    if (buf[i] == '\\') {
        /* PnP class */
        for (j = ++i; i < len; ++i) {
            if (buf[i] == '\\')
		break;
        }
	if (i >= len)
	    i -= 3;
	if (i > j + 1) {
            id->class = &buf[j];
            id->nclass = i - j;
        }
    }
    if (buf[i] == '\\') {
	/* compatible driver */
        for (j = ++i; i < len; ++i) {
            if (buf[i] == '\\')
		break;
        }
	/*
	 * PnP COM spec prior to v0.96 allowed '*' in this field, 
	 * it's not allowed now; just igore it.
	 */
	if (buf[j] == '*')
	    ++j;
	if (i >= len)
	    i -= 3;
	if (i > j + 1) {
            id->compat = &buf[j];
            id->ncompat = i - j;
        }
    }
    if (buf[i] == '\\') {
	/* product description */
        for (j = ++i; i < len; ++i) {
            if (buf[i] == ';')
		break;
        }
	if (i >= len)
	    i -= 3;
	if (i > j + 1) {
            id->description = &buf[j];
            id->ndescription = i - j;
        }
    }

    /* checksum exists if there are any optional fields */
    if ((id->nserial > 0) || (id->nclass > 0)
	|| (id->ncompat > 0) || (id->ndescription > 0)) {
#if 0
        ErrorF("Mouse: PnP checksum: 0x%02X\n", sum); 
#endif
        sprintf(s, "%02X", sum & 0x0ff);
        if (strncmp(s, &buf[len - 3], 2) != 0) {
#if 0
            /* 
	     * Checksum error!!
	     * I found some mice do not comply with the PnP COM device 
	     * spec regarding checksum... XXX
	     */
	    return FALSE;
#endif
        }
    }

    return TRUE;
}

static symtab_t *
pnpproto(id)
pnpid_t *id;
{
    symtab_t *t;
    int i, j;

    if (id->nclass > 0)
	if (strncmp(id->class, "MOUSE", id->nclass) != 0)
	    /* this is not a mouse! */
	    return NULL;

    if (id->neisaid > 0) {
        t = gettoken(pnpprod, id->eisaid, id->neisaid);
	if (t->val != -1)
            return t;
    }

    /*
     * The 'Compatible drivers' field may contain more than one
     * ID separated by ','.
     */
    if (id->ncompat <= 0)
	return NULL;
    for (i = 0; i < id->ncompat; ++i) {
        for (j = i; id->compat[i] != ','; ++i)
            if (i >= id->ncompat)
		break;
        if (i > j) {
            t = gettoken(pnpprod, id->compat + j, i - j);
	    if (t->val != -1)
                return t;
	}
    }

    return NULL;
}

/* name/val mapping */

static symtab_t *
gettoken(tab, s, len)
symtab_t *tab;
char *s;
int len;
{
    int i;

    for (i = 0; tab[i].name != NULL; ++i) {
	if (strncmp(tab[i].name, s, len) == 0)
	    break;
    }
    return &tab[i];
}
