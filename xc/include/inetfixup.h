/* $XFree86: xc/include/inetfixup.h,v 1.1.2.1 1999/07/17 05:00:47 dawes Exp $ */
/*
 * The sole purpose of this is to undo the #defines for inet_* in
 * the FreeBSD/ELF <arpa/inet.h> when building a.out compatibility versions
 * of the X libraries.
 */

#if defined(__FreeBSD__) && defined(AOUT_COMPAT_LIB)
#undef inet_addr
#undef inet_aton
#undef inet_lnaof
#undef inet_makeaddr
#undef inet_neta
#undef inet_netof
#undef inet_network
#undef inet_net_ntop
#undef inet_net_pton
#undef inet_ntoa
#undef inet_pton
#undef inet_ntop
#undef inet_nsap_addr
#undef inet_nsap_ntoa
#endif

