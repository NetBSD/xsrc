# Setup doxygen stuff.  Only really of interest to devs...

find_package(Doxygen)
if(DOXYGEN_FOUND)
	# Few configurable params
	if(NOT DOXYGEN_DIR)
		set(DOXYGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
	endif()
	if(NOT DOXYGEN_HAVE_DOT)
		# String YES/NO; let user override what find_package() got.
		set(DOXYGEN_HAVE_DOT ${DOXYGEN_DOT_FOUND})
	endif()
	if(NOT DOXYGEN_GRAPHIC_CALLGRAPHS)
		# String YES/NO.  These are expensive to generate and big.
		set(DOXYGEN_GRAPHIC_CALLGRAPHS "NO")
	endif()

	# The config codes the paths to the files, so we need to set them
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
		"${DOXYGEN_DIR}/Doxyfile" @ONLY)

	# Special target, since it hardly ever gets used.  We need to pull in
	# the various generated source files, and the easiest way to ensure
	# they're all these is to just build ctwm.  That gets a little
	# tedious when working on docs though, so try depending on CTWMSRC.
	# That seems to work well enough, and if we find edge cases that
	# break it...   well, it's a tool for devs, not end users, so we can
	# afford the possibility of surprises.
	add_custom_target(doxygen
		DEPENDS ${CTWMSRC}
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_DIR}/Doxyfile
		WORKING_DIRECTORY ${DOXYGEN_DIR}
		COMMENT "Generating Doxygen documentation in ${DOXYGEN_DIR}"
		VERBATIM)

	add_custom_target(doxyclean
		COMMAND rm -rf ${DOXYGEN_DIR}/html
		COMMENT "Cleaning up Doxygen docs")
endif(DOXYGEN_FOUND)
