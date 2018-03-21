#	$NetBSD: Makefile,v 1.3 2018/03/21 16:24:34 martin Exp $
#	@(#)Makefile	8.1 (Berkeley) 5/31/93

PROG=	ttf2wsfont

SRCS=	main.c
WARNS=	3
NOMAN=

CPPFLAGS+=	-DXFREE86_FT2 -I${DESTDIR}${X11INCDIR}/freetype2 -D_NETBSD_SOURCE

LDADD+=	-lfreetype

.include <bsd.x11.mk>
.include <bsd.prog.mk>
