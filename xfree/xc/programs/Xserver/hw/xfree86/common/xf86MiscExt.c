/*
 * Copyright (c) 1999 by The XFree86 Project, Inc.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86MiscExt.c,v 1.8 2001/08/16 14:33:51 dawes Exp $ */

/*
 * This file contains the Pointer/Keyboard functions needed by the 
 * XFree86-Misc extension.
 */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

#ifdef XF86MISC
#define _XF86MISC_SERVER_
#include "xf86misc.h"
#include "xf86miscproc.h"
#endif

#define XF86_OS_PRIVS
#include "xf86_OSlib.h"

#ifdef XINPUT
#include "XI.h"
#include "XIproto.h"
#include "xf86Xinput.h"
#else
#include "inputstr.h"
#endif

#include "xf86OSmouse.h"
#include "../input/mouse/mouse.h"

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

#ifdef XF86MISC

typedef struct {
	int	type;
	int	baudrate;
	int	samplerate;
	int	resolution;
	int	buttons;
	Bool	em3buttons;
	int	em3timeout;
	Bool	chordmiddle;
	int	flags;
        char*   device;
        pointer	private;
} mseParamsRec, *mseParamsPtr;

typedef struct {
	int	type;
	int	rate;
	int	delay;
	int	serverNumLock;	/* obsolete */
} kbdParamsRec, *kbdParamsPtr;

typedef enum {
    TO_MISC,
    FROM_MISC
} MseProtoMapDirection;

static void MiscExtClientStateCallback(pointer, pointer, pointer);

/*
    Sigh...

    The extension should probably be changed to use protocol
    names instead of ID numbers
*/
static int
MapMseProto(int proto, MseProtoMapDirection mapping)
{
    static int MapProto_ToMisc[] = {
	MTYPE_MICROSOFT,	MTYPE_MOUSESYS,		MTYPE_MMSERIES,
	MTYPE_LOGITECH,		MTYPE_LOGIMAN,		MTYPE_MMHIT,
	MTYPE_GLIDEPOINT,	MTYPE_IMSERIAL,		MTYPE_THINKING,
	MTYPE_ACECAD,		MTYPE_PS_2,		MTYPE_IMPS2,
	MTYPE_EXPPS2,           MTYPE_THINKINGPS2,	MTYPE_MMANPLUSPS2,
	MTYPE_GLIDEPOINTPS2,	MTYPE_NETPS2,		MTYPE_NETSCROLLPS2,
	MTYPE_BUSMOUSE, 	MTYPE_AUTOMOUSE,	MTYPE_SYSMOUSE
    };

    static MouseProtocolID MapProto_FromMisc[] = {
	PROT_MS,	PROT_MSC,	PROT_MM,	PROT_LOGI,
	PROT_BM,	PROT_LOGIMAN,	PROT_PS2,	PROT_MMHIT,
	PROT_GLIDE,	PROT_IMSERIAL,	PROT_THINKING,	PROT_IMPS2,
	PROT_THINKPS2,	PROT_MMPS2,	PROT_GLIDEPS2,	PROT_NETPS2,
	PROT_NETSCPS2,	PROT_SYSMOUSE,	PROT_AUTO,	PROT_ACECAD,
	PROT_EXPPS2
    };
#define PROT_DEFAULT  -2 /* PROT_UNKNOWN */

    if (mapping == TO_MISC) {
	if (proto < 0 || proto >= sizeof(MapProto_ToMisc)/sizeof(int))
		return MTYPE_UNKNOWN;
	return MapProto_ToMisc[proto];
    } else {
	if (proto < 0 || proto >= sizeof(MapProto_FromMisc)/sizeof(int))
		return PROT_DEFAULT;
	return MapProto_FromMisc[proto];
    }
#undef PROT_DEFAULT
}

Bool
MiscExtGetMouseSettings(pointer *mouse, char **devname)
{
    mseParamsPtr mseptr;

    DEBUG_P("MiscExtGetMouseSettings");

    mseptr = MiscExtCreateStruct(MISC_POINTER);
    if (!mseptr)
	return FALSE;

    {
	InputInfoPtr pInfo = mseptr->private;
	MouseDevPtr pMse;

	*devname = xf86FindOptionValue(pInfo->options, "Device");
	pMse = pInfo->private;

	mseptr->type =		MapMseProto(pMse->protocolID, TO_MISC);
	mseptr->baudrate =	pMse->baudRate;
	mseptr->samplerate =	pMse->sampleRate;
	mseptr->resolution =	pMse->resolution;
	mseptr->buttons =	pMse->buttons;
	mseptr->em3buttons =	pMse->emulate3Buttons;
	mseptr->em3timeout =	pMse->emulate3Timeout;
	mseptr->chordmiddle =	pMse->chordMiddle;
	mseptr->flags =		pMse->mouseFlags;
    }
    *mouse = mseptr;
    return TRUE;
}

int
MiscExtGetMouseValue(pointer mouse, MiscExtMseValType valtype)
{
    mseParamsPtr mse = mouse;

    DEBUG_P("MiscExtGetMouseValue");

    switch (valtype) {
	case MISC_MSE_PROTO:		return mse->type;
	case MISC_MSE_BAUDRATE:		return mse->baudrate;
	case MISC_MSE_SAMPLERATE:	return mse->samplerate;
	case MISC_MSE_RESOLUTION:	return mse->resolution;
	case MISC_MSE_BUTTONS:		return mse->buttons;
	case MISC_MSE_EM3BUTTONS:	return mse->em3buttons;
	case MISC_MSE_EM3TIMEOUT:	return mse->em3timeout;
	case MISC_MSE_CHORDMIDDLE:	return mse->chordmiddle;
	case MISC_MSE_FLAGS:		return mse->flags;
    }
    return 0;
}

Bool
MiscExtSetMouseValue(pointer mouse, MiscExtMseValType valtype, int value)
{
    mseParamsPtr mse = mouse;

    DEBUG_P("MiscExtSetMouseValue");

    switch (valtype) {
	case MISC_MSE_PROTO:
	    mse->type = value;
		return TRUE;
	case MISC_MSE_BAUDRATE:
		mse->baudrate = value;
		return TRUE;
	case MISC_MSE_SAMPLERATE:
		mse->samplerate = value;
		return TRUE;
	case MISC_MSE_RESOLUTION:
		mse->resolution = value;
		return TRUE;
	case MISC_MSE_BUTTONS:
		mse->buttons = value;
		return TRUE;
	case MISC_MSE_EM3BUTTONS:
		mse->em3buttons = value;
		return TRUE;
	case MISC_MSE_EM3TIMEOUT:
		mse->em3timeout = value;
		return TRUE;
	case MISC_MSE_CHORDMIDDLE:
		mse->chordmiddle = value;
		return TRUE;
	case MISC_MSE_FLAGS:
		mse->flags = value;
		return TRUE;
    }
    return FALSE;
}

/* The misc extension doesn't (yet) call this. */
#if 0
Bool
MiscExtSetMouseDevice(pointer mouse, char* device)
{
    mseParamsPtr mse = mouse;

    mse->device = device;
    return TRUE;
}
#endif
                                                                               
Bool
MiscExtGetKbdSettings(pointer *kbd)
{
    kbdParamsPtr kbdptr;

    DEBUG_P("MiscExtGetKbdSettings");

    kbdptr = MiscExtCreateStruct(MISC_KEYBOARD);
    if (!kbdptr)
	return FALSE;
    kbdptr->type  = xf86Info.kbdType;
    kbdptr->rate  = xf86Info.kbdRate;
    kbdptr->delay = xf86Info.kbdDelay;
    *kbd = kbdptr;
    return TRUE;
}

int
MiscExtGetKbdValue(pointer keyboard, MiscExtKbdValType valtype)
{
    kbdParamsPtr kbd = keyboard;

    DEBUG_P("MiscExtGetKbdValue");
    switch (valtype) {
	case MISC_KBD_TYPE:		return kbd->type;
	case MISC_KBD_RATE:		return kbd->rate;
	case MISC_KBD_DELAY:		return kbd->delay;
	case MISC_KBD_SERVNUMLOCK:	return 0;
    }
    return 0;
}

Bool
MiscExtSetKbdValue(pointer keyboard, MiscExtKbdValType valtype, int value)
{
    kbdParamsPtr kbd = keyboard;

    DEBUG_P("MiscExtSetKbdValue");
    switch (valtype) {
	case MISC_KBD_TYPE:
		kbd->type = value;
		return TRUE;
	case MISC_KBD_RATE:
		kbd->rate = value;
		return TRUE;
	case MISC_KBD_DELAY:
		kbd->delay = value;
		return TRUE;
	case MISC_KBD_SERVNUMLOCK:
		return TRUE;
    }
    return FALSE;
}

static void
MiscExtClientStateCallback(pointer callbacks, pointer data, pointer args)
{
    NewClientInfoRec *clientinfo = (NewClientInfoRec*)args;

    if (clientinfo->client == xf86Info.grabInfo.override &&
	clientinfo->client->clientState == ClientStateGone) {
	xf86Info.grabInfo.override = NULL;
	xf86Info.grabInfo.disabled = 0;
	DeleteCallback(&ClientStateCallback,
		       (CallbackProcPtr)MiscExtClientStateCallback, NULL);
    }
}

#define MiscExtGrabStateSuccess	0	/* No errors */
#define MiscExtGrabStateLocked	1	/* A client already requested that
					 * grabs cannot be removed/killed */
#define MiscExtGrabStateAlready	2	/* Request for enabling/disabling
					 * grab removeal/kill already done */
int
MiscExtSetGrabKeysState(ClientPtr client, int state)
{
    DEBUG_P("MiscExtSetGrabKeysState");

    if (xf86Info.grabInfo.override == NULL ||
	xf86Info.grabInfo.override == client) {
	if (state == 0 && xf86Info.grabInfo.disabled == 0) {
	    xf86Info.grabInfo.disabled = 1;
	    AddCallback(&ClientStateCallback,
			(CallbackProcPtr)MiscExtClientStateCallback, NULL);
	    xf86Info.grabInfo.override = client;
	}
	else if (state == 1 && xf86Info.grabInfo.disabled == 1) {
	    xf86Info.grabInfo.disabled = 0;
	    DeleteCallback(&ClientStateCallback,
			   (CallbackProcPtr)MiscExtClientStateCallback, NULL);
	    xf86Info.grabInfo.override = NULL;
	}
	else
	    return MiscExtGrabStateAlready;

	return MiscExtGrabStateSuccess;
    }

    return MiscExtGrabStateLocked;
}

pointer
MiscExtCreateStruct(MiscExtStructType mse_or_kbd)
{
    DEBUG_P("MiscExtCreateStruct");
    
    switch (mse_or_kbd) {
    case MISC_POINTER:
    {
	mseParamsPtr mseptr;
	InputInfoPtr pInfo = xf86InputDevs;
	
	while (pInfo) {
	    if (xf86IsCorePointer(pInfo->dev))
		break;
	    pInfo = pInfo->next;
	}
	if (!pInfo)
	    return NULL;
	
	mseptr = xcalloc(sizeof(mseParamsRec),1);
	if (mseptr)
	    mseptr->private = pInfo;
	return mseptr;
    }
    case MISC_KEYBOARD:
	return xcalloc(sizeof(kbdParamsRec),1);
    }
    return 0;
}

void
MiscExtDestroyStruct(pointer structure, MiscExtStructType mse_or_kbd)
{
    DEBUG_P("MiscExtDestroyStruct");

    switch (mse_or_kbd) {
	case MISC_POINTER:
	case MISC_KEYBOARD:
		xfree(structure);
    }
}

MiscExtReturn
MiscExtApply(pointer structure, MiscExtStructType mse_or_kbd)
{
    DEBUG_P("MiscExtApply");

    if (mse_or_kbd == MISC_POINTER) {
	Bool reopen = FALSE;
	mseParamsPtr mse = structure;
	InputInfoPtr pInfo;
	MouseDevPtr pMse;
#ifdef XFree86LOADER
	pointer xf86MouseProtocolIDToName
	    = LoaderSymbol("xf86MouseProtocolIDToName");
	if (!xf86MouseProtocolIDToName)
	    return MISC_RET_NOMODULE;
#endif
	if (mse->type < MTYPE_MICROSOFT
		|| ( mse->type > MTYPE_ACECAD
		    && (mse->type!=MTYPE_OSMOUSE && mse->type!=MTYPE_XQUEUE)))
	    return MISC_RET_BADMSEPROTO;
#ifdef OSMOUSE_ONLY
	if (mse->type != MTYPE_OSMOUSE)
	    return MISC_RET_BADMSEPROTO;
#else
	if (mse->type == MTYPE_XQUEUE)
	    return MISC_RET_BADMSEPROTO;
	if (mse->type == MTYPE_OSMOUSE)
	    return MISC_RET_BADMSEPROTO;
#endif /* OSMOUSE_ONLY */

	if (mse->em3timeout < 0)
	    return MISC_RET_BADVAL;

	if (mse->type == MTYPE_LOGIMAN
		&& !(mse->baudrate == 1200 || mse->baudrate == 9600) )
	    return MISC_RET_BADBAUDRATE;
	if (mse->type == MTYPE_LOGIMAN && mse->samplerate)
	    return MISC_RET_BADCOMBO;

	if (mse->flags & MF_REOPEN) {
	    reopen = TRUE;
	    mse->flags &= ~MF_REOPEN;
	}
	if (mse->type != MTYPE_OSMOUSE
		&& mse->type != MTYPE_XQUEUE
		&& mse->type != MTYPE_PS_2
		&& mse->type != MTYPE_BUSMOUSE
		&& mse->type != MTYPE_IMPS2
		&& mse->type != MTYPE_THINKINGPS2
		&& mse->type != MTYPE_MMANPLUSPS2
		&& mse->type != MTYPE_GLIDEPOINTPS2
		&& mse->type != MTYPE_NETPS2
		&& mse->type != MTYPE_NETSCROLLPS2
		&& mse->type != MTYPE_SYSMOUSE)
	{
	    if (mse->baudrate % 1200 != 0
		    || mse->baudrate < 1200 || mse->baudrate > 9600)
		return MISC_RET_BADBAUDRATE;
	}
	if ((mse->flags & (MF_CLEAR_DTR|MF_CLEAR_RTS))
		&& (mse->type != MTYPE_MOUSESYS))
	    return MISC_RET_BADFLAGS;

	if (mse->type != MTYPE_OSMOUSE
		&& mse->type != MTYPE_XQUEUE
		&& mse->type != MTYPE_BUSMOUSE)
	{
	    if (mse->samplerate < 0)
		return MISC_RET_BADVAL;
	}

	if (mse->resolution < 0)
	    return MISC_RET_BADVAL;
	if (mse->chordmiddle)
	{
	    if (mse->em3buttons || !(mse->type == MTYPE_MICROSOFT
				    || mse->type == MTYPE_LOGIMAN) )
		return MISC_RET_BADCOMBO;
	}

	/* XXX - This still needs work */

	pInfo = mse->private;
	pMse = pInfo->private;

	if (pMse->protocolID != MapMseProto(mse->type, FROM_MISC)
		|| pMse->baudRate != mse->baudrate
		|| pMse->sampleRate != mse->samplerate
		|| pMse->resolution != mse->resolution
		|| pMse->mouseFlags != mse->flags)
	    reopen = TRUE;

	if (reopen)
	    (pMse->device->deviceProc)(pMse->device, DEVICE_CLOSE);

	pMse->protocolID      = MapMseProto(mse->type, FROM_MISC);
	pMse->baudRate        = mse->baudrate;
	pMse->sampleRate      = mse->samplerate;
	pMse->resolution      = mse->resolution;
	pMse->buttons         = mse->buttons;
	pMse->emulate3Buttons = mse->em3buttons;
	pMse->emulate3Timeout = mse->em3timeout;
	pMse->chordMiddle     = mse->chordmiddle;
	pMse->mouseFlags      = mse->flags;

#ifdef XFree86LOADER
	pMse->protocol = ((const char *(*)(MouseProtocolID))
			  xf86MouseProtocolIDToName)(pMse->protocolID);
#else
	pMse->protocol = xf86MouseProtocolIDToName(pMse->protocolID);
#endif
	if (reopen)
	    (pMse->device->deviceProc)(pMse->device, DEVICE_ON);
	/* Set pInfo->options too */
	
       if (mse->device)
           xf86ReplaceStrOption(pInfo->options, "Device", mse->device);        

    }
    if (mse_or_kbd == MISC_KEYBOARD) {
	kbdParamsPtr kbd = structure;

	if (kbd->rate < 0)
	    return MISC_RET_BADVAL;
	if (kbd->delay < 0)
	    return MISC_RET_BADVAL;
	if (kbd->type < KTYPE_UNKNOWN || kbd->type > KTYPE_XQUEUE)
	    return MISC_RET_BADKBDTYPE;

	if (xf86Info.kbdRate!=kbd->rate || xf86Info.kbdDelay!=kbd->delay) {
	    char rad;

	    xf86Info.kbdRate = kbd->rate;
	    xf86Info.kbdDelay = kbd->delay;
	    if      (xf86Info.kbdDelay <= 375) rad = 0x00;
	    else if (xf86Info.kbdDelay <= 625) rad = 0x20;
	    else if (xf86Info.kbdDelay <= 875) rad = 0x40;
	    else                               rad = 0x60;

	    if      (xf86Info.kbdRate <=  2)   rad |= 0x1F;
	    else if (xf86Info.kbdRate >= 30)   rad |= 0x00;
	    else                               rad |= ((58/xf86Info.kbdRate)-2);

	    xf86SetKbdRepeat(rad);
	}
#if 0   /* Not done yet */
	xf86Info.kbdType = kbd->kbdtype;
#endif
    }
    return MISC_RET_SUCCESS;
}

#endif /* XF86MISC */

