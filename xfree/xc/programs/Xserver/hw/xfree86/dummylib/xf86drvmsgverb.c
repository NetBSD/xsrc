/* $XFree86$ */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "dummylib.h"

/*
 * Utility functions required by libxf86_os. 
 */

void
xf86DrvMsgVerb(int i, MessageType type, int verb, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    VErrorFVerb(verb, format, ap);
    va_end(ap);
}

