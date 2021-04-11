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
# Known alternate spellings:
#   -xc99  (Sun C 5.10 SunOS_i386, sunstudio12.1, OpenIndiana)
include(CheckCCompilerFlag)
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


# With -std=c99, GNU libc's includes get strict about what they export.
# Particularly, a lot of POSIX stuff doesn't get defined unless we
# explicitly ask for it.  Do our best at checking for what's there...
check_include_files(features.h HAS_FEATURES_H)
if(HAS_FEATURES_H)
	# Check if including it with our args sets __USE_ISOC99; that's a
	# sign it's what we're looking for here.
	check_symbol_exists(__USE_ISOC99 features.h SETS_USE_ISOC99)
	if(SETS_USE_ISOC99)
		# OK, it does.  Assume that's a good enough test that things are
		# acting as we expect.
		set(GLIBC_FEATURE_FLAGS
			"-D_POSIX_C_SOURCE=200809L"
			"-D_XOPEN_SOURCE=700"
			)
		# asprintf() seems to need _GNU_SOURCE; no other way to expose it
		# I can find.
		check_symbol_exists(__GNU_LIBRARY__ features.h SETS_GNU_LIBRARY)
		if(SETS_GNU_LIBRARY)
			list(APPEND GLIBC_FEATURE_FLAGS "-D_GNU_SOURCE")
		endif(SETS_GNU_LIBRARY)

		message(STATUS "Enabling glibc feature macros: ${GLIBC_FEATURE_FLAGS}")
		add_definitions(${GLIBC_FEATURE_FLAGS})
	endif()
endif(HAS_FEATURES_H)


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
