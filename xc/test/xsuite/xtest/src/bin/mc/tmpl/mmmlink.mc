# $XConsortium: mmmlink.mc,v 1.3 94/01/29 16:24:16 rws Exp $
# A version of the test that can be combined with all the other tests for
# the macro version of the function.
#
mlink.c: $(SOURCES)
	$(CODEMAKER) -m -l -o mlink.c $(SOURCES)

mlink.o: $(DEPHEADERS)

linkexec:: m$(LINKEXEC) $(AUXFILES) ;

m$(LINKEXEC): ../Tests
	$(RM) m$(LINKEXEC)
	$(LN) ../Tests m$(LINKEXEC)
