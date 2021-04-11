#
# Setup some vars we use in the configure/build process
#

# The dir in which we ship pregen'd source files
set(GENSRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)

# Where our manual source (asciidoc) files are
set(MANSRCDIR ${CMAKE_SOURCE_DIR}/doc/manual)

# Various build tools
set(TOOLS ${CMAKE_SOURCE_DIR}/tools)


# Our base set of sources
set(CTWMSRC
	# Basic files  ##STDSRC-START
	add_window.c
	animate.c
	captive.c
	clargs.c
	clicktofocus.c
	colormaps.c
	ctopts.c
	ctwm_main.c
	cursor.c
	drawing.c
	event_core.c
	event_handlers.c
	event_names.c
	event_utils.c
	functions.c
	functions_captive.c
	functions_icmgr_wsmgr.c
	functions_identify.c
	functions_misc.c
	functions_warp.c
	functions_win.c
	functions_win_moveresize.c
	functions_workspaces.c
	gc.c
	iconmgr.c
	icons.c
	icons_builtin.c
	image.c
	image_bitmap.c
	image_bitmap_builtin.c
	image_xwd.c
	list.c
	mask_screen.c
	menus.c
	mwmhints.c
	occupation.c
	otp.c
	parse.c
	parse_be.c
	parse_yacc.c
	session.c
	util.c
	vscreen.c
	win_decorations.c
	win_decorations_init.c
	win_iconify.c
	win_ops.c
	win_regions.c
	win_resize.c
	win_utils.c
	windowbox.c
	workspace_config.c
	workspace_manager.c
	workspace_utils.c

	# External libs
	ext/repl_str.c
	##STDSRC-END

	# Generated files  ##GENSRC-START
	ctwm_atoms.c
	deftwmrc.c
	gram.tab.c
	lex.c
	version.c
	##GENSRC-END
)


# Libs to link in (init empty list)
set(CTWMLIBS)


# Our normal set of warning flags
set(STD_WARNS
	-Wall
	-Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wundef
	-Wredundant-decls -Wcast-align -Wcast-qual -Wchar-subscripts
	-Winline -Wnested-externs -Wmissing-declarations
)
