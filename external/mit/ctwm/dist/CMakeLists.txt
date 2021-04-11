cmake_minimum_required(VERSION 2.6)
project(ctwm C)

# Figure our version
file(READ "VERSION" vf_str)
string(STRIP ${vf_str} vf_str)
# Not currently using the split out variants
if(0)
	set(version_file_re "([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)")
	string(REGEX REPLACE ${version_file_re} "\\1" ctwm_version_major ${vf_str})
	string(REGEX REPLACE ${version_file_re} "\\2" ctwm_version_minor ${vf_str})
	string(REGEX REPLACE ${version_file_re} "\\3" ctwm_version_patch ${vf_str})
	string(REGEX REPLACE ${version_file_re} "\\4" ctwm_version_addl  ${vf_str})
	set(ctwm_version_str "${ctwm_version_major}.${ctwm_version_minor}.${ctwm_version_patch}${ctwm_version_addl}")
else()
	set(ctwm_version_str ${vf_str})
endif()

# Modules we'll need
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)

# Guard against in-tree builds
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insrc)
if(insrc)
	message(FATAL_ERROR "Please build out of tree; don't run cmake "
			"directly in the source tree.")
endif(insrc)


#
# Most of our bits are broken out into smaller files in a local dir
#
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_files)

# Setup basic vars for where our pieces are, list of source files, etc.
include(basic_vars)

# Do some basic checks of the compiler and stdlib
include(compiler_feature_checks)

# Set our install paths
include(install_paths)


#
# First things first.  If we don't have X, we're going nowhere.
#
find_package(X11)
if(NOT X11_FOUND)
	# This just isn't feasible...
	message(FATAL_ERROR "Can't find X libs.")
endif(NOT X11_FOUND)
if(NOT X11_INCLUDE_DIR)
	message(FATAL_ERROR "Can't find X includes.")
endif(NOT X11_INCLUDE_DIR)

include_directories(${X11_INCLUDE_DIR})

# Need to link in at least these; double check that we found 'em before
# blindly applying them.  We can seemingly get through the above just
# fine, even if it didn't find all the bits...
foreach(VEXT LIBRARIES Xmu_LIB Xt_LIB)
	if(NOT X11_${VEXT})
		message(FATAL_ERROR "Can't find X11_${VEXT}; missing lib or "
			"-devel package?")
	endif()
	list(APPEND CTWMLIBS ${X11_${VEXT}})
endforeach()


#
# Setup some search paths
#
set(INCSEARCH
	"${CMAKE_INSTALL_PREFIX}/include"
	${X11_INCLUDE_DIR}
	"/usr/local/include"
	"/usr/include"
)
set(LIBSEARCH
	"${CMAKE_INSTALL_PREFIX}/lib"
	${X11_LIBRARY_DIR}
	"/usr/local/lib"
	"/usr/lib"
	"/lib"
)

# Header files are in both source and build dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


#
# Look for various things on the system and setup our handlers for them,
# and put the build together.
#

# Check our build options and set things based on them
include(build_options)

# Do checks for library functions we need, and enable workarounds for any
# cases we decide are worth handling that way.
include(check_funcs_etc)

# If we're building out of a VCS, find those bits so we can stash the
# revision info.
include(vcs_checks)

# Find some tools used for generating sources and manuals, and setup the
# targets to build them all.
include(setup_lex)
include(setup_yacc)
include(handle_manual)
include(gen_source_files)

# Targets to run doxygen and ctags; nobody but devs care
include(doxygen)
include(ctags)


# And link up the actual ctwm binary.  We actually build a libctwmlib
# with all our contents except the trivial main() wrapper, then build
# ctwm with that; this makes it easy to build other binaries (like tests)
# with access to all our internal funcs.
add_library(ctwmlib STATIC ${CTWMSRC})
add_executable(ctwm "ctwm_wrap.c")

# Keep ctwmlib before the shared libs.  With some ld(1)'s and build
# environments that set --as-needed, having it after means that ld will
# have already discarded all the shared libs as "unused" before it gets
# to all the actual ctwm objects that need the symbols.
target_link_libraries(ctwm ctwmlib)
target_link_libraries(ctwm ${CTWMLIBS})

# This doesn't really serve much purpose at the moment, so it's not even
# documented, but the code exists.  So make it buildable.
if(DO_CLIENT)
	add_subdirectory(client)
endif(DO_CLIENT)


# Setup the installation
include(do_install)


#
# And some trailing misc bits
#

# Pull in some CPack config for auto-building packages (like .deb and
# .rpm)
include(cpack_setup)

# Some targets to support release management stuff; building generated
# files for the tarballs
include(mktar_support)

# Pull in dtrace bits
include(dtrace_support)

# Let cmake write out compile commands for external tools.  Requires
# cmake 3.5+, but setting an unknown var won't hurt earlier versions.
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Include tests
include(CTest)
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()

# Finish by outputting various information about what we've figured and
# what we're doing for the builder's edification.
include(show_build_info)
