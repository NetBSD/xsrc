LIBDIR	= ../../lib/posix_c
INCDIR	= ../../inc/posix_c
CC	= cc
CFLAGS	= -I$(INCDIR)

all:	tc1 tc2 tc3

tc1:	tc1.c $(INCDIR)/tet_api.h
	$(CC) $(CFLAGS) -o tc1 tc1.c $(LIBDIR)/tcm.o $(LIBDIR)/libapi.a

tc2:	tc2.c $(INCDIR)/tet_api.h
	$(CC) $(CFLAGS) -o tc2 tc2.c $(LIBDIR)/tcm.o $(LIBDIR)/libapi.a

tc3:	tc3.c $(INCDIR)/tet_api.h
	$(CC) $(CFLAGS) -o tc3 tc3.c $(LIBDIR)/tcm.o $(LIBDIR)/libapi.a

