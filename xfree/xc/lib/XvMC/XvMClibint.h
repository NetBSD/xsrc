/* $XFree86: xc/lib/XvMC/XvMClibint.h,v 1.3 2001/04/01 14:00:02 tsi Exp $ */

#ifndef _XVMCLIBINT_H
#define _XVMCLIBINT_H
#define NEED_REPLIES

#include "Xlibint.h"
#include "Xvproto.h"
#include "XvMCproto.h"
#include "XvMClib.h"

#define XvMCCheckExtension(dpy, i, val) \
  XextCheckExtension(dpy, i, xvmc_extension_name, val)


#if defined(__STDC__) && !defined(UNIXCPP)
#define XvMCGetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + sizeof(xvmc##name##Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xvmc##name##Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
        req->xvmcReqType = xvmc_##name; \
        req->length = sizeof(xvmc##name##Req)>>2;\
	dpy->bufptr += sizeof(xvmc##name##Req);\
	dpy->request++
#else
#define XvMCGetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + sizeof(xvmc/**/name/**/Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xvmc/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
	req->xvmcReqType = xvmc_/**/name;\
	req->length = sizeof(xvmc/**/name/**/Req)>>2;\
	dpy->bufptr += sizeof(xvmc/**/name/**/Req);\
	dpy->request++
#endif

#endif /* XVMCLIBINT_H */
