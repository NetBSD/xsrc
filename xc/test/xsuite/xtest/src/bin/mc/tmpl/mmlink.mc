# $XConsortium: mmlink.mc,v 1.3 94/01/29 16:24:43 rws Exp $
#
# A version of the test that can be combined together with
# all the other tests to make one executable.  This will save a
# fair amount of disk space especially if the system does not
# have shared libraries.  Different names are used so that
# there is no possibility of confusion.
#
link.c: $(SOURCES)
	$(CODEMAKER) -l -o link.c $(SOURCES)

link.o: $(DEPHEADERS)

# Link the objects into one large object.
#
$(LINKOBJ): $(LOFILES)
	$(LD) $(LINKOBJOPTS) $(LOFILES) -o $(LINKOBJ)

# Link the object file into the parent directory.
#
../$(LINKOBJ): $(LINKOBJ)
	$(RM) ../$(LINKOBJ)
	$(LN) $(LINKOBJ) ..

# Make a link to the combined executable.
#
$(LINKEXEC): ../Tests
	$(RM) $(LINKEXEC)
	$(LN) ../Tests $(LINKEXEC)

../Tests: ../$(LINKOBJ)

linkexec:: $(LINKEXEC) $(AUXFILES) ;

