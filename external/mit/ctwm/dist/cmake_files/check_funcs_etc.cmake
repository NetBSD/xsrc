#
# Check for library functions and the like that we need.
#
# In some cases, we may decide to enable workarounds of various sorts
# here too; in others, we just bomb out.  Technically, doing that is
# unnecessary, since the build will just fail later, but this may be
# friendlier in telling the user _why_ than letting them try to interpret
# compiler or linker errors.
#
# This is distinct from the checking in build_options.cmake; that's about
# stuff we have build options to enable/disable, whereas this is more
# stuff we depend on unconditionally.
#


# getopt_long(3) is in getopt.h everywhere I can find it.  Until we find
# a system where it's somewhere else, or systems it's nowhere and we have
# to provide it, we'll either find it there or error out and blow up.  If
# we don't find problems that require additional workaround in the next
# couple releases, we should retire the check to avoid wasting time/space
# checking things we know will work.
check_include_files(getopt.h HAS_GETOPT_H)
check_function_exists(getopt_long HAS_GETOPT_LONG)
if(NOT HAS_GETOPT_H)
	message(FATAL_ERROR "Can't find getopt.h (needed for getopt_long()).")
endif(NOT HAS_GETOPT_H)
if(NOT HAS_GETOPT_LONG)
	message(FATAL_ERROR "You don't seem to have getopt_long().")
endif(NOT HAS_GETOPT_LONG)



# asprintf(3) has been in glibc since at least the early 90's, the BSD's
# since the mid 90's, and [Open]Solaris since 11.  It's not in Sol10, at
# least some versions of AIX, etc.  There's a version in openssh-portable
# that I believe is pretty portable if we find a system we care about
# lacking it and need to pull in a local version, but I don't expect to.
check_function_exists(asprintf HAS_ASPRINTF)
if(NOT HAS_ASPRINTF)
	message(FATAL_ERROR "You don't seem to have asprintf(3).")
endif(NOT HAS_ASPRINTF)
