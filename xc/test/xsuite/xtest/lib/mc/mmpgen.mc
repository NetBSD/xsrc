# $XConsortium: mmpgen.mc,v 1.2 92/06/29 19:42:21 rws Exp $
#
# Pixel generation makerules for generating the reference
# known good image files.
#

PVOFILES=pvtest.o

pvgen: $(PVOFILES) $(PVLIBS) $(TCM)
	$(CC) $(LDFLAGS) -o $@ $(PVOFILES) $(TCM) \
	$(PVLIBS) $(SYSLIBS) $(SYSMATHLIB)

pvtest.o: pvtest.c
	cc -c -DGENERATE_PIXMAPS $(CFLAGS) pvtest.c

pvtest.c: Test.c
	$(RM) pvtest.c; \
	$(LN) Test.c pvtest.c

