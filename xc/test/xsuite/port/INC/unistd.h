/* $XConsortium: unistd.h,v 1.2 92/06/12 09:34:58 rws Exp $ */

/* We want to see if we have to define ENOSYS.. */
#include <errno.h>
#ifndef ENOSYS
#define ENOSYS -2
#endif

#ifndef _PID_T
#define _PID_T
	typedef	int 	pid_t;
#endif
#ifndef _MODE_T
#define _MODE_T
typedef int 	mode_t;
#endif

/* For F_OK etc */
/*#include	<sys/file.h>*/
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

/* For fcntl. Probably not the right place for this. */
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

/*
 * POSIX mode defines
 */
#define	S_IRWXU	0700
#define	S_IRWXG	0070
#define	S_IRWXO	0007
#define S_IRUSR	0400
#define S_IWUSR	0200
#define S_IXUSR	0100
#define S_IRGRP	0040
#define S_IWGRP	0020
#define S_IXGRP	0010
#define S_IROTH	0004
#define S_IWOTH	0002
#define S_IXOTH	0001

#define	SEEK_SET	0

#ifndef NULL
#define NULL	0
#endif

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 255
#endif

char	*getenv();
char *getlogin();
char	*getcwd();

