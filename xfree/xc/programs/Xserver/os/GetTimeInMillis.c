#include "Xos.h"
#include "Xmd.h"

CARD32
GetTimeInMillis(void)
{
    struct timeval  tp;

    X_GETTIMEOFDAY(&tp);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
