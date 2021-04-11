# Setup tagets to run ctags.  Only really of interest to devs...

find_program(CTAGS NAMES uctags exctags ctags)
if(CTAGS)
	message(STATUS "Found ctags: ${CTAGS}")
	add_custom_target(tags
		COMMAND ${CTAGS} *.[ch] ext/*.[ch] build/*.[ch]
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
endif()
