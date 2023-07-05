#
# Automated tests
#

set(CTWMBIN "ctwm")
set(CTWMPATH "${CMAKE_BINARY_DIR}/ctwm")
add_custom_target(test_bins)
add_dependencies(test_bins ctwm)


# Add some infrastructure for building executables for unit tests
#
# BIN - name of the binary to run.
# BINPATH - path of the binary to run.
# ARGS - Add'l arguments to the binary for the test
# WORKING_DIRECTORY - self-explanatory
#
# Currently, ${BINPATH} means a thing is already built, so we don't need
# to worry about building, that's just the path to run.  Else, ${BIN} is
# built from ${BIN}.c (though multiple calls with the same ${BIN} won't
# try to re-do the builds).  e.g., commonly, $BINPATH is used when we're
# running the main ctwm binary, instead of s test-specific bin.
#
# This is not un-janky, but it works, and we'll worry about cleaning it
# up if it gets worse.
function(ctwm_simple_unit_test TNAME)
	set(_opts ALLOW_DISPLAY)
	set(_arg_one BIN BINPATH WORKING_DIRECTORY)
	set(_arg_multi ARGS)
	cmake_parse_arguments(_ARGS "${_opts}" "${_arg_one}" "${_arg_multi}"
		${ARGN})

	# Maybe we're passed a specific path to run
	if(_ARGS_BINPATH)
		set(BIN_RUN "${_ARGS_BINPATH}")
	endif()

	# What's the binary name?
	if(_ARGS_BIN)
		set(BIN "${_ARGS_BIN}")
	else()
		set(BIN "${TNAME}")
	endif()

	# Derive path if we only got a name
	if(NOT BIN_RUN)
		set(BIN_RUN $<TARGET_FILE:${BIN}>)
	endif()

	# Building and linking it
	if(NOT TARGET ${BIN})
		add_executable(${BIN} EXCLUDE_FROM_ALL "${BIN}.c")
		target_sources(${BIN} PUBLIC $<TARGET_OBJECTS:ctwmlib>)

		# Few of our tests really need any of the X or other libs we pull in.
		# However, most of the .o's for ctwm itself are going to have some
		# ref out to them somewhere, so any tests that wind up pulling
		# functions from those are going to make the linker want to resolve
		# them when we link ${TNAME}.  So just unconditionally add them and
		# don't worry about which tests may not actually need 'em.
		target_link_libraries(${BIN} ${CTWMLIBS})

		# Add to pre-test target
		add_dependencies(test_bins ${BIN})
	endif()

	# And add the test itself
	add_test(NAME ${TNAME}
		COMMAND ${BIN_RUN} ${_ARGS_ARGS}
		WORKING_DIRECTORY ${_ARGS_WORKING_DIRECTORY}
		)

	# Don't allow $DISPLAY unless specifically requested.
	if(NOT _ARGS_ALLOW_DISPLAY)
		set_tests_properties(${TNAME} PROPERTIES
			ENVIRONMENT DISPLAY=
			)
	endif()

endfunction()


# Simple wrapper for when we want to explicitly skip a test (as opposed
# to just silently not doing it; better UX to say we're skipping one and
# why...)
#
# XXX This seems like the simplest way of actually "skip"'ing a test
# based on configure options.  That's nuts.  REQUIRED_FILES sounds like
# it would, but actually causes the test to "not run" and be considered
# failed   :(
function(ctwm_skip_unit_test TNAME REASON)
	add_test(NAME ${TNAME} COMMAND sh -c "echo Skipped: ${REASON} ; exit 99")
	set_tests_properties(${TNAME} PROPERTIES SKIP_RETURN_CODE 99)
endfunction()



# First a simple smoke test of the built binary
ctwm_simple_unit_test(run-info
	BIN ${CTWMBIN}
	BINPATH ${CTWMPATH}
	ARGS --info
	)
set_tests_properties(run-info PROPERTIES
	PASS_REGULAR_EXPRESSION "Twm version: "
	)


# Try parsing the system.ctwmrc
ctwm_simple_unit_test(cfgchk
	BIN ${CTWMBIN}
	BINPATH ${CTWMPATH}
	ARGS --cfgchk -f ${CMAKE_SOURCE_DIR}/system.ctwmrc
	)


# Simple test of m4 preprocessing, but we skip if built without m4.
if(USE_M4)
	add_subdirectory(test_m4)
else()
	ctwm_skip_unit_test(test_m4 "Built without USE_M4")
endif()


# A first run at a unit test
add_subdirectory(util_expand)

# RLayout stuff
add_subdirectory(layout)

# TwmKeys menu bits
add_subdirectory(menu_twmkeys)
