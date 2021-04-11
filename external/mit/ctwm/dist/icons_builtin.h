/*
 * Builtin icon builders
 */

#ifndef _CTWM_ICONS_BUILTIN_H
#define _CTWM_ICONS_BUILTIN_H

Pixmap CreateMenuIcon(int height, unsigned int *widthp, unsigned int *heightp);
Pixmap Create3DMenuIcon(unsigned int height,
                        unsigned int *widthp, unsigned int *heightp,
                        ColorPair cp);

extern const unsigned int im_iconified_icon_width;
extern const unsigned int im_iconified_icon_height;

Pixmap Create3DIconManagerIcon(ColorPair cp);
Pixmap Create2DIconManagerIcon(void);


#endif // _CTWM_ICONS_BUILTIN_H
