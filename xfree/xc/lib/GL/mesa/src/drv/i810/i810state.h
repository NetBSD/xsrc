#ifndef _I810_STATE_H
#define _I810_STATE_H

#include "i810context.h"

extern void i810DDUpdateHwState( GLcontext *ctx );
extern void i810DDUpdateState( GLcontext *ctx );
extern void i810DDInitState( i810ContextPtr imesa );
extern void i810DDInitStateFuncs( GLcontext *ctx );


#endif
