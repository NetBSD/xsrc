# $XConsortium: mmxplib.mc,v 1.2 92/06/11 19:34:31 rws Exp $
# 
# This part of the makefile checks for the existance of the libraries
# and creates them if necessary.
#

# The xtestlib is made if it doesn't exist
#
$(XTESTLIB):
	cd $(XTESTROOT)/src/lib; $(TET_BUILD_TOOL) install

# The fontlib is made if it doesn't exist
#
$(XTESTFONTLIB):
	cd $(XTESTROOT)/fonts; $(TET_BUILD_TOOL) install

# The X Protocol test library is made if it doesn't exist
#
$(XSTLIB):
	cd $(XTESTROOT)/src/libproto; $(TET_BUILD_TOOL) install

