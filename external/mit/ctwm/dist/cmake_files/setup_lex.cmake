#
# Setup flex to build lexer
#
# We don't really support any lex(1) other than flex.  It's possible some
# minor editing of lex.l could get you through that...
#
# If you DO have flex, we go ahead and use it to build the lexer.  If you
# don't, we check to see if there's a prebuilt one in the tree; one will
# be shipped with tarballs for releases etc (but you'll still build your
# own if you have flex).  If neither of those hit, not much we can do but
# bomb...

# Override for forcing use of pregen'd source files
if(NOT FORCE_PREGEN_FILES)
	find_package(FLEX)
endif()


if(FLEX_FOUND)
	FLEX_TARGET(ctwm_lexer lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.c)
else()
	# See if we have a pre-built lex.c
	find_file(LEX_C lex.c
		PATHS ${GENSRCDIR}
		NO_DEFAULT_PATH)
	if(LEX_C)
		# Make the build process just copy it in
		message(STATUS "No flex found, using prebuilt lex.c")
		add_custom_command(OUTPUT lex.c
			DEPENDS ${LEX_C}
			COMMAND cp ${LEX_C} .
			COMMENT "Copying in prebuilt lex.c"
		)
	else()
		# No flex, no pre-built lex.c
		message(FATAL_ERROR "Can't find flex, and no prebuilt files "
			"available.")
	endif(LEX_C)
endif(FLEX_FOUND)
