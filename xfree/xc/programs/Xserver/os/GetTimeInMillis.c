#include "Xos.h"
#include "Xmd.h"

CARD32
GetTimeInMillis(void)
{
    struct timeval  tp;
    register CARD32 val;
    register INT32 diff;
    static CARD32 oldval = 0;
    static CARD32 time = 0;

    X_GETTIMEOFDAY(&tp);
    val = (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
    if (oldval) {
	diff = val - oldval;
	if (diff > 0)
	    time += diff;
    }
    oldval = val;

    return time;
}
