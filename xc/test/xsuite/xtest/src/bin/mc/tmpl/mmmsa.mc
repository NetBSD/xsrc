# $XConsortium: mmmsa.mc,v 1.3 94/01/29 16:23:51 rws Exp $
#
# Build a standalone version of the test case using the macro version
# of the function.
#
MTest: $(MOFILES) $(LIBS) $(TCM) $(AUXFILES)
	$(CC) $(LDFLAGS) -o $@ $(MOFILES) $(TCM) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

MTest.c: $(SOURCES)
	$(CODEMAKER) -m -o MTest.c $(SOURCES)

MTest.o: $(DEPHEADERS)
