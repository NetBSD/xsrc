
#if PSZ == 8
#elif PSZ == 32

#define decTgaValidateGC	decTga32ValidateGC
#define decTgaMatchCommon	decTga32MatchCommon
#define decTgaCreateGC		decTga32CreateGC

#define decTgaCopyArea		decTga32CopyArea
#define decTgaCopyWindow	decTga32CopyWindow
#define decTgaDoBitblt		decTga32DoBitblt
#define decTgaDoBitbltCopy	decTga32DoBitbltCopy
#define decTgaDoBitbltSimple	decTga32DoBitbltSimple

#define decTgaGCFuncs		decTga32GCFuncs
#define decTgaTEOps1Rect	decTga32TEOps1Rect
#define decTgaNonTEOps1Rect	decTga32NonTEOps1Rect
#define decTgaTEOps		decTga32TEOps
#define decTgaNonTEOps		decTga32NonTEOps

#else
#error Unsupported PSZ
#endif

#include <dev/pci/tgareg.h>

void decTgaDoBitbltSimple(unsigned int *, unsigned int *, unsigned int,
    unsigned int, tga_reg_t **, int, int, int, int, int, int, int, int, int);
