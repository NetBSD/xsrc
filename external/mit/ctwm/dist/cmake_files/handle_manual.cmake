#
# Handle stuff related to building the manual in various ways
#


# Lookup the asciidoc[tor] bits
include(find_asciidoc_bits)



#
# Setup some vars for the various steps in the process
#

# The original source.
set(ADOC_SRC ${MANSRCDIR}/ctwm.1.adoc)

# Where we build stuff.  Because we need to process the ADOC_SRC to
# replace build paths etc, we need to dump it somewhere.  We could just
# leave it right in the build dir root, but is cleaner.
set(MAN_TMPDIR ${CMAKE_BINARY_DIR}/mantmp)

# Where we copy the source to during rewrites, and then do the actual
# build from.
set(ADOC_TMPSRC ${MAN_TMPDIR}/ctwm.1.adoc)

# Where the end products wind up
set(MANPAGE ${CMAKE_BINARY_DIR}/ctwm.1)
set(MANHTML ${CMAKE_BINARY_DIR}/ctwm.1.html)
set(MANDBXML ${CMAKE_BINARY_DIR}/ctwm.1.xml)
set(MANPDF   ${CMAKE_BINARY_DIR}/ctwm.1.pdf)

# How we rewrite vars in the manual.  I decided not to use
# configure_file() for this, as it opens up too many chances for
# something to accidentally get sub'd, since we assume people will write
# pretty freeform in the manual.
# \-escaped @ needed for pre-3.1 CMake compat and warning avoidance;
# x-ref `cmake --help-policy CMP0053`
set(MANSED_CMD sed -e \"s,\@ETCDIR@,${ETCDIR},\"
	-e \"s,\@ctwm_version_str@,`head -1 ${CMAKE_SOURCE_DIR}/VERSION`,\")

# Pregen'd doc file paths we might have, in case we can't build them
# ourselves.
set(MAN_PRESRC ${GENSRCDIR}/ctwm.1)
set(HTML_PRESRC ${GENSRCDIR}/ctwm.1.html)





# Figure what we can build
#
# These are both boolean "We can build this type of output" flags, and
# enums for later code for "What method we use to build this type of
# output".

# We use the DocBook XML output for PDF (if manually requested), and
# _can_ use it in very special cases for manpages.  So find out first if
# we can even build it.
set(MANUAL_BUILD_DBXML)
if(ASCIIDOCTOR AND ASCIIDOCTOR_CAN_DBXML)
	set(MANUAL_BUILD_DBXML asciidoctor)
elseif(ASCIIDOC AND ASCIIDOC_CAN_DBXML)
	set(MANUAL_BUILD_DBXML asciidoc)
endif()

# If we have asciidoctor, use it to build the HTML.  Else, we could use
# asciidoc, but leave it disabled because it's very slow.
set(MANUAL_BUILD_HTML)
if(ASCIIDOCTOR AND ASCIIDOCTOR_CAN_HTML)
	set(MANUAL_BUILD_HTML asciidoctor)
elseif(ASCIIDOC)
	set(MANUAL_BUILD_HTML asciidoc)
	if(NOT ENABLE_ASCIIDOC_HTML)
		set(NOAUTO_HTML 1)
		message(STATUS "Not enabling HTML manual build; asciidoc is slow.")
	endif()
endif()

# For the manpage output, asciidoctor has to be of a certain version.  If
# it's not there or not high enough version, we fall back to asciidoc/a2x
# (which is very slow at this too, but we need to build a manpage, so eat
# the expense).  And it's possible to go via the DocBook XML output, but
# it takes very odd cases to wind up there.
set(MANUAL_BUILD_MANPAGE)
if(ASCIIDOCTOR AND ASCIIDOCTOR_CAN_MAN)
	set(MANUAL_BUILD_MANPAGE asciidoctor)
elseif(A2X AND ASCIIDOC_CAN_MAN)
	set(MANUAL_BUILD_MANPAGE a2x)
elseif(XMLTO AND XMLTO_CAN_STUFF AND MANUAL_BUILD_DBXML)
	# Should probably never happen in reality
	set(MANUAL_BUILD_MANPAGE xmlto)
endif()

# PDF output is not hooked into the build process by default, but is made
# available by an extra target.
set(MANUAL_BUILD_PDF)
if(DBLATEX AND DBLATEX_CAN_PDF AND MANUAL_BUILD_DBXML)
	set(MANUAL_BUILD_PDF dblatex)
endif()


# Override: allow forcing use of pregen'd files.
if(FORCE_PREGEN_FILES)
	set(MANUAL_BUILD_HTML)
	set(MANUAL_BUILD_MANPAGE)
	set(MANUAL_BUILD_DBXML)
endif()


# If we can build stuff, prepare bits for it.  Technically unnecessary if
# we're not building stuff, but doesn't do anything bad to define it in
# those cases, and it's easier than listing every MANUAL_BUILD_* in the
# conditions.
set(SETUP_MAN_REWRITE 1)
if(SETUP_MAN_REWRITE)
	# Setup a temp dir under the build for our processing
	file(MAKE_DIRECTORY ${MAN_TMPDIR})

	# We hop through a temporary file to process in definitions for e.g.
	# $ETCDIR.
	add_custom_command(OUTPUT ${ADOC_TMPSRC}
		DEPENDS ${ADOC_SRC} ${CMAKE_SOURCE_DIR}/VERSION
		COMMAND ${MANSED_CMD} < ${ADOC_SRC} > ${ADOC_TMPSRC}
		COMMENT "Processing ctwm.1.adoc -> mantmp/ctwm.1.adoc"
	)

	# We can't actually make other targets depend just on that generated
	# source file, because cmake will try to multi-build it in parallel.
	# To work around, we add a do-nothing custom target that depends on
	# $ADOC_TMPSRC, that uses of it depend on.
	#
	# x-ref http://public.kitware.com/Bug/view.php?id=12311
	#
	# Note, however, that this _doesn't_ transmit the dependancy on
	# ${ADOC_TMPDIR} through; it serves only to serialize the build so it
	# doesn't try to happen twice at once.  So we still need to include
	# ${ADOC_TMPSRC} in the DEPENDS for the targets building off it, or
	# they don't notice when they go out of date.
	add_custom_target(mk_adoc_tmpsrc DEPENDS ${ADOC_TMPSRC})
endif(SETUP_MAN_REWRITE)




#
# Building the manpage variant
#
set(HAS_MAN 0)
if(MANUAL_BUILD_MANPAGE)
	# Got the tool to build it
	message(STATUS "Building manpage with ${MANUAL_BUILD_MANPAGE}.")
	set(HAS_MAN 1)

	if(${MANUAL_BUILD_MANPAGE} STREQUAL "asciidoctor")
		# We don't need the hoops for a2x here, since asciidoctor lets us
		# specify the output directly.
		asciidoctor_mk_manpage(${MANPAGE} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	elseif(${MANUAL_BUILD_MANPAGE} STREQUAL "a2x")
		# a2x has to jump through some stupid hoops
		a2x_mk_manpage(${MANPAGE} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	elseif(${MANUAL_BUILD_MANPAGE} STREQUAL "xmlto")
		# xmlto does its own hoops too
		xmlto_mk_manpage(${MANPAGE} ${MANDBXML})
	else()
		message(FATAL_ERROR "I don't know what to do with that manpage "
			"building type!")
	endif()
elseif(EXISTS ${MAN_PRESRC})
	# Can't build it ourselves, but we've got a prebuilt version.
	message(STATUS "Can't build manpage, using prebuilt version.")
	set(HAS_MAN 1)

	# We still have to do the substitutions like above, but we're doing
	# it on the built version now, rather than the source.
	add_custom_command(OUTPUT ${MANPAGE}
		DEPENDS ${MAN_PRESRC} ${CMAKE_SOURCE_DIR}/VERSION
		COMMAND ${MANSED_CMD} < ${MAN_PRESRC} > ${MANPAGE}
		COMMENT "Processing prebuilt manpage -> ctwm.1"
	)
else()
	# Can't build it, no prebuilt.  Not quite fatal, but very bad.
	message(WARNING "Can't build manpage, and no prebuilt version "
		"available.  You won't get one.")
endif(MANUAL_BUILD_MANPAGE)


# Assuming we have it, compress manpage (conditionally).  We could add
# more magic to allow different automatic compression, but that's
# probably way more involved than we need to bother with.  Most systems
# use gzip, and for the few that don't, the packagers can use
# NOMANCOMPRESS and handle it out of band.
if(HAS_MAN)
	if(NOT NOMANCOMPRESS)
		find_program(GZIP_CMD gzip)
		add_custom_command(OUTPUT "${MANPAGE}.gz"
			DEPENDS ${MANPAGE}
			COMMAND ${GZIP_CMD} -nc ${MANPAGE} > ${MANPAGE}.gz
			COMMENT "Compressing ctwm.1.gz"
		)
		add_custom_target(man ALL DEPENDS "${MANPAGE}.gz")
		set(INSTMAN "${MANPAGE}.gz")
	else()
		add_custom_target(man ALL DEPENDS ${MANPAGE})
		set(INSTMAN ${MANPAGE})
	endif(NOT NOMANCOMPRESS)
endif(HAS_MAN)




#
# Do the HTML manual
#
set(HAS_HTML 0)
if(MANUAL_BUILD_HTML AND NOAUTO_HTML)
	message(STATUS "Not autobuilding HTML manual with ${MANUAL_BUILD_HTML}.")
endif()
# Separate if() rather than an elseif() so that the above case can still
# fall into the elseif(EXISTS ${HTML_PRESRC}) below and use the pregen'd
# version.
if(MANUAL_BUILD_HTML AND NOT NOAUTO_HTML)
	# Got the tool to build it
	message(STATUS "Building HTML manual with ${MANUAL_BUILD_HTML}.")
	set(HAS_HTML 1)

	if(${MANUAL_BUILD_HTML} STREQUAL "asciidoctor")
		asciidoctor_mk_html(${MANHTML} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	elseif(${MANUAL_BUILD_HTML} STREQUAL "asciidoc")
		asciidoc_mk_html(${MANHTML} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	else()
		message(FATAL_ERROR "I don't know what to do with that HTML manual "
			"building type!")
	endif()
elseif(EXISTS ${HTML_PRESRC})
	# Can't build it ourselves, but we've got a prebuilt version.
	message(STATUS "Can't build HTML manual, using prebuilt version.")
	set(HAS_HTML 1)
	set(NOAUTO_HTML) # Clear so ALL target get set below

	# As with the manpage above, we need to do the processing on the
	# generated version for build options.
	add_custom_command(OUTPUT ${MANHTML}
		DEPENDS ${HTML_PRESRC} ${CMAKE_SOURCE_DIR}/VERSION
		COMMAND ${MANSED_CMD} < ${HTML_PRESRC} > ${MANHTML}
		COMMENT "Processing prebuilt manual -> ctwm.1.html"
	)

else()
	# Can't build it, no prebuilt.
	# Left as STATUS, since this is "normal" for now.
	message(STATUS "Can't build HTML manual, and no prebuilt version "
		"available.")
endif()  # Variants of building HTML manual


# If we have (or are building) the HTML, add an easy target for it, and
# define a var for the install process to notice.
if(HAS_HTML)
	if(NOAUTO_HTML)
		add_custom_target(man-html DEPENDS ${MANHTML})
	else()
		add_custom_target(man-html ALL DEPENDS ${MANHTML})
		set(INSTHTML ${MANHTML})
	endif(NOAUTO_HTML)
endif(HAS_HTML)




#
# Building DocBook XML
#
set(HAS_DBXML 0)
if(MANUAL_BUILD_DBXML)
	# Got the tool to build it
	#message(STATUS "Building DocBook XML with ${MANUAL_BUILD_DBXML}.")
	set(HAS_DBXML 1)

	if(${MANUAL_BUILD_DBXML} STREQUAL "asciidoctor")
		# We don't need the hoops for a2x here, since asciidoctor lets us
		# specify the output directly.
		asciidoctor_mk_docbook(${MANDBXML} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	elseif(${MANUAL_BUILD_DBXML} STREQUAL "asciidoc")
		# a2x has to jump through some stupid hoops
		asciidoc_mk_docbook(${MANDBXML} ${ADOC_TMPSRC} DEPENDS mk_adoc_tmpsrc)
	else()
		message(FATAL_ERROR "I don't know what to do with that DocBook "
			"building type!")
	endif()
endif(MANUAL_BUILD_DBXML)




#
# And the PDF output
#
set(HAS_PDF 0)
if(MANUAL_BUILD_PDF)
	# Got the tool to build it
	#message(STATUS "Building PDF with ${MANUAL_BUILD_PDF}.")
	set(HAS_PDF 1)

	if(${MANUAL_BUILD_PDF} STREQUAL "dblatex")
		dblatex_mk_pdf(${MANPDF} ${MANDBXML})
	else()
		message(FATAL_ERROR "I don't know what to do with that PDF "
			"building type!")
	endif()
endif(MANUAL_BUILD_PDF)

if(HAS_PDF)
	add_custom_target(man-pdf DEPENDS ${MANPDF})
endif(HAS_PDF)




#
# Handy target
#
set(MAN_TYPES)
if(HAS_MAN)
	list(APPEND MAN_TYPES man)
endif()
if(HAS_HTML)
	list(APPEND MAN_TYPES man-html)
endif()
if(HAS_PDF)
	list(APPEND MAN_TYPES man-pdf)
endif()
add_custom_target(man-all DEPENDS ${MAN_TYPES})
