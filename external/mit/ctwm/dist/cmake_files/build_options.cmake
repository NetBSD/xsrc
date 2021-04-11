#
# Our build-time options
#


#
# Define options
# These can be set at the command line; e.g., "cmake -DUSE_JPEG=OFF"
#
option(USE_XPM    "Enable XPM support"                 ON )
option(USE_JPEG   "Enable libjpeg support"             ON )
option(USE_M4     "Enable m4 support"                  ON )
option(USE_RPLAY  "Enable librplay sound support"      OFF)
option(USE_SREGEX "Use regex from libc"                ON )
option(USE_EWMH   "Support some Extended Window Manager Hints"  ON )



#
# Now check what's set, make sure we can find stuff, and configure bits
# up.
#

# Hard to imagine xpm not being around or somebody not wanting it, but...
if(USE_XPM)
	if(NOT X11_Xpm_FOUND)
		message(FATAL_ERROR "Couldn't find XPM libs")
	endif(NOT X11_Xpm_FOUND)

	list(APPEND CTWMLIBS ${X11_Xpm_LIB})
	list(APPEND CTWMSRC image_xpm.c)
	message(STATUS "Enabling XPM support: ${X11_Xpm_LIB}")

	# DATADIR should already be defined; guard against me being stupid
	# when I change something
	if(NOT DATADIR)
		message(FATAL_ERROR "Internal error: DATADIR not defined!")
	endif(NOT DATADIR)
endif(USE_XPM)


# libjpeg is pretty common
if(USE_JPEG)
	find_package(JPEG)
	if(NOT JPEG_FOUND)
		message(FATAL_ERROR "Couldn't find libjpeg")
	endif()

	include_directories(${JPEG_INCLUDE_DIR})
	list(APPEND CTWMLIBS ${JPEG_LIBRARIES})
	list(APPEND CTWMSRC image_jpeg.c)
	message(STATUS "Enabling libjpeg support.")
endif(USE_JPEG)


# m4 is on by default too
if(USE_M4)
	if(NOT M4_CMD)
		find_program(M4_CMD m4 gm4)
	endif(NOT M4_CMD)
	if(NOT M4_CMD)
		message(FATAL_ERROR "Can't find m4 program: try setting M4_CMD.")
	endif(NOT M4_CMD)
	list(APPEND CTWMSRC parse_m4.c)
	message(STATUS "Enabling m4 support (${M4_CMD}).")
endif(USE_M4)


# rplay off by default
if(USE_SOUND)
	message(WARNING "USE_SOUND is deprecated; use USE_RPLAY instead.")
	set(USE_RPLAY YES)
endif(USE_SOUND)
if(USE_RPLAY)
	find_library(LIBRPLAY NAMES rplay PATHS ${LIBSEARCH})
	if(NOT LIBRPLAY)
		message(FATAL_ERROR "Can't find librplay lib.")
	endif(NOT LIBRPLAY)
	find_path(LIBRPLAY_INCLUDE_DIR NAME rplay.h PATHS ${INCSEARCH})
	if(NOT LIBRPLAY_INCLUDE_DIR)
		message(FATAL_ERROR "Can't find rplay.h.")
	endif(NOT LIBRPLAY_INCLUDE_DIR)

	list(APPEND CTWMSRC sound.c)
	list(APPEND CTWMLIBS ${LIBRPLAY})
	include_directories(${LIBRPLAY_INCLUDE_DIR})
	message(STATUS "Enabling librplay sound support.")
endif(USE_RPLAY)


# Check if the user wants EWMH support built in.
if(USE_EWMH)
    # Hand-build ewmh_atoms.[ch]
	set(ewmh_atoms ewmh_atoms.h ewmh_atoms.c)
	add_custom_command(OUTPUT ${ewmh_atoms}
		DEPENDS ewmh_atoms.in ${TOOLS}/mk_atoms.sh
		COMMAND ${TOOLS}/mk_atoms.sh ${CMAKE_CURRENT_SOURCE_DIR}/ewmh_atoms.in ewmh_atoms EWMH
    )

	list(APPEND CTWMSRC ewmh.c ewmh_atoms.c)
	message(STATUS "Enabling Extended Window Manager Hints support.")
else()
	message(STATUS "Disabling Extended Window Manager Hints support.")
endif(USE_EWMH)


# System provides regex stuff in libc?
if(USE_SREGEX)
	check_include_files(regex.h HAS_REGEX_H)
	check_function_exists(regexec HAS_REGEXEC)

	if(NOT HAS_REGEX_H)
		message(FATAL_ERROR "Can't find regex.h")
	endif(NOT HAS_REGEX_H)
	if(NOT HAS_REGEXEC)
		message(FATAL_ERROR "Can't find regexec()")
	endif(NOT HAS_REGEXEC)

	message(STATUS "Enabling libc regex usage.")
else()
	message(FATAL_ERROR "USE_SREGEX=OFF no longer supported.")
endif(USE_SREGEX)
