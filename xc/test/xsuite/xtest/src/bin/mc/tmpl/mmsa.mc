# $XConsortium: mmsa.mc,v 1.3 94/01/29 16:24:30 rws Exp $
#
# Build a standalone version of the test case.
#
Test: $(OFILES) $(LIBS) $(TCM) $(AUXFILES)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(TCM) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

Test.c: $(SOURCES)
	$(CODEMAKER) -o Test.c $(SOURCES)

Test.o: $(DEPHEADERS)
