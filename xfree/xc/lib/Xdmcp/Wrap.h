/* $XConsortium: Wrap.h,v 1.3 94/02/06 20:09:49 rws Exp $ */
/*
 * header file for compatibility with something useful
 */

/* $XFree86$ */

typedef unsigned char auth_cblock[8];	/* block size */

typedef struct auth_ks_struct { auth_cblock _; } auth_wrapper_schedule[16];

extern void _XdmcpWrapperToOddParity (unsigned char *in, unsigned char *out);
