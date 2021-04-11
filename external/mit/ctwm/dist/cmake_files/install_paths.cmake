#
# Figure out install paths for stuff
#

# CMAKE_INSTALL_PREFIX is the user's responsibility to set right.  We
# just assume it is.


# Where ctwm looks for fallback config files
if(NOT ETCDIR)
	set(ETCDIR ${CMAKE_INSTALL_PREFIX}/etc)
endif(NOT ETCDIR)


# Where we stick the binary
if(NOT BINDIR)
	set(BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
endif(NOT BINDIR)


# Where runtime data stuff (e.g., images) is found
if(NOT DATADIR)
	set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ctwm)
endif(NOT DATADIR)


# Where we install non-manpage docs to
if(NOT DOCDIR)
	set(DOCDIR ${CMAKE_INSTALL_PREFIX}/share/doc/ctwm)
endif(NOT DOCDIR)


# Where we put example configs etc.
if(NOT EXAMPLEDIR)
	set(EXAMPLEDIR ${CMAKE_INSTALL_PREFIX}/share/examples/ctwm)
endif(NOT EXAMPLEDIR)


# Where should we stick manpages?
# User can set MANDIR to override, if they don't, check a few likely
# places...
if(NOT MANDIR)
	find_file(MANDIR man
		PATHS
			"${CMAKE_INSTALL_PREFIX}/share"
			"${CMAKE_INSTALL_PREFIX}"
		NO_DEFAULT_PATH)
endif(NOT MANDIR)

# If we can't find something, hardcode a guess and yell about it
if(NOT MANDIR)
	set(MANDIR "${CMAKE_INSTALL_PREFIX}/man")
	message(WARNING "Can't find install base for manpages, "
			"assuming ${MANDIR}")
endif(NOT MANDIR)

# Since we only have 1 manpage, and it's in section 1, just list the man1
# dir.
set(MAN1PATH "${MANDIR}/man1")

# This is fixed too
set(PIXMAPDIR ${DATADIR}/images)
