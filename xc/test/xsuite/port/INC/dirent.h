/* $XConsortium: dirent.h,v 1.1 92/06/11 15:30:11 rws Exp $ */
/* POSIX dirent.h compatibility. */

#include <sys/dir.h>


/* This is GROSS! BSD uses direct, POSIX uses dirent. */
#define dirent direct
