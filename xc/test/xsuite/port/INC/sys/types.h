/* $XConsortium: types.h,v 1.2 92/06/12 09:37:27 rws Exp $ */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef _CLOCK_T
#define _CLOCK_T
typedef	long         	clock_t; 
#endif
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	long	swblk_t;
typedef	int	bool_t;
typedef	int	spl_t;

typedef long            dev_t;
typedef short		gid_t;
typedef	unsigned long   ino_t;
typedef	long		key_t;
#ifndef _MODE_T
#define _MODE_T
typedef unsigned short	mode_t;
#endif
typedef	short		nlink_t;
#ifndef _PID_T
#define _PID_T
typedef	int		pid_t;
#endif
typedef long            off_t;
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int    size_t;
#endif
#ifndef _TIME_T
#define _TIME_T
typedef	long         	time_t; 
#endif
typedef short		uid_t;

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned long	u_long;

#endif	/* _SYS_TYPES_H */
