/*
 * Bitmap image handling function bits
 */
#ifndef _CTWM_IMAGE_BITMAP_H
#define _CTWM_IMAGE_BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Image *GetBitmapImage(const char *name, ColorPair cp);
Pixmap GetBitmap(const char *name);

/* Used for cursors */
extern int HotX, HotY;

#endif /* _CTWM_IMAGE_BITMAP_H */
