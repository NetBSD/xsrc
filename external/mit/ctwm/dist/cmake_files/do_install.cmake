#
# Define the install process
#

# Some bits we always install; the binary, the default config, and our
# CHANGES file.
install(TARGETS ctwm
	DESTINATION ${BINDIR}
)
install(FILES system.ctwmrc
	DESTINATION ${EXAMPLEDIR}
)
install(FILES README.md CHANGES.md
	DESTINATION ${DOCDIR}
)


# If we's using XPM (really, when are we not?), install the pixmaps.
if(USE_XPM)
	install(DIRECTORY xpm/
		DESTINATION ${PIXMAPDIR}
		FILES_MATCHING PATTERN "*.xpm"
	)
endif(USE_XPM)


#
# Install manual bits, assuming we have them.
#

# If we don't have the manpage, that's pretty exceptional, so give a
# warning about it.
if(NOT HAS_MAN)
	# STRING(CONCAT x y z) could build this message, but requires cmake
	# 3.0 which we aren't yet willing to require.
	install(CODE "message(WARNING \"No manpage to install: recheck config if this is unexpected.\")")
else()
	install(FILES ${INSTMAN}
		DESTINATION ${MAN1PATH}
	)
endif(NOT HAS_MAN)

# ATM, the HTML manual is more optionalish
if(INSTHTML)
	install(FILES ${INSTHTML}
		DESTINATION ${DOCDIR}
	)
endif(INSTHTML)
