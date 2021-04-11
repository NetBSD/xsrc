/*
 * General image handling function bits
 */
#ifndef _CTWM_IMAGE_H
#define _CTWM_IMAGE_H


/* Widely used through the codebase */
struct Image {
	Pixmap pixmap;
	Pixmap mask;
	int    width;
	int    height;
	Image *next;
};


Image *GetImage(const char *name, ColorPair cp);
Image *AllocImage(void);
void FreeImage(Image *image);


/* Used internally in image*.c */
extern bool reportfilenotfound;
extern Colormap AlternateCmap;

char *ExpandPixmapPath(const char *name);
Image *get_image_anim_cp(const char *name, ColorPair cp,
                         Image * (*imgloader)(const char *, ColorPair));


/*
 * These are really image_bitmap_builtin stuff, but a few places in the
 * codebase reference them, and there's no need for them to pull in a
 * pretty deep internal header to do it.
 */
#define TBPM_DOT ":dot"         /* name of titlebar pixmap for dot */
#define TBPM_ICONIFY ":iconify" /* same image as dot */
#define TBPM_RESIZE ":resize"   /* name of titlebar pixmap for resize button */
#define TBPM_XLOGO ":xlogo"     /* name of titlebar pixmap for xlogo */
#define TBPM_DELETE ":delete"   /* same image as xlogo */
#define TBPM_MENU ":menu"       /* name of titlebar pixmap for menus */
#define TBPM_QUESTION ":question"       /* name of unknown titlebar pixmap */

#define TBPM_3DCROSS ":xpm:cross"
#define TBPM_3DICONIFY ":xpm:iconify"
#define TBPM_3DSUNKEN_RESIZE ":xpm:sunkresize"
#define TBPM_3DBOX ":xpm:box"

#define TBPM_3DDOT ":xpm:dot"           /* name of titlebar pixmap for dot */
#define TBPM_3DRESIZE ":xpm:resize"     /* name of titlebar pixmap for resize button */
#define TBPM_3DMENU ":xpm:menu" /* name of titlebar pixmap for menus */
#define TBPM_3DZOOM ":xpm:zoom"
#define TBPM_3DBAR ":xpm:bar"
#define TBPM_3DVBAR ":xpm:vbar"

/* Ditto for a few funcs */
Pixmap mk_blackgray_pixmap(const char *which, Drawable dw,
                           unsigned long fg, unsigned long bg);
void get_blackgray_size(int *width, int *height);

#endif /* _CTWM_IMAGE_H */
