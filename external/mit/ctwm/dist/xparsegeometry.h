/*
 * Copyright notice...
 */

#ifndef _CTWM_XPARSEGEOMETRY_H
#define _CTWM_XPARSEGEOMETRY_H

#include "r_structs.h"

int RLayoutXParseGeometry(RLayout *layout, const char *geometry, int *x, int *y,
                          unsigned int *width, unsigned int *height);

#endif  /* _CTWM_XPARSEGEOMETRY_H */
