
#if PSZ == 8
#elif PSZ == 32

#define alphaSfbValidateGC	alphaSfb32ValidateGC
#define alphaSfbMatchCommon	alphaSfb32MatchCommon
#define alphaSfbCreateGC	alphaSfb32CreateGC

#define alphaSfbCopyArea	alphaSfb32CopyArea
#define alphaSfbCopyWindow	alphaSfb32CopyWindow
#define alphaSfbDoBitblt	alphaSfb32DoBitblt
#define alphaSfbDoBitbltCopy	alphaSfb32DoBitbltCopy
#define alphaSfbDoBitbltSimple	alphaSfb32DoBitbltSimple

#define alphaSfbGCFuncs		alphaSfb32GCFuncs
#define alphaSfbTEOps1Rect	alphaSfb32TEOps1Rect
#define alphaSfbNonTEOps1Rect	alphaSfb32NonTEOps1Rect
#define alphaSfbTEOps		alphaSfb32TEOps
#define alphaSfbNonTEOps	alphaSfb32NonTEOps

#else
#error Unsupported PSZ
#endif

#include <dev/tc/sfbreg.h>

void alphaSfbDoBitbltSimple(unsigned int *, unsigned int *, unsigned int,
    unsigned int, sfb_reg_t **, int, int, int, int, int, int, int, int, int);
