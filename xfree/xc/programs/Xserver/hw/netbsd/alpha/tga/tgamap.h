
#if PSZ == 8
#elif PSZ == 32

#define alphaTgaValidateGC	alphaTga32ValidateGC
#define alphaTgaMatchCommon	alphaTga32MatchCommon
#define alphaTgaCreateGC	alphaTga32CreateGC

#define alphaTgaCopyArea	alphaTga32CopyArea
#define alphaTgaCopyWindow	alphaTga32CopyWindow
#define alphaTgaDoBitblt	alphaTga32DoBitblt
#define alphaTgaDoBitbltCopy	alphaTga32DoBitbltCopy
#define alphaTgaDoBitbltSimple	alphaTga32DoBitbltSimple

#define alphaTgaGCFuncs		alphaTga32GCFuncs
#define alphaTgaTEOps1Rect	alphaTga32TEOps1Rect
#define alphaTgaNonTEOps1Rect	alphaTga32NonTEOps1Rect
#define alphaTgaTEOps		alphaTga32TEOps
#define alphaTgaNonTEOps	alphaTga32NonTEOps

#else
#error Unsupported PSZ
#endif

#include <dev/pci/tgareg.h>

void alphaTgaDoBitbltSimple(unsigned int *, unsigned int *, unsigned int,
    unsigned int, tga_reg_t **, int, int, int, int, int, int, int, int, int);
