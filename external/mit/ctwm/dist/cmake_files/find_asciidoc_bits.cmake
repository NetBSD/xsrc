#
# Find and setup asciidoc[tor] bits
#


# First see if we can find the programs
find_program(ASCIIDOCTOR asciidoctor)
find_program(ASCIIDOC asciidoc)
find_program(A2X a2x)
find_program(DBLATEX dblatex)
find_program(XMLTO xmlto)


# If we have asciidoctor, we need to figure out the version, as manpage
# output is relatively new.
if(ASCIIDOCTOR)
	execute_process(
		COMMAND ${ASCIIDOCTOR} --version
		RESULT_VARIABLE _adoctor_result
		OUTPUT_VARIABLE _adoctor_verout
		ERROR_QUIET
	)
	if(NOT ${_adoctor_result} EQUAL "0")
		# Err...
		message(WARNING "Unexpected result trying asciidoctor --version.")
		set(_adoctor_verout "Asciidoctor 0.0.0 FAKE")
	endif()
	unset(_adoctor_result)

	# Break out the version.
	set(_adoctor_veregex "Asciidoctor ([0-9]+\\.[0-9]+\\.[0-9]+).*")
	string(REGEX REPLACE ${_adoctor_veregex} "\\1"
		ASCIIDOCTOR_VERSION ${_adoctor_verout})
	unset(_adoctor_verout)
	unset(_adoctor_veregex)
	message(STATUS "Found asciidoctor (${ASCIIDOCTOR}) version ${ASCIIDOCTOR_VERSION}")

	# 1.5.3 is the first release that can write manpages natively.  This
	# means 1.5.3 dev versions after a certain point can as well; assume
	# anybody running a 1.5.3 dev is keeping up well enough that it can
	# DTRT too.  We assume any version can do HTML.
	set(ASCIIDOCTOR_CAN_MAN  0)
	set(ASCIIDOCTOR_CAN_HTML 1)
	set(ASCIIDOCTOR_CAN_DBXML 1)
	if(${ASCIIDOCTOR_VERSION} VERSION_GREATER "1.5.2")
		set(ASCIIDOCTOR_CAN_MAN 1)
	elseif(${ASCIIDOCTOR_VERSION} VERSION_LESS "0.0.1")
		set(ASCIIDOCTOR_CAN_HTML 0)
		set(ASCIIDOCTOR_CAN_DBXML 0)
	endif()

	# dblatex PDF output works fine with docbook5.  xmlto/docbook XSL
	# manpage generation doesn't, so it has to override this.
	set(ASCIIDOCTOR_DB_VER 5)
endif(ASCIIDOCTOR)


# For asciidoc, it doesn't really matter, but look up the version for
# cosmetics anyway
if(ASCIIDOC)
	execute_process(
		COMMAND ${ASCIIDOC} --version
		RESULT_VARIABLE _adoc_result
		OUTPUT_VARIABLE _adoc_verout
		ERROR_QUIET
	)
	if(NOT ${_adoc_result} EQUAL "0")
		# Err...
		message(WARNING "Unexpected result trying asciidoc --version.")
		set(_adoc_verout "asciidoc 0.0.0")
	endif()
	unset(_adoc_result)

	# Break out the version.
	set(_adoc_veregex "asciidoc ([0-9]+\\.[0-9]+\\.[0-9]+).*")
	string(REGEX REPLACE ${_adoc_veregex} "\\1"
		ASCIIDOC_VERSION ${_adoc_verout})
	unset(_adoc_verout)
	unset(_adoc_veregex)
	message(STATUS "Found asciidoc (${ASCIIDOC}) version ${ASCIIDOC_VERSION}")

	# Can always do both, unless horked
	if(${ASCIIDOC_VERSION} VERSION_GREATER "0.0.0")
		set(ASCIIDOC_CAN_MAN  1)
		set(ASCIIDOC_CAN_HTML 1)
		set(ASCIIDOC_CAN_DBXML 1)
	endif()

	# This is an example of 'horked'...
	if(NOT A2X)
		set(ASCIIDOC_CAN_MAN 0)
	endif()

	# Only docbook version python asciidoc supports
	set(ASCIIDOC_DB_VER 45)
endif(ASCIIDOC)


# dblatex lets us build PDF's from the DocBook XML.  This is pretty
# fringe and not part of normal builds, so try to minimize the impact of
# the checks.
if(DBLATEX)
	# Don't really care about the version, so save the extra checks
	if(0)
		execute_process(
			COMMAND ${DBLATEX} --version
			RESULT_VARIABLE _dblatex_result
			OUTPUT_VARIABLE _dblatex_verout
			ERROR_QUIET
		)
		if(NOT ${_dblatex_result} EQUAL "0")
			# Err...
			message(WARNING "Unexpected result trying dblatex --version.")
			set(_dblatex_verout "dblatex 0.0.0 FAKE")
		endif()
		unset(_dblatex_result)

		# Break out the version.
		set(_dblatex_veregex "dblatex version ([0-9]+\\.[0-9]+\\.[0-9]+).*")
		string(REGEX REPLACE ${_dblatex_veregex} "\\1"
			DBLATEX_VERSION ${_dblatex_verout})
		unset(_dblatex_verout)
		unset(_dblatex_veregex)
		message(STATUS "Found dblatex (${DBLATEX}) version ${DBLATEX_VERSION}")
	else()
		message(STATUS "Found dblatex (${DBLATEX})")
	endif()

	# I guess it works...
	set(DBLATEX_CAN_PDF 1)
endif(DBLATEX)


# xmlto is another frontend for DocBook XML -> stuff.  It can indirect
# through dblatex (like we just do manually above) or through fop for PDF
# output, but also knows how to invoke xsltproc to generate manpage
# output, which gives us another route from adoc -> XML -> manpage.  And
# potentially other formats, if we start caring.
if(XMLTO)
	# Don't really care about the version, so save the extra checks
	if(0)
		execute_process(
			COMMAND ${XMLTO} --version
			RESULT_VARIABLE _xmlto_result
			OUTPUT_VARIABLE _xmlto_verout
			ERROR_QUIET
		)
		if(NOT ${_xmlto_result} EQUAL "0")
			# Err...
			message(WARNING "Unexpected result trying xmlto --version.")
			set(_xmlto_verout "xmlto 0.0.0 FAKE")
		endif()
		unset(_xmlto_result)

		# Break out the version.
		set(_xmlto_veregex "xmlto version ([0-9]+\\.[0-9]+\\.[0-9]+).*")
		string(REGEX REPLACE ${_xmlto_veregex} "\\1"
			XMLTO_VERSION ${_xmlto_verout})
		unset(_xmlto_verout)
		unset(_xmlto_veregex)
		message(STATUS "Found xmlto (${XMLTO}) version ${XMLTO_VERSION}")
	else()
		message(STATUS "Found xmlto (${XMLTO})")
	endif()

	# I guess it can do whatever...
	set(XMLTO_CAN_STUFF 1)
endif(XMLTO)




#
# Generator functions for creating targets for the various
# transformations.
#

# Lot of boilerplate in all of them
macro(_ad_mk_boilerplate PROG OUT)
	# Minimal seatbelt
	set(my_usage "${PROG}_mk_${OUT}(<output> <input> [DEPENDS <deps>] [COMMENT <comment>])")
	cmake_parse_arguments(
		_ARGS
		""
		"COMMENT"
		"DEPENDS"
		${ARGN}
	)
	if(_ARGS_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR ${my_usage})
	endif()

	# Always depend on the input file, maybe on more
	set(dependancies ${ADFILE})
	if(_ARGS_DEPENDS)
		list(APPEND dependancies ${_ARGS_DEPENDS})
	endif()

	# Come up with some comment or other
	if(NOT _ARGS_COMMENT)
		get_filename_component(basename ${OUTFILE} NAME)
		set(_ARGS_COMMENT "Generating ${basename} with ${PROG}")
	endif()
endmacro(_ad_mk_boilerplate)


# Build a manpage via asciidoctor
function(asciidoctor_mk_manpage OUTFILE ADFILE)
	# Guard
	if(NOT ASCIIDOCTOR_CAN_MAN)
		message(FATAL_ERROR "asciidoctor can't do man")
	endif()

	_ad_mk_boilerplate(asciidoctor manpage ${ARGN})

	# Setup the rule
	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND ${ASCIIDOCTOR} -b manpage -o ${OUTFILE} ${ADFILE}
		COMMENT ${_ARGS_COMMENT}
	)
endfunction(asciidoctor_mk_manpage)


# Build a manpage via asciidoc (technically, a2x)
function(a2x_mk_manpage OUTFILE ADFILE)
	# Guard
	if(NOT A2X OR NOT ASCIIDOC_CAN_MAN)
		message(FATAL_ERROR "asciidoc/a2x can't do man")
	endif()

	_ad_mk_boilerplate(a2x manpage ${ARGN})

	# a2x gives us very little control over input/output files, so we
	# have to do some vaguely stupid dances.  In theory, -D works for the
	# manpage output, but it's doc'd not to and will warn, so don't even
	# try.  The result is that it always puts the outfile file next to
	# the input.  So we make a temporary dir (with a hopefully unique
	# name) and do all our stuff in there.
	get_filename_component(basedir ${ADFILE} DIRECTORY)
	while(1)
		string(RANDOM rndstr)
		set(a2x_tmpdir "${basedir}/a2x.${rndstr}")
		if(NOT IS_DIRECTORY ${a2x_tmpdir})
			break()
		endif()
	endwhile()
	file(MAKE_DIRECTORY ${a2x_tmpdir})

	# This had better already be named "someprog.somesection.adoc",
	# because a2x is going to magically figure the program and section
	# name from the contents and make that output file.
	get_filename_component(inbasename ${ADFILE} NAME)
	string(REGEX REPLACE "(.*)\\.adoc$" "\\1" outbasename ${inbasename})
	if(NOT outbasename)
		message(FATAL_ERROR "Can't figure output for ${inbasename}")
	endif()

	# In/out tmpfile names
	set(a2x_intmp  "${a2x_tmpdir}/${inbasename}")
	set(a2x_outtmp "${a2x_tmpdir}/${outbasename}")

	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND cp ${ADFILE} ${a2x_intmp}
		COMMAND ${A2X} --doctype manpage --format manpage ${a2x_intmp}
		COMMAND mv ${a2x_outtmp} ${OUTFILE}
		COMMAND rm ${a2x_intmp}
		COMMENT ${_ARGS_COMMENT}
	)
endfunction(a2x_mk_manpage)


# Build a manpage via xmlto
function(xmlto_mk_manpage OUTFILE XMLFILE)
	# Guard
	if(NOT XMLTO)
		message(FATAL_ERROR "xmlto can't do man")
	endif()

	_ad_mk_boilerplate(xmlto manpage ${ARGN})

	# As with a2x, this had better already be named
	# "someprog.somesection.xml" because we have so little control over
	# the output location.
	get_filename_component(inbasename ${XMLFILE} NAME)
	string(REGEX REPLACE "(.*)\\.xml$" "\\1" outbasename ${inbasename})
	if(NOT outbasename)
		message(FATAL_ERROR "Can't figure output for ${inbasename}")
	endif()

	get_filename_component(basedir ${XMLFILE} DIRECTORY)
	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${XMLFILE} ${dependancies}
		COMMAND ${XMLTO}
			--skip-validation
			-o ${basedir}
			# This apparently doesn't work right...
			--stringparam 'man.endnotes.list.enabled=0'
			man ${XMLFILE}
		COMMENT ${_ARGS_COMMENT}
	)

	# Set various overrides.  Note that this leads to rather worse PDF
	# output.  If we ever decide to make xmlto a more likely part of the
	# process, we probably need to rework things so we generate a
	# different XML for the manpage path vs. the PDF path...
	set(OVERRIDE_DTYPE manpage PARENT_SCOPE)

	# This does _very_ poorly [currently?] with DocBook 5 output.
	if(ASCIIDOCTOR_CAN_DBXML)
		set(_addg "; downgrading asciidoctor output to docbook45")
		set(ASCIIDOCTOR_DB_VER 45 PARENT_SCOPE)
	endif()

	message(WARNING "Using xmlto manpage generation${_addg}.  This "
		"will compromise the quality of PDF output.")
endfunction(xmlto_mk_manpage)



# Build HTML output with asciidoctor
function(asciidoctor_mk_html OUTFILE ADFILE)
	# Guard
	if(NOT ASCIIDOCTOR_CAN_HTML)
		message(FATAL_ERROR "asciidoctor can't do html")
	endif()

	_ad_mk_boilerplate(asciidoctor html ${ARGN})

	# Setup the rule
	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND ${ASCIIDOCTOR} -atoc -anumbered -o ${OUTFILE} ${ADFILE}
		COMMENT ${_ARGS_COMMENT}
	)
endfunction(asciidoctor_mk_html)


# And the asciidoc HTML
function(asciidoc_mk_html OUTFILE ADFILE)
	# Guard
	if(NOT ASCIIDOC_CAN_HTML)
		message(FATAL_ERROR "asciidoc can't do html")
	endif()

	_ad_mk_boilerplate(asciidoc html ${ARGN})

	# Setup the rule
	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND ${ASCIIDOC} -atoc -anumbered -o ${OUTFILE} ${ADFILE}
		COMMENT ${_ARGS_COMMENT}
	)
endfunction(asciidoc_mk_html)


# Building DocBook XML
function(asciidoctor_mk_docbook OUTFILE ADFILE)
	# Guard
	if(NOT ASCIIDOCTOR_CAN_DBXML)
		message(FATAL_ERROR "asciidoctor can't do DocBook")
	endif()

	_ad_mk_boilerplate(asciidoctor docbook ${ARGN})

	set(DTYPE article)
	if(OVERRIDE_DTYPE)
		set(DTYPE ${OVERRIDE_DTYPE})
	endif()

	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND ${ASCIIDOCTOR} -b docbook${ASCIIDOCTOR_DB_VER}
			-d ${DTYPE} -o ${OUTFILE} ${ADFILE}
		COMMENT "${_ARGS_COMMENT} (docbook${ASCIIDOCTOR_DB_VER},${DTYPE})"
	)
endfunction(asciidoctor_mk_docbook)

function(asciidoc_mk_docbook OUTFILE ADFILE)
	# Guard
	if(NOT ASCIIDOC_CAN_DBXML)
		message(FATAL_ERROR "asciidoc can't do DocBook")
	endif()

	_ad_mk_boilerplate(asciidoc docbook ${ARGN})

	set(DTYPE article)
	if(OVERRIDE_DTYPE)
		set(DTYPE ${OVERRIDE_DTYPE})
	endif()

	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${dependancies}
		COMMAND ${ASCIIDOC} -b docbook${ASCIIDOC_DB_VER}
			-d ${DTYPE} -o ${OUTFILE} ${ADFILE}
		COMMENT "${_ARGS_COMMENT} (docbook${ASCIIDOC_DB_VER},${DTYPE})"
	)
endfunction(asciidoc_mk_docbook)


# PDF via dblatex
function(dblatex_mk_pdf OUTFILE XMLFILE)
	if(NOT DBLATEX_CAN_PDF)
		message(FATAL_ERROR "dblatex can't do PDF")
	endif()

	_ad_mk_boilerplate(dblatex pdf ${ARGN})

	# Passes through to LaTeX geometry.
	# Likely choices: letterpaper, a4paper
	if(NOT DBLATEX_PAPERSIZE)
		set(DBLATEX_PAPERSIZE "a4paper")
	endif()

	add_custom_command(OUTPUT ${OUTFILE}
		DEPENDS ${XMLFILE} ${dependancies}
		COMMAND ${DBLATEX}
			-tpdf
			-Pdoc.collab.show=0
			-Platex.output.revhistory=0
			-Ppaper.type=${DBLATEX_PAPERSIZE}
			-Ppage.margin.top=2cm
			-Ppage.margin.bottom=2cm
			-o ${OUTFILE} ${XMLFILE}
		COMMENT ${_ARGS_COMMENT}
	)
endfunction(dblatex_mk_pdf)
