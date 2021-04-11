/*
 * Image handling functions
 *
 * This provides some general and hub stuff.  Details of different image
 * type functions, and generation of builtins, go in their own files.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "list.h"
#include "screen.h"

#include "image.h"
#include "image_bitmap.h"
#include "image_bitmap_builtin.h"
#include "image_xwd.h"
#ifdef JPEG
#include "image_jpeg.h"
#endif
#if defined (XPM)
#include "image_xpm.h"
#endif

/* Flag (maybe should be retired */
bool reportfilenotfound = false;
Colormap AlternateCmap = None;


/*
 * Find (load/generate) an image by name
 */
Image *
GetImage(const char *name, ColorPair cp)
{
	name_list **list;
#define GIFNLEN 256
	char fullname[GIFNLEN];
	Image *image;

	if(name == NULL) {
		return NULL;
	}
	image = NULL;

	list = &Scr->ImageCache;
	if(0) {
		/* dummy */ ;
	}
	else if((name [0] == '@') || (strncmp(name, "xpm:", 4) == 0)) {
#ifdef XPM
		snprintf(fullname, GIFNLEN, "%s%dx%d", name, (int) cp.fore, (int) cp.back);

		if((image = LookInNameList(*list, fullname)) == NULL) {
			int startn = (name [0] == '@') ? 1 : 4;
			if((image = GetXpmImage(name + startn, cp)) != NULL) {
				AddToList(list, fullname, image);
			}
		}
#else
		fprintf(stderr, "XPM support disabled, ignoring image %s\n", name);
		return NULL;
#endif
	}
	else if(strncmp(name, "jpeg:", 5) == 0) {
#ifdef JPEG
		if((image = LookInNameList(*list, name)) == NULL) {
			if((image = GetJpegImage(&name [5])) != NULL) {
				AddToList(list, name, image);
			}
		}
#else
		fprintf(stderr, "JPEG support disabled, ignoring image %s\n", name);
		return NULL;
#endif
	}
	else if((strncmp(name, "xwd:", 4) == 0) || (name [0] == '|')) {
		int startn = (name [0] == '|') ? 0 : 4;
		if((image = LookInNameList(*list, name)) == NULL) {
			if((image = GetXwdImage(&name [startn], cp)) != NULL) {
				AddToList(list, name, image);
			}
		}
	}
	else if(strncmp(name, ":xpm:", 5) == 0) {
		snprintf(fullname, GIFNLEN, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
		if((image = LookInNameList(*list, fullname)) == NULL) {
			image = get_builtin_scalable_pixmap(name, cp);
			if(image == NULL) {
				/* g_b_s_p() already warned */
				return NULL;
			}
			AddToList(list, fullname, image);
		}
	}
	else if(strncmp(name, "%xpm:", 5) == 0) {
		snprintf(fullname, GIFNLEN, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
		if((image = LookInNameList(*list, fullname)) == NULL) {
			image = get_builtin_animated_pixmap(name, cp);
			if(image == NULL) {
				/* g_b_a_p() already warned */
				return NULL;
			}
			AddToList(list, fullname, image);
		}
	}
	else if(name [0] == ':') {
		unsigned int    width, height;
		Pixmap          pm = 0;
		XGCValues       gcvalues;

		snprintf(fullname, GIFNLEN, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
		if((image = LookInNameList(*list, fullname)) == NULL) {
			pm = get_builtin_plain_pixmap(name, &width, &height);
			if(pm == None) {
				/* g_b_p_p() already warned */
				return NULL;
			}
			image = AllocImage();
			image->pixmap = XCreatePixmap(dpy, Scr->Root, width, height, Scr->d_depth);
			if(Scr->rootGC == (GC) 0) {
				Scr->rootGC = XCreateGC(dpy, Scr->Root, 0, &gcvalues);
			}
			gcvalues.background = cp.back;
			gcvalues.foreground = cp.fore;
			XChangeGC(dpy, Scr->rootGC, GCForeground | GCBackground, &gcvalues);
			XCopyPlane(dpy, pm, image->pixmap, Scr->rootGC, 0, 0, width, height, 0, 0,
			           (unsigned long) 1);
			image->width  = width;
			image->height = height;
			AddToList(list, fullname, image);
		}
	}
	else {
		snprintf(fullname, GIFNLEN, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
		if((image = LookInNameList(*list, fullname)) == NULL) {
			if((image = GetBitmapImage(name, cp)) != NULL) {
				AddToList(list, fullname, image);
			}
		}
	}
	return image;
#undef GIFNLEN
}


/*
 * Creation/cleanup of Image structs
 */
Image *
AllocImage(void)
{
	return calloc(1, sizeof(Image));
}

void
FreeImage(Image *image)
{
	Image *im, *im2;

	im = image;
	while(im != NULL) {
		/* Cleanup sub-bits */
		if(im->pixmap) {
			XFreePixmap(dpy, im->pixmap);
		}
		if(im->mask) {
			XFreePixmap(dpy, im->mask);
		}

		/* Cleanup self */
		im2 = im->next;
		im->next = NULL;
		free(im);

		/*
		 * Loop back around, unless we hit the original.  e.g.,
		 * "foo%.xpm" animations load the images into a closed loop, so
		 * FreeImage() would do Very Bad Things running around the track
		 * until it segfaults or the like.
		 */
		if(im2 == image) {
			break;
		}
		im = im2;
	}
}



/*
 * Utils for image*
 */

/*
 * Expand out the real pathname for an image.  Turn ~ into $HOME if
 * it's there, and look under the entries in PixmapDirectory if the
 * result isn't a full path.
 */
char *
ExpandPixmapPath(const char *name)
{
	char *ret;

	ret = NULL;

	/* If it starts with '~/', replace it with our homedir */
	if(name[0] == '~' && name[1] == '/') {
		asprintf(&ret, "%s/%s", Home, name + 2);
		return ret;
	}

	/*
	 * If it starts with /, it's an absolute path, so just pass it
	 * through.
	 */
	if(name[0] == '/') {
		return strdup(name);
	}

	/*
	 * If we got here, it's some sort of relative path (or a bare
	 * filename), so search for it under PixmapDirectory if we have it.
	 */
	if(Scr->PixmapDirectory) {
		char *colon;
		char *p = Scr->PixmapDirectory;

		/* PixmapDirectory is a colon-separated list */
		while((colon = strchr(p, ':'))) {
			*colon = '\0';
			asprintf(&ret, "%s/%s", p, name);
			*colon = ':';
			if(!access(ret, R_OK)) {
				return (ret);
			}
			free(ret);
			p = colon + 1;
		}

		asprintf(&ret, "%s/%s", p, name);
		if(!access(ret, R_OK)) {
			return (ret);
		}
		free(ret);
	}


	/*
	 * If we get here, we have no idea.  For simplicity and consistency
	 * for our callers, just return what we were given.
	 */
	return strdup(name);
}


/*
 * Generalized loader for animations.
 *
 * These are specified with a '%' in the filename, which is replaced by a
 * series of numbers.  So e.g.
 *
 * "foo%.xpm" -> [ "foo1.xpm", "foo2.xpm", ...]
 *
 * These then turn into a looped-linked-list of Image's.  We support
 * these for all types of images, so write it up into a central handler
 * once to centralize the logic.
 */
Image *
get_image_anim_cp(const char *name,
                  ColorPair cp, Image * (*imgloader)(const char *, ColorPair))
{
	Image   *head, *tail;
	char    *pref, *suff, *stmp;
	int     i;

	/* This shouldn't get called for non-animations */
	if((stmp = strchr(name, '%')) == NULL) {
		fprintf(stderr, "%s() called for non-animation '%s'\n", __func__, name);
		return NULL;
	}
	if(stmp[1] == '\0') {
		fprintf(stderr, "%s(): nothing after %% in '%s'\n", __func__, name);
		return NULL;
	}
	stmp = NULL;

	/*
	 * For animated requests, we load a series of files, replacing the %
	 * with numbers in series.
	 */
	tail = head = NULL;

	/* Working copy of the filename split to before/after the % */
	pref = strdup(name);
	suff = strchr(pref, '%');
	*suff++ = '\0';

	/* "foo%.xpm" -> [ "foo1.xpm", "foo2.xpm", ...] */
	for(i = 1 ; ; i++) {
#define ANIM_PATHLEN 256
		char path[ANIM_PATHLEN];
		Image *tmp;

		if(snprintf(path, ANIM_PATHLEN, "%s%d%s", pref, i,
		                suff) >= (ANIM_PATHLEN - 1)) {
			fprintf(stderr, "%s(): generated filename for '%s' #%d longer than %d.\n",
			        __func__, name, i, ANIM_PATHLEN);
			FreeImage(head);
			free(pref);
			return NULL;
		}
#undef ANIM_PATHLEN

		/*
		 * Load this image, and set ->next so it's explicitly the
		 * [current] tail of the list.
		 */
		tmp = imgloader(path, cp);
		if(tmp == NULL) {
			break;
		}
		tmp->next = NULL;

		/*
		 * If it's the first, it's the head (image) we return, as well as
		 * our current tail marker (s).  Else, append to that tail.
		 */
		if(head == NULL) {
			tail = head = tmp;
		}
		else {
			tail->next = tmp;
			tail = tmp;
		}
	}
	free(pref);

	/* Set the tail to loop back to the head */
	if(tail != NULL) {
		tail->next = head;
	}

	/* Warn if we got nothing */
	if(head == NULL) {
		fprintf(stderr, "Cannot find any image frames for '%s'\n", name);
	}

	return head;
}
