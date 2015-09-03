# Description file for CTWM.

.INCLUDE descrip.local

.IFDEF XPM
.IFDEF USER_XPMLIBDIR
XPMLIBDIR       = $(USER_XPMLIBDIR)
.ELSE
XPMLIBDIR       =
.ENDIF
.IFDEF USER_XPMINCDIR
XPMINCDIR       = ,$(USER_XPMINCDIR)
.ELSE
XPMINCDIR       =
.ENDIF

XPMDEFINES      = ,XPM
XPMLIB          = $(XPMLIBDIR)LIBXPM
.ELSE
XPMLIBDIR       = 
XPMINCDIR       = 

XPMDEFINES      = 
XPMLIB          = 
.ENDIF

.IFDEF JPEG
.IFDEF USER_JPEGLIBDIR
JPEGLIBDIR      = $(USER_JPEGLIBDIR)
.ELSE
JPEGLIBDIR      =
.ENDIF
.IFDEF USER_JPEGINCDIR
JPEGINCDIR      = ,$(USER_JPEGINCDIR)
.ELSE
JPEGINCDIR      =
.ENDIF

JPEGDEFINES     = ,JPEG
JPEGLIB         = $(JPEGLIBDIR)LIBJPEG
.ELSE
JPEGLIBDIR      = 
JPEGINCDIR      = 

JPEGDEFINES     = 
JPEGLIB         = 
.ENDIF

.IFDEF IMCONV
.IFDEF USER_IMCONVLIBDIR
IMCONVLIBDIR    = $(USER_IMCONVLIBDIR)
.ELSE
IMCONVLIBDIR    =
.ENDIF
.IFDEF USER_IMCONVINCDIR
IMCONVINCDIR    = ,$(USER_IMCONVINCDIR)
.ELSE
IMCONVINCDIR    =
.ENDIF

IMCONVDEFINES   = ,IMCONV
IMCONVLIB       = $(IMCONVLIBDIR)LIBIM,$(IMCONVLIBDIR)LIBSDSC
.ELSE
IMCONVLIBDIR    = 
IMCONVINCDIR    = 

IMCONVDEFINES   = 
IMCONVLIB       = 
.ENDIF

.IFDEF USE_M4
M4LIBDIR    =
M4INCDIR    =

M4DEFINES   = ,USEM4
M4LIB       = $(M4LIBDIR)
.ELSE
M4LIBDIR    = 
M4INCDIR    = 

M4DEFINES   = 
M4LIB       = 
.ENDIF

.IFDEF GNOME
GNOMEDEFINES   = ,GNOME
GNOMESRC       = GNOME.C
GNOMEOBJ       = ,$(GOAL)GNOME.$(EXT)OBJ
.ELSE
GNOMEDEFINES   = 
GNOMESRC       = 
GNOMEOBJ       = 
.ENDIF

.IFDEF USE_SOUND
.IFDEF USER_SOUNDLIBDIR
SOUNDLIBDIR     = $(USER_SOUNDLIBDIR)
.ELSE
SOUNDLIBDIR     =
.ENDIF
.IFDEF USER_SOUNDINCDIR
SOUNDINCDIR     = ,$(USER_SOUNDINCDIR)
.ELSE
SOUNDINCDIR     =
.ENDIF

SOUNDDEFINES    = ,SOUNDS
SOUNDLIB        = $(SOUNDLIBDIR)LIBRPLAY
SOUNDSRC        = SOUND.C
SOUNDOBJ        = ,$(GOAL)SOUND.$(EXT)OBJ
.ELSE
SOUNDLIBDIR     = 
SOUNDINCDIR     = 

SOUNDDEFINES    = 
SOUNDLIB        = 
SOUNDSRC        = 
SOUNDOBJ        = 
.ENDIF

MORE_CFLAGS    = /DEBUG/DEFINE=(VMS$(M4DEFINES)$(GNOMEDEFINES)$(IMCONVDEFINES)$(XPMDEFINES)$(JPEGDEFINES)$(SOUNDDEFINES),FUNCPROTO=14,C_ALLOCA,""emacs"",BLOCK_INPUT,EXIT_ENDSESSION=$(EXIT_ENDSESSION)''extra_defs')''extra_includes'
EXTRA_INCLUDES = $(M4INCDIR)$(IMCONVINCDIR)$(XPMINCDIR)$(JPEGINCDIR)$(SOUNDINCDIR)

LD = link
#LDFLAGS = /DEBUG
LDFLAGS = /NOTRACE

# All the files we distribute
DISTFILES = readme.* changes. *.txt *.doc *.*mms imakefile.* *.com *.c *.c_vms *.h *.h_vms *.y *.l *.ctwmrc *.*orig *.bm [.xpm]*.*

# Some files we distribute in the source-only archive
SRCDISTEXTRA = *.*diff

# Some files we distribute in the .zip archive file.
ZIPDISTEXTRA = *.*_obj

# Files we NEVER distribute.  I have no idea how I will deal with this
# regarding tar, though...
NODIST = gram.tab.c gram.tab.h lex.c

all : setup
	@ ! We define the MMS/MMK macros as symbols, or we might get problems
	@ ! we too long DCL lines...		(suggested by Michael Lemke.)
	LD="$(LD)"
	LDFLAGS="$(LDFLAGS)"
	GOAL="$(GOAL)"
	X11XMU="$(X11XMU)"
	X11XMULIB="$(X11XMULIB)"
	X11SM="$(X11SM)"
	X11SMLIB="$(X11SMLIB)"
	XPMINCDIR="$(XPMINCDIR)"
	XPMLIBDIR="$(XPMLIBDIR)"
	XPMDEFINES="$(XPMDEFINES)"
	XPMLIB="$(XPMLIB)"
	JPEGINCDIR="$(JPEGINCDIR)"
	JPEGLIBDIR="$(JPEGLIBDIR)"
	JPEGDEFINES="$(JPEGDEFINES)"
	JPEGLIB="$(JPEGLIB)"
	IMCONVINCDIR="$(IMCONVINCDIR)"
	IMCONVLIBDIR="$(IMCONVLIBDIR)"
	IMCONVDEFINES="$(IMCONVDEFINES)"
	IMCONVLIB="$(IMCONVLIB)"
	M4INCDIR="$(M4INCDIR)"
	M4LIBDIR="$(M4LIBDIR)"
	M4DEFINES="$(M4DEFINES)"
	M4LIB="$(M4LIB)"
	GNOMEDEFINES="$(GNOMEDEFINES)"
	GNOMESRC="$(GNOMESRC)"
	GNOMEOBJ="$(GNOMEOBJ)"
	SOUNDINCDIR="$(SOUNDINCDIR)"
	SOUNDLIBDIR="$(SOUNDLIBDIR)"
	SOUNDDEFINES="$(SOUNDDEFINES)"
	SOUNDLIB="$(SOUNDLIB)"
	SOUNDSRC="$(SOUNDSRC)"
	SOUNDOBJ="$(SOUNDOBJ)"
	$(MMS) $(MMSQUALIFIERS) /OVERRIDE /DESCRIPTION=DESCRIP.SUBMMS -
		/MACRO=(EXT="''ext'",EXE_EXT="''exe_ext'"'do_option''dep_xwdfile')

setup : version.opt
	__axp = f$getsyi("CPU") .ge. 128
	__tmp = f$edit("$(CFLAGS)","UPCASE")
	__decc = f$search("SYS$SYSTEM:DECC$COMPILER.EXE") .nes. "" -
		.and. __tmp - "/VAXC" .eqs. __tmp
	ext = "VAX_"
	if __axp then ext = "AXP_"
	exe_ext = ext
	if .not. __decc then ext = ext + "VAXC_"
	opt_ext = "DECC_OPT"
	if .not. __decc then opt_ext = "VAXC_OPT"
	CLIB = ""
	if .not. __decc then CLIB = "SYS$SHARE:VAXCRTL/SHARE"
	extra_defs = ""
	if .not. __decc then extra_defs = extra_defs + ",NO_LOCALE"
	if f$search("XWDFILE.H") .nes. "" then -
		extra_defs = extra_defs + ",HAVE_XWDFILE_H"
	dep_xwdfile = ""
	if extra_defs - ",HAVE_XWDFILE_H" .nes. extra_defs then -
		dep_xwdfile = ",DEP_XWDFILE=""XWDFILE.H"""
	extra_includes = "$(EXTRA_INCLUDES)" - ","
	if extra_includes .nes. "" then -
		extra_includes = "/INCLUDE=(" + extra_includes + ")"
	! Consider that the default CFLAGS contains `/OBJECT=$*.OBJ'
	CFLAGS = "$(CFLAGS)" - "/OBJECT=SETUP.OBJ" + "$(MORE_CFLAGS)"
	do_option = ",OPTION=""$(GOAL)CTWM.''opt_ext'"",OPTIONCMD="",$(GOAL)CTWM.''opt_ext'/OPT"""
	if __decc then CFLAGS = "/DECC/PREFIX=ALL " + CFLAGS
	if __decc .and. __axp then CFLAGS = CFLAGS + "/L_DOUBLE=64"

!ctwm.hlp : ctwm.rnh
!	runoff ctwm.rnh

mostlyclean :
	- delete/log *.*_obj;*
	- delete/log *.*_opt;*,*.opt;*
	- purge/log

clean : mostlyclean
	- delete/log *.*_exe;*
	- delete/log *.hlp;*
	- delete/log zip.*;*

!######################## Only for distributors #############################

!The clauses following are not meant to look at if you don't intend to make
!a ctwm distribution.  With time, this section will most probably be filled
!with one hack after the other.  I'll try to put some comments so you won't
!get too much lost...					   /Richard Levitte

dist_tar :	setup hackfiles versions real_dist_tar unhackfiles FRC
	@ !

real_dist_tar :
	- tar -cvf ctwm'vmsver'-src.tar $(DISTFILES) $(SRCDISTEXTRA)

dist_zip :	setup hackfiles versions zip.comment-link real_dist_zip -
		unhackfiles FRC
	@ !

real_dist_zip :
	- define/user sys$input zip.comment-link
	- define/user sys$output zip.log
	- zip -"Vz" ctwm'vmsver'.zip $(DISTFILES) $(ZIPDISTEXTRA) -x $(NODIST)

zip_src :	setup hackfiles versions zip.comment real_zip_src unhackfiles -
		FRC
	@ !

real_zip_src :
	- define/user sys$input zip.comment
	- define/user sys$output zip.log
	- zip -"z" ctwm'vmsver'-src.zip $(DISTFILES) $(SRCDISTEXTRA) -x $(NODIST)

hackfiles : LEX.C_VMS GRAM.C_VMS GRAM.H_VMS 
	- purge LEX.C
	- rename LEX.C LEX.C_VMS
	- purge LEX.C_VMS
	- purge GRAM.C
	- rename GRAM.C GRAM.C_VMS
	- purge GRAM.C_VMS
	- purge GRAM.H
	- rename GRAM.H GRAM.H_VMS
	- purge GRAM.H_VMS
	- delete XWDFILE.H;*
	@ write sys$output "**************************************************"
	@ write sys$output "* WARNING!!!                                     *"
	@ write sys$output "* Some of your files will be HACKED in a moment. *"
.IFDEF NONOVICE
	@ write sys$output "* I'll wait five seconds before I start.         *"
.ELSE
	@ write sys$output "* I'll wait one minute before I start.           *"
.ENDIF
	@ write sys$output "* The hacked files will be restored when I've    *"
	@ write sys$output "* finished doing the distribution you wanted     *"
	@ write sys$output "* DO NOT STOP WHEN IT HAS STARTED!               *"
	@ write sys$output "* If you do anyway, do a `MMS unhackfiles' and   *"
	@ write sys$output "* all hacks will hopefully be cleared.  In any   *"
	@ write sys$output "* case, DO NOT PURGE before this is resolved     *"
	@ write sys$output "* resolved (you will be told when all is OK)     *"
	@ write sys$output "**************************************************"
.IFDEF NONOVICE
.ELSE
	@ wait 0:0:30
	@ write sys$output "*** 30 seconds left"
	@ wait 0:0:15
	@ write sys$output "*** 15 seconds left"
	@ wait 0:0:5
	@ write sys$output "*** 10 seconds left"
	@ wait 0:0:5
	@ write sys$output "*** 5 seconds left"
.ENDIF
	@ wait 0:0:5
	@ write sys$output "*** OK, here goes..."
	@ write sys$output "*** Processing XPM.H"
	- purge XPM.H
	- rename XPM.H XPM.H_NODIST
	@ !Now, to the real hackery!
	@ open/write foo routine.tpu_tmp
	@ write foo "input_file:=GET_INFO(COMMAND_LINE, ""file_name"");"
	@ write foo "main_buffer := CREATE_BUFFER (""main"", input_file);"
	@ write foo "position (beginning_of (main_buffer));"
	@ write foo "loop"
	@ write foo "	r := search_quietly (LINE_BEGIN+(""XPMLIB=""+unanchor+"""")@r0+LINE_END, FORWARD);"
	@ write foo "	EXITIF r = 0;"
	@ write foo "	if beginning_of(r0) <> end_of(r0)"
	@ write foo "	then"
	@ write foo "		erase (r0);"
	@ write foo "		position (r0);"
	@ write foo "		copy_text (""XPMLIB=$(FAKE_XPMLIB)descrip.mms"");"
	@ write foo "	endif;"
	@ write foo "	position (end_of (r));"
	@ write foo "endloop;"
	@ write foo "position (beginning_of (main_buffer));"
	@ write foo "loop"
	@ write foo "	r := search_quietly (LINE_BEGIN+(""$ define/nolog xpmlib ""+unanchor+"""")@r0+LINE_END, FORWARD);"
	@ write foo "	EXITIF r = 0;"
	@ write foo "	if beginning_of(r0) <> end_of(r0)"
	@ write foo "	then"
	@ write foo "		erase (r0);"
	@ write foo "		position (r0);"
	@ write foo "		copy_text (""$ define/nolog xpmlib $(FAKE_XPMLIB)make.com"");"
	@ write foo "	endif;"
	@ write foo "	position (end_of (r));"
	@ write foo "endloop;"
	@ write foo "write_file(main_buffer, get_info (command_line, ""output_file""));"
	@ write foo "quit;"
	@ close foo
	@ save_mesg = f$environment("MESSAGE")
	@ ! set message/nofacility/noidentification/noseverity/notext
	@ write sys$output "*** Processing DESCRIP.MMS"
	@ edit/tpu/nosection/nodisplay -
		/command=routine.tpu_tmp/out=descrip.mms -
		descrip.mms
	@ write sys$output "*** Processing MAKE.COM"
	@ edit/tpu/nosection/nodisplay -
		/command=routine.tpu_tmp/out=make.com -
		make.com
	@ delete routine.tpu_tmp;*
	@ set message'save_mesg'

unhackfiles :
	@ __globalresult := YES
	@ write sys$output "*** Trying to restore DESCRIP.MMS"
	@- __result = f$search("descrip.mms") .nes. ""
	@- if __result then __result = f$search("descrip.mms") .nes. ""
	@- if __result then search descrip.mms "$(FAKE_XPMLIB)descrip.mms"/output=descrip.search
	@- if __result .and. f$file("descrip.search","eof") .ne. 0 then delete descrip.mms;/log
	@- if __result .and. f$file("descrip.search","eof") .eq. 0 then -
		write sys$output "***        ... didn't need to."
	@- search descrip.mms "$(FAKE_XPMLIB)descrip.mms"/output=descrip.search
	@- if __result .and. f$file("descrip.search","eof") .ne. 0 then __globalresult := no
	@- if .not. __result .and. f$file("descrip.search","eof") .ne. 0 then -
		write sys$output "You will need to change the definition of XPMLIB in DESCRIP.MMS."
	@- delete descrip.search;*
	@ write sys$output "*** Trying to restore MAKE.COM"
	@- __result = f$search("make.com") .nes. ""
	@- if __result then __result = f$search("make.com") .nes. ""
	@- if __result then search make.com "$(FAKE_XPMLIB)make.com"/output=make.search
	@- if __result .and. f$file("make.search","eof") .ne. 0 then delete make.com;/log
	@- if __result .and. f$file("make.search","eof") .eq. 0 then -
		write sys$output "***        ... didn't need to."
	@- search make.com "$(FAKE_XPMLIB)make.com"/output=make.search
	@- if __result .and. f$file("make.search","eof") .ne. 0 then __globalresult := no
	@- if .not. __result .and. f$file("make.search","eof") .ne. 0 then -
		write sys$output "You will need to change the definition of XPMLIB in MAKE.COM."
	@- delete make.search;*
	@ write sys$output "*** Trying to restore XPM.H"
	@- __result = f$search("XPM.H") .eqs. ""
	@- if __result then rename XPM.H_NODIST XPM.H/LOG
	@- if .not. __result then -
		write sys$output "***        ... didn't need to."
	@ if .not. __globalresult then -
		write sys$output "********************************************"
	@ if .not. __globalresult then -
		write sys$output "* WARNING!!!                               *"
	@ if .not. __globalresult then -
		write sys$output "* You need to do `MMS unhackfiles' again.  *"
	@ if .not. __globalresult then -
		write sys$output "********************************************"
	@ if __globalresult then -
		write sys$output "********************************************"
	@ if __globalresult then -
		write sys$output "* There, done.  You're safe now...         *"
	@ if __globalresult then -
		write sys$output "* It's possible that some things will get  *"
	@ if __globalresult then -
		write sys$output "* recompiled if you issue MMS again, but   *"
	@ if __globalresult then -
		write sys$output "* that's less important...                 *"
	@ if __globalresult then -
		write sys$output "********************************************"

version.opt : version.c descrip.mms
	@ sear version.c "#define VERSION_ID"/out=version.tmp
	@- open/read foo version.tmp
	@- read foo line
	@- close foo
	@- delete version.tmp;*
	@ version = f$extract(0,10,f$element(1,"""",line))
	open/write foo version.opt
	write foo "NAME=""CTWM"""
	write foo "IDENT=""V''version'"""
	close foo

versions : version.opt FRC
	@- open/read foo version.opt
	@- read foo line		! the name
	@- read foo line		! the ident
	@- close foo
	@ version = f$extract(1,10,f$element(1,"""",line))
	major = f$element(0,".",version)
	minor = f$element(0,"-",f$element(1,".",version))
	update = f$element(0," ",f$element(1,"-",version))
	testword = f$element(1," ",version)
	vmsver = "0''major'''minor'"
	if major .gt. 9 then vmsver = "''major'''minor'"
	if update .nes. "-" then vmsver = "U''update'"+vmsver
	if testword .nes. " " then vmsver = vmsver+"-"+testword
	
zip.comment : versions descrip.mms
	- open/write foo zip.comment
	- write foo ""
	- write foo "CTWM ''version' -- Neat window manager for Un*x and VMS"
	- write foo ""
	- write foo "This version runs on both OpenVMS/VAX and OpenVMS/AXP."
	- write foo ""
	- close foo

zip.comment-link : versions descrip.mms
	- open/write foo zip.comment-link
	- write foo ""
	- write foo "CTWM ''version' -- Neat window manager for Un*x and VMS"
	- write foo ""
	- write foo "This version runs on both OpenVMS/VAX and OpenVMS/AXP."
	- write foo ""
	- write foo "To get the executable, just do @LINK after you extracted"
	- write foo "the files, then follow the installation instructions in"
	- write foo "README.VMS."
	- write foo ""
	- close foo
