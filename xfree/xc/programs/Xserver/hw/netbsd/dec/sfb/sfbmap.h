
#if PSZ == 8
#elif PSZ == 32

#define decSfbValidateGC	decSfb32ValidateGC
#define decSfbMatchCommon	decSfb32MatchCommon
#define decSfbCreateGC		decSfb32CreateGC

#define decSfbCopyArea		decSfb32CopyArea
#define decSfbCopyWindow	decSfb32CopyWindow
#define decSfbDoBitblt		decSfb32DoBitblt
#define decSfbDoBitbltCopy	decSfb32DoBitbltCopy
#define decSfbDoBitbltSimple	decSfb32DoBitbltSimple

#define decSfbGCFuncs		decSfb32GCFuncs
#define decSfbTEOps1Rect	decSfb32TEOps1Rect
#define decSfbNonTEOps1Rect	decSfb32NonTEOps1Rect
#define decSfbTEOps		decSfb32TEOps
#define decSfbNonTEOps		decSfb32NonTEOps

#else
#error Unsupported PSZ
#endif

#include <dev/tc/sfbreg.h>

void decSfbDoBitbltSimple(unsigned int *, unsigned int *, unsigned int,
    unsigned int, sfb_reg_t **, int, int, int, int, int, int, int, int, int);
