/*
 * quartz.h
 *
 * External interface of the Quartz modes seen by the generic, mode
 * independent parts of the Darwin X server.
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartz.h,v 1.8 2001/12/22 05:28:35 torrey Exp $ */

#ifndef _QUARTZ_H
#define _QUARTZ_H

#include "screenint.h"
#include "quartzPasteboard.h"

int QuartzProcessArgument(int argc, char *argv[], int i);
void QuartzInitOutput(int argc, char **argv);
Bool QuartzAddScreen(int index, ScreenPtr pScreen);
Bool QuartzSetupScreen(int index, ScreenPtr pScreen);
void QuartzGiveUp(void);
void QuartzHide(void);
void QuartzShow(int x, int y);

#endif
