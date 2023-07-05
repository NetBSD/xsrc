/*
 * Copyright notice...
 */

#ifndef _CTWM_R_LAYOUT_H
#define _CTWM_R_LAYOUT_H

#include "r_structs.h"


RLayout *RLayoutNew(RAreaList *monitors);
void RLayoutFree(RLayout *self);

RLayout *RLayoutCopyCropped(const RLayout *self, int left_margin,
                            int right_margin,
                            int top_margin, int bottom_margin);

RLayout *RLayoutSetMonitorsNames(RLayout *self, char **names);

RArea RLayoutGetAreaAtXY(const RLayout *self, int x, int y);
RArea RLayoutGetAreaIndex(const RLayout *self, int index);
RArea RLayoutGetAreaByName(const RLayout *self, const char *name, int len);

void RLayoutFindTopBottomEdges(const RLayout *self, const RArea *area, int *top,
                               int *bottom);
int RLayoutFindBottomEdge(const RLayout *self, const RArea *area);
int RLayoutFindTopEdge(const RLayout *self, const RArea *area);
void RLayoutFindLeftRightEdges(const RLayout *self, const RArea *area,
                               int *left,
                               int *right);
int RLayoutFindLeftEdge(const RLayout *self, const RArea *area);
int RLayoutFindRightEdge(const RLayout *self, const RArea *area);

int RLayoutFindMonitorBottomEdge(const RLayout *self, const RArea *area);
int RLayoutFindMonitorTopEdge(const RLayout *self, const RArea *area);
int RLayoutFindMonitorLeftEdge(const RLayout *self, const RArea *area);
int RLayoutFindMonitorRightEdge(const RLayout *self, const RArea *area);

RArea RLayoutFullHoriz(const RLayout *self, const RArea *area);
RArea RLayoutFullVert(const RLayout *self, const RArea *area);
RArea RLayoutFull(const RLayout *self, const RArea *area);
RArea RLayoutFullHoriz1(const RLayout *self, const RArea *area);
RArea RLayoutFullVert1(const RLayout *self, const RArea *area);
RArea RLayoutFull1(const RLayout *self, const RArea *area);

RArea RLayoutBigArea(const RLayout *self);
int RLayoutNumMonitors(const RLayout *self);
void RLayoutPrint(const RLayout *self);

#endif  /* _CTWM_R_LAYOUT_H */
