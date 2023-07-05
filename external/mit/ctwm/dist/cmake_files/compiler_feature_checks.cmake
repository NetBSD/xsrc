#
# Compiler/stdlib feature checks for ctwm
#


# Expect and try to enforce a C99 capable compiler.  There doesn't seem
# an obvious way to be sure in a fully portable way, but we probably
# don't work well in places that compile with something other than a
# program called like 'cc', and a cc that supports C99 should accept -std
# calls, so that's good enough.  Lacking it is not (yet) a fatal error,
# but is a sign that it's a compiler or platform we're moving further
# away from.
#
# cmake 3.1+ has C_STANDARD and related vars that seem like they'd help
# with this, but it's unclear that they actually solve the whole
# problem...
#
# Known alternate spellings:
#   -xc99  (Sun C 5.10 SunOS_i386, sunstudio12.1, OpenIndiana)
include(CheckCCompilerFlag)
set(MANUAL_C_STD_FLAG true)
if(NOT MANUAL_C_STD_FLAG)
	# This is the Better Way(tm), but is disabled by default because, as
	# with the manual one below, the added arg doesn't apply in
	# check_symbol_exists(), so it screws up the tests below.  I'm unable
	# to find a way to get info from cmake about what arg it would add
	# for the specified standard, so we can't pull it out manually to add
	# like we do our found C99_FLAG below, so...
	if(NOT "c_std_99" IN_LIST CMAKE_C_COMPILE_FEATURES)
		message(WARNING "cmake doesn't know about c99 support for this "
			"compiler, trying manual search...")
		set(MANUAL_C_STD_FLAG true)
	else()
		message(STATUS "Enabling C99 mode")
		set(CMAKE_C_EXTENSIONS false)
		set(CMAKE_C_STANDARD 99)
	endif()
endif()
if(MANUAL_C_STD_FLAG)
	set(c99_flag_options -std=c99 -xc99)
	foreach(_C99_FLAG ${c99_flag_options})
		# CheckCCompilerFlag calls into CheckCSourceCompiles, which won't do
		# anything if the result var is already set in the cache, so we have
		# to unset it.  Otherwise, the second and later invocations don't
		# actually do anything, and it'll never check any flag after the
		# first.
		unset(COMPILER_C99_FLAG CACHE)
		check_c_compiler_flag(${_C99_FLAG} COMPILER_C99_FLAG)
		if(COMPILER_C99_FLAG)
			set(C99_FLAG ${_C99_FLAG})
			break()
		endif(COMPILER_C99_FLAG)
	endforeach(_C99_FLAG)
	if(C99_FLAG)
		message(STATUS "Enabling C99 flag: ${C99_FLAG}")
		add_definitions(${C99_FLAG})
	else()
		message(WARNING "Compiler doesn't support known C99 flag, "
				"building without it.")
	endif(C99_FLAG)
endif()



# With -std=c99, some systems/compilers/etc enable ANSI strictness, and
# so don't export symbols from headers for e.g. a lot of POSIX etc stuff.
# So we may need some extra -D's.
# Some refs:
# https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
# https://illumos.org/man/5/standards

# Somewhat irritatingly, check_symbol_exists() doesn't use the extra
# flags we set above for the C standard.  So we have to add it manually,
# and stash up the old C_R_D.  x-ref above.
set(OLD_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
list(APPEND CMAKE_REQUIRED_DEFINITIONS ${C99_FLAG})

# What might and will we add?
set(AVAIL_SYM_FLAGS -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700
	-D_GNU_SOURCE -D__EXTENSIONS__)
set(EXTRA_SYM_FLAGS "")

# Abstract
macro(_check_func_flag FUNC HEADER)
	unset(_HAS_FUNC CACHE)
	check_symbol_exists(${FUNC} ${HEADER} _HAS_FUNC)
	if(NOT _HAS_FUNC)
		foreach(_SFLAG ${AVAIL_SYM_FLAGS})
			unset(_HAS_FUNC CACHE)
			list(APPEND CMAKE_REQUIRED_DEFINITIONS ${_SFLAG})
			check_symbol_exists(${FUNC} ${HEADER} _HAS_FUNC)
			list(REMOVE_ITEM CMAKE_REQUIRED_DEFINITIONS ${_SFLAG})
			if(_HAS_FUNC)
				message(STATUS "${FUNC}() needs ${_SFLAG}")
				list(APPEND EXTRA_SYM_FLAGS ${_SFLAG})
				break()
			endif()
		endforeach()
		if(NOT _HAS_FUNC)
			message(WARNING "Couldn't find def for ${FUNC}, not good...")
		endif()
	endif()
	unset(_HAS_FUNC CACHE)
endmacro(_check_func_flag)

# strdup is POSIX, so see if we have to ask for that.  Probably
# _POSIX_C_SOURCE.
_check_func_flag(strdup string.h)

# isascii falls into XOPEN on glibc, POSIX on Illumos.
_check_func_flag(isascii ctype.h)

# asprintf() is even weirder.  glibc apparently usually uses _GNU_SOURCE,
# Illumos has a pure __EXTENSIONS__
_check_func_flag(asprintf stdio.h)

if(EXTRA_SYM_FLAGS)
	list(REMOVE_DUPLICATES EXTRA_SYM_FLAGS)
	message(STATUS "Adding extra visibility flags: ${EXTRA_SYM_FLAGS}")
	add_definitions(${EXTRA_SYM_FLAGS})
endif()

# And restore
set(CMAKE_REQUIRED_DEFINITIONS ${OLD_CMAKE_REQUIRED_DEFINITIONS})




# Some compilers (like Sun's) don't take -W flags for warnings.  Do a
# quick check with -Wall.  They're mostly for devs, so we don't care THAT
# much, I guess...  maybe we should be more thorough about checking the
# flags we use too, but worry about what when it becomes an issue.
check_c_compiler_flag("-Wall" COMPILER_TAKES_WALL)
if(NOT COMPILER_TAKES_WALL)
	message(STATUS "Compiler doesn't like -Wall, disabling warnings.")
	set(NO_WARNS 1)
endif(NOT COMPILER_TAKES_WALL)

if(NOT NO_WARNS)
	add_definitions(${STD_WARNS})
	message(STATUS "Enabling standard warnings.")
endif(NOT NO_WARNS)
