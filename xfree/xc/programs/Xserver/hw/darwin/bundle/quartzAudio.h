//
// QuartzAudio.h
//
// X Window bell support using CoreAudio or AppKit.
// Greg Parker   gparker@cs.stanford.edu   19 Feb 2001
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzAudio.h,v 1.2 2001/04/01 20:45:43 tsi Exp $ */

#ifndef _QUARTZAUDIO_H
#define _QUARTZAUDIO_H

#include "input.h"

void QuartzAudioInit(void);
void QuartzBell(int volume, DeviceIntPtr pDevice, pointer ctrl, int class);

#endif
