/*
 * Copyright notice...
 */

#ifndef _CTWM_R_AREA_H
#define _CTWM_R_AREA_H

#include "r_structs.h"


RArea *RAreaNewStatic(int x, int y, int width, int height);
RArea RAreaNew(int x, int y, int width, int height);

RArea RAreaInvalid(void);
bool RAreaIsValid(const RArea *self);

int RAreaX2(const RArea *self);
int RAreaY2(const RArea *self);
int RAreaArea(const RArea *self);
RArea RAreaIntersect(const RArea *self, const RArea *other);
bool RAreaIsIntersect(const RArea *self, const RArea *other);
bool RAreaContainsXY(const RArea *self, int x, int y);
RAreaList *RAreaHorizontalUnion(const RArea *self, const RArea *other);
RAreaList *RAreaVerticalUnion(const RArea *self, const RArea *other);

void RAreaPrint(const RArea *self);

#endif  /* _CTWM_R_AREA_H */
