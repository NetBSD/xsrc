#ifndef vl_hwmc_h
#define vl_hwmc_h

#include <xf86xvmc.h>

XF86MCAdaptorPtr vlCreateAdaptorXvMC(ScreenPtr pScreen, char *xv_adaptor_name);
void vlDestroyAdaptorXvMC(XF86MCAdaptorPtr adaptor);
void vlInitXvMC(ScreenPtr pScreen, unsigned int num_adaptors, XF86MCAdaptorPtr *adaptors);

#endif

