/* 
   QuartzPasteboard.h

   Mac OS X pasteboard <-> X cut buffer
   Greg Parker     gparker@cs.stanford.edu     March 8, 2001
*/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzPasteboard.h,v 1.1 2001/03/15 22:24:27 torrey Exp $ */

#ifndef _QUARTZPASTEBOARD_H
#define _QUARTZPASTEBOARD_H

// Aqua->X 
void QuartzReadPasteboard();
char * QuartzReadCocoaPasteboard(void);	// caller must free string

// X->Aqua
void QuartzWritePasteboard();
void QuartzWriteCocoaPasteboard(char *text);

#endif	/* _QUARTZPASTEBOARD_H */