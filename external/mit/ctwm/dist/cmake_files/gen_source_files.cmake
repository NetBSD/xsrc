#
# Rules for generated source files
#


# Hand-build deftwmrc.c
set(defc ${CMAKE_CURRENT_BINARY_DIR}/deftwmrc.c)
set(mkdefc ${TOOLS}/mk_deftwmrc.sh)
add_custom_command(OUTPUT ${defc}
	DEPENDS system.ctwmrc ${mkdefc}
	COMMAND ${mkdefc} ${CMAKE_CURRENT_SOURCE_DIR}/system.ctwmrc > ${defc}
)


# Hand-build ctwm_atoms.[ch]
set(ctwm_atoms ctwm_atoms.h ctwm_atoms.c)
add_custom_command(OUTPUT ${ctwm_atoms}
	DEPENDS ctwm_atoms.in ${TOOLS}/mk_atoms.sh
	COMMAND ${TOOLS}/mk_atoms.sh ${CMAKE_CURRENT_SOURCE_DIR}/ctwm_atoms.in ctwm_atoms CTWM
)


# Generate up the event names lookup source
set(en_list ${CMAKE_CURRENT_SOURCE_DIR}/event_names.list)
set(en_out  ${CMAKE_CURRENT_BINARY_DIR}/event_names_table.h)
set(en_mk   ${TOOLS}/mk_event_names.sh)
add_custom_command(OUTPUT ${en_out}
	DEPENDS ${en_list} ${en_mk}
	COMMAND ${en_mk} ${en_list} > ${en_out}
)
# Have to manually add this, or cmake won't notice that it's needed in
# time to make it.
#set_source_files_properties(event_names.c OBJECT_DEPENDS ${en_out})
# This also seems a blessed hackaround, and avoids having to encode the
# knowledge of what files #include it.
list(APPEND CTWMSRC ${CMAKE_CURRENT_BINARY_DIR}/event_names_table.h)


# Create function bits
set(fd_list ${CMAKE_CURRENT_SOURCE_DIR}/functions_defs.list)
set(fd_mk   ${TOOLS}/mk_function_bits.sh)
set(fd_h    ${CMAKE_CURRENT_BINARY_DIR}/functions_defs.h)
set(fdd_h   ${CMAKE_CURRENT_BINARY_DIR}/functions_deferral.h)
set(fpt_h   ${CMAKE_CURRENT_BINARY_DIR}/functions_parse_table.h)
set(fde_h   ${CMAKE_CURRENT_BINARY_DIR}/functions_dispatch_execution.h)
add_custom_command(
	OUTPUT ${fd_h} ${fdd_h} ${fpt_h} ${fde_h}
	DEPENDS ${fd_list} ${fd_mk}
	COMMAND ${fd_mk} ${fd_list} ${CMAKE_CURRENT_BINARY_DIR}
)
list(APPEND CTWMSRC ${fd_h} ${fdd_h} ${fpt_h} ${fde_h})


# Setup config header file
configure_file(ctwm_config.h.in ctwm_config.h ESCAPE_QUOTES)


# Fill in version info

# Need the VCS bits; add a guard here to prevent dev screwups when
# working on the build system.
if(NOT VCS_CHECKS_RUN)
	message(FATAL_ERROR "Internal error: VCS checks not done yet!")
endif()

# Building the version.c file happens in 2 steps.  First, an original
# source file (${version_c_src}) is processed with tools/mk_version_in.sh
# to write in the ctwm version info found in the VERSION file in the root
# of the tree.  That's written to an intermediate file (${version_c_in}).
# Secondly, the intermediate file is processed to store up info about the
# VCS revision being built from, into the final file (${version_c}) which
# is actually compiled.
#
# A special case occurs when there's no VCS info we can find to work
# with, AND we have a pregen'd .in file (${vresion_c_gen}); this is
# commonly the case with a release tarball, where part of the release
# building process pre-stashes the bzr revision info.  In that case, we
# swap the original (${version_c_src}) to point to the pregen'd file
# (which already has the VCS info), and make the second step just cp the
# intermediate file over.
set(version_c_src ${CMAKE_CURRENT_SOURCE_DIR}/version.c.in)
set(version_c_in  ${CMAKE_CURRENT_BINARY_DIR}/version.c.in)
set(version_c     ${CMAKE_CURRENT_BINARY_DIR}/version.c)

# Maybe a pregen'd
set(version_c_gen ${GENSRCDIR}/version.c.in)

# Override
if(FORCE_PREGEN_FILES)
	set(HAS_BZR 0)
	set(HAS_GIT 0)
endif()

# Now, what's our process for going from the intermediate .c.in file to
# the final .c with the VCS revision info added?  This is the 2nd step in
# the process, but we describe it first here so we can change where
# ${version_c_src} points if necessary.
if(IS_BZR_CO AND HAS_BZR)
	# We're in a bzr tree, so write in the bzr revision info
	set(rw_ver_bzr "${TOOLS}/rewrite_version_bzr.sh")
	add_custom_command(OUTPUT ${version_c}
		DEPENDS ${version_c_in} ${BZR_DIRSTATE_FILE} ${rw_ver_bzr}
		COMMAND ${rw_ver_bzr} < ${version_c_in} > ${version_c}
		COMMENT "Generating version.c from current WT state."
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
elseif(IS_GIT_CO AND HAS_GIT)
	# We're in a git tree, so write in the git revision info
	set(rw_ver_git "${TOOLS}/rewrite_version_git.sh")
	add_custom_command(OUTPUT ${version_c}
		DEPENDS ${version_c_in} ${GIT_INDEX_FILE} ${rw_ver_git}
		COMMAND ${rw_ver_git} < ${version_c_in} > ${version_c}
		COMMENT "Generating version.c from current git state."
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
elseif(EXISTS ${version_c_gen})
	# There's a prebuilt one to use; commonly this means we're building
	# from a release tarball, and it includes pre-stashed bzr revision
	# info about the release.  So, in this case, we replace the original
	# source with our pregen'd one (which already has the info in it),
	# and just use 'cp' to turn the intermediate .c.in into the final .c.
	set(version_c_src ${version_c_gen})

	add_custom_command(OUTPUT ${version_c}
		DEPENDS ${version_c_in}
		COMMAND cp ${version_c_in} ${version_c}
		COMMENT "Using pregenerated version.c."
	)
else()
	# We've got nothing at all, so just NULL out the VCS info; it's all
	# we can do.
	add_custom_command(OUTPUT ${version_c}
		DEPENDS ${version_c_in}
		COMMAND sed -e 's/%%VCSTYPE%%/NULL/' -e 's/%%REVISION%%/NULL/'
			< ${version_c_in} > ${version_c}
		COMMENT "Using null version.c."
	)
endif()

# Now handle the first step; turning the original source .c.in into the
# intermediate .c.in with the ctwm version written in.  That's _usually_
# starting with the version.c.in in the source tree, but in the "no VCS
# info, but we have a pregen'd gen/version.c to use" case above it's the
# pregen'd file.
add_custom_command(OUTPUT ${version_c_in}
	DEPENDS ${version_c_src} ${CMAKE_CURRENT_SOURCE_DIR}/VERSION
	DEPENDS ${TOOLS}/mk_version_in.sh
	COMMAND ${TOOLS}/mk_version_in.sh ${version_c_src} > ${version_c_in}
	COMMENT "Writing version info into version.c.in"
)


# Setup a 'version' binary build tool too, for easily printing bits or
# wholes of our version.
#
# Not currently being used, but left as an example of possibilities.  If
# we need the version in the build process, we'll get it from the
# ${ctwm_version_*} vars now.
#add_executable(version ${version_c})
#set_target_properties(version PROPERTIES COMPILE_FLAGS "-DBUILD_VERSION_BIN")
