#
# See what sort of version control bits we might have around.
#


# See if we're building from a bzr checkout.  This is fragile in the
# sense that it'll break if the bzr WT format changes, but that's
# staggeringly unlikely now, so...
set(BZR_DIRSTATE_FILE ${CMAKE_SOURCE_DIR}/.bzr/checkout/dirstate)
if(EXISTS ${BZR_DIRSTATE_FILE})
	set(IS_BZR_CO 1)
else()
	set(IS_BZR_CO 0)
endif()


# If we are, see if we can find bzr(1) installed
set(HAS_BZR 0)
if(IS_BZR_CO)
	find_program(BZR_CMD bzr)
	if(BZR_CMD)
		set(HAS_BZR 1)
		message(STATUS "Building from a checkout and found bzr.")
	else()
		message(STATUS "Building from a checkout, but no bzr found.")
	endif(BZR_CMD)
else()
	message(STATUS "You aren't building from a bzr checkout.")
endif(IS_BZR_CO)


# If not bzr, do a little check to see if we're building from git instead
if(NOT IS_BZR_CO)
	set(GIT_INDEX_FILE ${CMAKE_SOURCE_DIR}/.git/index)
	set(IS_GIT_CO 0)
	if(EXISTS ${GIT_INDEX_FILE})
		set(IS_GIT_CO 1)
	endif()

	if(IS_GIT_CO)
		set(HAS_GIT 0)
		find_program(GIT_CMD git)
		if(GIT_CMD)
			set(HAS_GIT 1)
			message(STATUS "Building from git repo and found git.")
		else()
			message(STATUS "Building from git repo, but no git found.")
		endif(GIT_CMD)
	endif(IS_GIT_CO)
endif(NOT IS_BZR_CO)


# Flag for dev use
set(VCS_CHECKS_RUN 1)
