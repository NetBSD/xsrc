/*
 * $Xorg: constype.c,v 1.3 2000/08/17 19:48:29 cpqbld Exp $
 *
 * consoletype - utility to print out string identifying Sun console type
 *
 * Copyright 1988 SRI
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SRI not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SRI makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Doug Moran, SRI
 */
/* $XFree86: xc/programs/Xserver/hw/sun/constype.c,v 3.10 2004/03/08 15:37:04 tsi Exp $ */

/*
SUN-SPOTS DIGEST         Thursday, 17 March 1988       Volume 6 : Issue 31

Date:    Wed, 2 Mar 88 14:50:26 PST
From:    Doug Moran <moran@ai.sri.com>
Subject: Program to determine console type

There have been several requests in this digest for programs to determine
the type of the console.  Below is a program that I wrote to produce an
identifying string (I start suntools in my .login file and use this pgm to
determine which arguments to use).

Caveat:  my cluster has only a few of these monitor types, so the pgm has
not been fully tested.

Note on coding style: the function wu_fbid is actually located in a local
library, accounting for what otherwise might appear to be a strange coding
style.
*/
#include <stdio.h>
#if defined(SVR4) || defined(CSRG_BASED)
#include <string.h>
#else
/*  SunOS  */
#include <strings.h>
#endif
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/file.h>
#if defined(SVR4) || defined(__bsdi__)
# include <fcntl.h>
# include <sys/fbio.h>
# ifdef sun
/* VIS_GETIDENTIFIER ioctl added in Solaris 2.3 */
/* Don't actually #include the header, so this can be built on older SunOS's */
/* #include <sys/visual_io.h> */
#  define VIS_GETIDENTIFIER	(('V' << 8) | 0)
#  define VIS_MAXNAMELEN	128
struct vis_identifier {
	char name[VIS_MAXNAMELEN];
};
# endif
#else
# ifndef __NetBSD__
#  ifdef CSRG_BASED
#   include <sun/fbio.h>
#  else
#   include <machine/fbio.h>
#  endif
# endif 
#endif

static int wu_fbid(char *devname, char **fbname, int *fbtype);

int
main(int argc, char **argv)
{
    int fbtype = -1;
    char *fbname, *dev;
    int print_num = 0;
    int error;

    if (argc > 1 && argv[1][0] == '/') {
	dev = argv[1];
	argc--; argv++;
    } else {
	dev = "/dev/fb";
    }
    error = wu_fbid(dev, &fbname, &fbtype );
    if (argc > 1 && strncmp(argv[1], "-num", strlen(argv[1])) == 0)
	print_num = 1;

    printf ("%s", fbname ? fbname : "tty");
    if (print_num && (fbtype >= 0)) {
	printf (" %d", fbtype);
    }
    putchar ('\n');
    return error;
}
#include <sys/ioctl.h>
#include <sys/file.h>
#if defined(SVR4) || defined(__bsdi__)
#include <fcntl.h>
#include <sys/fbio.h>
#if defined(SVR4) && defined(sun)
/* VIS_GETIDENTIFIER ioctl added in Solaris 2.3 */
#include <sys/visual_io.h>
#endif
#else
#ifndef CSRG_BASED
#include <sun/fbio.h>
#else
#  ifdef __NetBSD__
#   include <dev/sun/fbio.h>	/* -wsr */
#  else
#   include <machine/fbio.h>
#  endif
#endif
#endif

static char *decode_fb[] = {
    "bw1",	/* FBTYPE_SUN1BW */
    "cg1",	/* FBTYPE_SUN1COLOR */
    "bw2",	/* FBTYPE_SUN2BW */
    "cg2",	/* FBTYPE_SUN2COLOR */
    "gp2",	/* FBTYPE_SUN2GP */
    "bw3",	/* FBTYPE_SUN5COLOR */
    "cg3",	/* FBTYPE_SUN3COLOR */
    "cg8",	/* FBTYPE_MEMCOLOR */
    "cg4",	/* FBTYPE_SUN4COLOR */
    "nsA",	/* FBTYPE_NOTSUN1 */
    "nsB",	/* FBTYPE_NOTSUN2 */
    "nsC",	/* FBTYPE_NOTSUN3 */
    "cg6",	/* FBTYPE_SUNFAST_COLOR */
    "rop",	/* FBTYPE_SUNROP_COLOR */
    "vid",	/* FBTYPE_SUNFB_VIDEO */
    "gifb",	/* FBTYPE_SUNGIFB */
    "plas",	/* FBTYPE_SUNGPLAS */
    "cg12",	/* FBTYPE_SUNGP3 */
    "gt",	/* FBTYPE_SUNGT */
    "leo",	/* FBTYPE_SUNLEO */
    "cg14",	/* FBTYPE_MDICOLOR */
    "tcx",	/* FBTYPE_TCXCOLOR */		/* not in fbio.h */
    "creator",	/* FBTYPE_CREATOR */		/* not in fbio.h */
    "iga",	/* FBTYPE_PCI_IGA1682 */	/* not in fbio.h */
    "p9",	/* FBTYPE_P9100COLOR */		/* not in fbio.h */
    NULL
};

#define NUM_FBS ((int)(sizeof(decode_fb) / sizeof(decode_fb[0])))

static int wu_fbid(char* devname, char** fbname, int* fbtype)
{
    struct fbgattr fbattr;
    int fd;
#ifdef sun
    struct vis_identifier fbid;
#endif

    if ( (fd = open(devname, O_RDWR, 0)) == -1 ) {
	*fbname = "unable to open fb";
	*fbtype = -1;
	return 2;
    }

#ifdef sun
    if (ioctl(fd, VIS_GETIDENTIFIER, &fbid) >= 0) {
	*fbname = fbid.name;
	*fbtype = -1;
	close(fd);
	return 0;
    }
#endif

    /* FBIOGATTR fails for early frame buffer types */
    if ((*fbtype = ioctl(fd, FBIOGATTR, &fbattr)) < 0)
	*fbtype = ioctl(fd, FBIOGTYPE, &fbattr.fbtype);

    close(fd);

    if (*fbtype < 0) {
	*fbname = "ioctl on fb failed";
	*fbtype = errno;
	return 2;
    }

    *fbtype = fbattr.fbtype.fb_type;
    if ((*fbtype >= 0) && (*fbtype < NUM_FBS)) {
	*fbname = decode_fb[*fbtype];
	return 0;
    }

    *fbname = "unknown";
    return 1;
}
