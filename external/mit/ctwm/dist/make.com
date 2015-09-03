$! Make.com for building CTWM
$ SAVE_VERIFY='F$VERIFY(0)
$!
$!	Compile the CTWM Window Manager
$!
$!--------------------------- Customization -------------------------------
$!  Define logicals pointing to the needed directories
$!
$! this is where the Xmu include files are held,
$!  usually in sys$sysroot:[decw$include.xmu]
$ define/nolog x11xmu sys$sysroot:[decw$include.xmu]
$
$! the file spec. of the Xmu library.
$ define/nolog x11xmulib "SYS$SHARE:DECW$XMULIBSHR/share"
$
$! where the xpm object library is.
$ define/nolog xpmlib pd:[src.xpm-3_4e]
$
$! ctwm exit style:
$!	0 - Just exit with error code 20.
$!	1 - run sys$system:decw$endsession.exe on exit
$ EXIT_ENDSESSION = 0
$
$!------------------------ End of Customization ---------------------------
$!
$ if p1 .Eqs. "CLEAN" then goto clean
$ if p1 .Eqs. "CLOBBER" then goto clobber
$
$! Sanity check!				suggested by Michael Lemke
$ if f$search("XPM.H") .eqs. ""
$ then
$     write sys$output "Hey, you there!  Read README.VMS!  It says that you have to copy XPM.H"
$     write sys$output "from somewhere, like a XPM library distribution!  I can't work without it,"
$     write sys$output "so I'll quit for now."
$     exit
$ endif
$ FAKE_XPMLIB:=you:[must.define.this.in]
$ l=f$trnlnm("XPMLIB")
$ if l - (FAKE_XPMLIB+"make.com") .nes. l
$ then
$     write sys$output "You must change the definition of XPMLIB in MAKE.COM."
$     exit
$ endif
$
$ FLEX = FLEX
$ Set Symbol/Scope=NoGlobal
$ if p1 .Eqs. "LINK"
$ then
$     link_options = "/NOTRACE"
$     __axp = f$getsyi("CPU") .ge. 128
$     __decc = f$search("SYS$SHARE:DECC$SHR.EXE") .nes. ""
$
$     EXT = "VAX_"
$     if __axp then EXT = "AXP_"
$     OPT_EXT = "DECC_OPT"
$     CLIB = ""
$     if .not. __axp .and (.not. __decc .or. f$search("*.''EXT'OBJ") .eqs. "")
$     then
$	  OPT_EXT = "VAXC_OPT"
$	  EXT = EXT + "VAXC_"
$	  CLIB := SYS$SHARE:VAXCRTL/SHARE
$     endif
$     if f$search("*.''EXT'OBJ") .eqs. ""
$     then
$	  write sys$output "Sorry, but the object files I need are missing."
$	  write sys$output "You need to compile them first.  Read README.VMS"
$	  write sys$output "for further instructions"
$	  exit
$     endif
$     Goto Link
$ endif
$!
$! Establish the Compiling Environment
$!
$ COPTS = F$TrnLnm("COPTS")
$ extra_defs = ""
$ CLIB = ""
$ Cpu_Model = F$GetSYI("HW_MODEL")
$ If Cpu_Model .gt. 1024	! Cross compiling
$ Then
$	CC := CC/PREFIX=ALL/L_DOUBLE=64
$	OPT_EXT = "DECC_OPT"
$	EXT = "AXP_"
$ Else
$	write sys$error "Ignore the possible %DCL-W-IVQUAL or %CC-W-EMPTYFILE"
$	CC := CC/PREFIX=ALL
$	OPT_EXT = "DECC_OPT"
$	EXT = "VAX_"
$	CLIB = ""
$	Set Noon
$	CC NL:/OBJ=NL:
$	If $status .eq. %x00038240
$	Then	! no DEC C or the standard is to run VAX C
$		CC := CC/DECC/PREFIX=ALL
$		CC NL:/OBJ=NL:
$		If $status .eq. %x00038240
$		Then	! no DEC C, meaning VAX C without the /VAXC qualifier
$			CC := CC
$			OPT_EXT = "VAXC_OPT"
$			EXT = "VAX_VAXC_"
$			CLIB = "SYS$SHARE:VAXCRTL/SHARE"
$			extra_defs = extra_defs + ",NO_LOCALE"
$		Endif
$	Endif
$	write sys$error "There, you can look again now..."
$ Endif
$ 
$ ! This is a pure hack to circumvent a bug in the file provided by
$ ! Digital.  This is so ugly we don't want the user to see what
$ ! really happens.
$ ! But perhaps we'll give him just a little hint, huh?  :-)
$ write sys$output "Doing some hackery with XWDFile.h...  Sit tight"
$ DEP_XWDFILE = ""
$ if f$search("DECW$UTILS:XWDFILE.H") .nes. "" 
$ then
$     open/write foo xwdfile.tpu_tmp
$     write foo "input_file:=GET_INFO(COMMAND_LINE, ""file_name"");"
$     write foo "main_buffer := CREATE_BUFFER (""main"", input_file);"
$     write foo "position (beginning_of (main_buffer));"
$     write foo "loop"
$     write foo "	r := search_quietly (""""""copyright.h"""""", FORWARD);"
$     write foo "	EXITIF r = 0;"
$     write foo "	if beginning_of(r) <> end_of(r)"
$     write foo "	then"
$     write foo "		erase (r);"
$     write foo "		position (r);"
$     write foo "		copy_text (""<decw$utils/copyright.h>"");"
$     write foo "	endif;"
$     write foo "	position (end_of (r));"
$     write foo "endloop;"
$     write foo "position (beginning_of (main_buffer));"
$     write foo "loop"
$     write foo "	r := search_quietly (LINE_BEGIN + ""struct {"", FORWARD);"
$     write foo "	EXITIF r = 0;"
$     write foo "	if beginning_of(r) <> end_of(r)"
$     write foo "	then"
$     write foo "		erase (r);"
$     write foo "		position (r);"
$     write foo "		copy_text (""typedef struct {"");"
$     write foo "	endif;"
$     write foo "	position (end_of (r));"
$     write foo "endloop;"
$     write foo "write_file(main_buffer, get_info (command_line, ""output_file""));"
$     write foo "quit;"
$     close foo
$     save_mesg = f$environment("MESSAGE")
$     !set message/nofacility/noidentification/noseverity/notext
$     edit/tpu/nosection/nodisplay -
		/command=xwdfile.tpu_tmp/out=xwdfile.h -
		decw$utils:xwdfile.h
$     purge xwdfile.h
$     delete xwdfile.tpu_tmp;*
$     set message'save_mesg'
$     DEP_XWDFILE = "XWDFILE.H"
$     write sys$output "There, done."
$ endif
$
$ if f$search("xwdfile.h") .nes. "" then -
     extra_defs = extra_defs + ",HAVE_XWDFILE_H"
$!
$!  Get the compiler options via the logical name COPTS
$!
$ cc_options = COPTS + -
	       "/define=(VMS,XPM,C_ALLOCA,""emacs"",BLOCK_INPUT," + -
	       "EXIT_ENDSESSION=''EXIT_ENDSESSION'''extra_defs')" + -
	       "/OBJ=.''EXT'OBJ"
$!
$!  Get the linker options via the logical name LOPTS
$!
$ link_options = f$trnlnm("LOPTS")
$!
$!  Compile the "C" files
$!
$! procedure target	command 			depends upon
$! CALL MAKE FILE.OBJ	"CC ''cc_options' FILE.C"	FILE.C
$!
$ write sys$output "Building LEX.C from LEX.L using flex"
$ CALL MAKE LEX.C		"FLEX LEX.L" -
       LEX.L
$ if f$search("LEXYY.C") .nes. ""
$ then
$     CALL MAKE LEX.C -
	   "RENAME LEXYY.C LEX.C/LOG" -
	   LEXYY.C
$ else
$     CALL MAKE LEX.C -
	   "COPY LEX.C_VMS LEX.C/LOG" -
	   LEX.C_VMS
$ endif
$ write sys$output -
	"Building GRAM.C and GRAM.H from GRAM.Y using bison/def/yacc"
$ CALL MAKE Y_TAB.C		"BISON /DEF/YACC GRAM.Y"	GRAM.Y
$ if f$search("Y_TAB.C") .nes. ""
$ then
$     CALL MAKE GRAM.C -
	   "RENAME Y_TAB.C GRAM.C/LOG; RENAME Y_TAB.H GRAM.H/LOG" -
	   Y_TAB.C Y_TAB.H
$ else
$     CALL MAKE GRAM.C -
	   "COPY GRAM.C_VMS GRAM.C/LOG; COPY GRAM.H_VMS GRAM.H/LOG" -
	   GRAM.C_VMS GRAM.H_VMS
$ endif
$ CALL MAKE GRAM.H		-
       "BISON /DEF/YACC GRAM.Y; DELETE Y_TAB.C.*" -
       GRAM.Y
$ if f$search("Y_TAB.H") .nes. ""
$ then
$     CALL MAKE GRAM.H -
	   "RENAME Y_TAB.H GRAM.H/LOG" -
	   Y_TAB.H
$ else
$     CALL MAKE GRAM.H -
	   "COPY GRAM.H_VMS GRAM.H/LOG" -
	   GRAM.H_VMS
$ endif
$ write sys$output -
	"Building DEFTWMRC.C from SYSTEM.CTWMRC using GENDEFTWMRC.COM"
$ CALL MAKE DEFTWMRC.C		"@GENDEFTWMRC.COM"		SYSTEM.CTWMRC -
       GENDEFTWMRC.COM
$ write sys$output "Compiling CTWM sources with CC=",CC
$ CALL MAKE ADD_WINDOW.'EXT'OBJ "CC 'cc_options' ADD_WINDOW.C"	ADD_WINDOW.C -
       "TWM.H ADD_WINDOW.H UTIL.H RESIZE.H PARSE.H GRAM.H LIST.H EVENTS.H" -
       "MENUS.H SCREEN.H ICONS.H ICONMGR.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE CLICKTOFOCUS.'EXT'OBJ-
			"CC 'cc_options' CLICKTOFOCUS.C" CLICKTOFOCUS.C -
       "CLICKTOFOCUS.H TWM.H UTIL.H SCREEN.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE CTWM.'EXT'OBJ  	"CC 'cc_options' CTWM.C"	CTWM.C -
       "TWM.H ADD_WINDOW.H GC.H PARSE.H VERSION.H MENUS.H EVENTS.H UTIL.H" -
       "GRAM.H SCREEN.H ICONS.H ICONMGR.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE CURSOR.'EXT'OBJ  	"CC 'cc_options' CURSOR.C"	CURSOR.C -
       "TWM.H SCREEN.H UTIL.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE DEFTWMRC.'EXT'OBJ  	"CC 'cc_options' DEFTWMRC.C"	DEFTWMRC.C
$ CALL MAKE EVENTS.'EXT'OBJ  	"CC 'cc_options' EVENTS.C"	EVENTS.C -
       "TWM.H ADD_WINDOW.H MENUS.H EVENTS.H RESIZE.H PARSE.H GRAM.H UTIL.H" -
       "SCREEN.H ICONS.H ICONMGR.H VERSION.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE GC.'EXT'OBJ  	"CC 'cc_options' GC.C"	GC.C -
       "TWM.H UTIL.H GRAM.H SCREEN.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE GRAM.'EXT'OBJ  	"CC 'cc_options' GRAM.C"	GRAM.C -
       "TWM.H MENUS.H ICONS.H ADD_WINDOW.H LIST.H UTIL.H SCREEN.H PARSE.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE ICONMGR.'EXT'OBJ  	"CC 'cc_options' ICONMGR.C"	ICONMGR.C -
       "TWM.H UTIL.H PARSE.H SCREEN.H RESIZE.H ADD_WINDOW.H SICONIFY.BM" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE ICONS.'EXT'OBJ  	"CC 'cc_options' ICONS.C"	ICONS.C -
       "TWM.H SCREEN.H ICONS.H GRAM.H LIST.H PARSE.H UTIL.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE LEX.'EXT'OBJ  	"CC 'cc_options' LEX.C"	LEX.C -
       "GRAM.H PARSE.H"
$ CALL MAKE LIST.'EXT'OBJ  	"CC 'cc_options' LIST.C"	LIST.C -
       "TWM.H SCREEN.H GRAM.H UTIL.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE MENUS.'EXT'OBJ  	"CC 'cc_options' MENUS.C"	MENUS.C -
       "TWM.H GC.H MENUS.H RESIZE.H EVENTS.H UTIL.H PARSE.H GRAM.H SCREEN.H" -
       "ICONS.H VMS_CMD_SERVICE.H VERSION.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE PARSE.'EXT'OBJ  	"CC 'cc_options' PARSE.C"	PARSE.C -
       "TWM.H SCREEN.H MENUS.H UTIL.H GRAM.H PARSE.H VERSION.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE RESIZE.'EXT'OBJ  	"CC 'cc_options' RESIZE.C"	RESIZE.C -
       "TWM.H PARSE.H UTIL.H RESIZE.H ADD_WINDOW.H SCREEN.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE UTIL.'EXT'OBJ  	"CC 'cc_options' UTIL.C"	UTIL.C -
       "TWM.H UTIL.H GRAM.H SCREEN.H ICONS.H XPM.H SICONIFY.BM" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H ''DEP_XWDFILE'"
$ CALL MAKE VERSION.'EXT'OBJ  	"CC 'cc_options' VERSION.C"	VERSION.C
$ CALL MAKE VSCREEN.'EXT'OBJ  	"CC 'cc_options' VSCREEN.C"	VSCREEN.C -
       "TWM.H SCREEN.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE WORKMGR.'EXT'OBJ  	"CC 'cc_options' WORKMGR.C"	WORKMGR.C -
       "TWM.H UTIL.H PARSE.H SCREEN.H ICONS.H RESIZE.H ADD_WINDOW.H EVENTS.H" -
       "GRAM.H CLICKTOFOCUS.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE WINDOWBOX.'EXT'OBJ 	"CC 'cc_options' WINDOWBOX.C"	WINDOWBOX.C -
       "TWM.H SCREEN.H WINDOWBOX.H" -
       "LIST.H MENUS.H ICONMGR.H VSCREEN.H WORKMGR.H"
$ CALL MAKE VMS_CMD_SERVICES.'EXT'OBJ "CC 'cc_options' VMS_CMD_SERVICES.C" -
       VMS_CMD_SERVICES.C VMS_CMD_SERVICES.H LNM.H
$ CALL MAKE LNM.'EXT'OBJ	"CC 'cc_options' LNM.C"	LNM.C
$ CALL MAKE ALLOCA.'EXT'OBJ	"CC 'cc_options' ALLOCA.C"	ALLOCA.C
$!
$ Link:		! Link the executable.  If all object files aren't here,
$		! I'm deeply sorry
$ EXE_EXT = EXT - "VAXC_"
$ OPTION := CTWM.'OPT_EXT'
$ OPTION_CMD := ,CTWM.'OPT_EXT'/OPT
$
$ CALL MAKE VERSION.OPT		"CALL MAKEVEROPT VERSION.OPT"	VERSION.C -
       MAKE.COM
$ CALL MAKE 'OPTION'		"CALL MAKELIBOPT ''OPTION' ''F$TRNLNM("X11XMULIB")' ''CLIB'" -
       MAKE.COM
$ CALL MAKE OBJS.'EXT'OPT	"CALL MAKEOBJOPT OBJS.''EXT'OPT *.''EXT'OBJ" -
       *.'EXT'OBJ MAKE.COM
$ write sys$output "Building CTWM image"
$ CALL MAKE CTWM.'EXE_EXT'EXE	-
       "LINK ''link_options'/EXE=CTWM.''EXE_EXT'EXE OBJS.''EXT'OPT/OPT,VERSION.OPT/OPT ''OPTION_CMD'" -
       OBJS.'EXT'OPT 'OPTION' VERSION.OPT
$!
$ deassign xpmlib
$ deassign x11xmu
$
$ write sys$output "All done"
$!
$ exit
$!
$ Clobber:	! Delete executables, Purge directory and clean up object files and listings
$ Delete/noconfirm/log *.exe;*
$!
$ Clean:	! Purge directory, clean up object files and listings
$ Purge
$ Delete/noconfirm/log *.lis;*
$ Delete/noconfirm/log *.obj;*
$!
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$	Argument = P'arg
$	If Argument .Eqs. "" Then Goto NoWork
$	El=0
$Loop2:
$	File = F$Element(El," ",Argument)
$	If File .Eqs. " " Then Goto Endl
$	AFile = ""
$Loop3:
$	OFile = AFile
$	AFile = F$Search(File)
$	If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$	If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$	Goto Loop3
$NextEL:
$	El = El + 1
$	Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$NoWork: 
$ Goto Exit
$
$BailOut:
$ tmp = $status ! 'F$Verify(0)
$ If V Then Set Verify
$ write sys$error "Exiting with status code ",tmp
$ Exit tmp
$KillMe: 
$ If V Then Set Verify
$ write sys$error -
	"Exiting because user pressed Ctrl-Y (we do this the ugly way)"
$ Exit 20
$Makeit:
$ On Error Then Goto BailOut
$ On Control_y Then Goto KillMe
$ _Index = 0
$Makeit2:
$ _Command = F$Edit(F$Element(_Index,";",P2),"TRIM")
$ _Index = _Index + 1
$ If _Command .nes. ";"
$ Then
$     If F$Edit(F$Extract(0,5,_Command),"UPCASE") .nes. "CALL " Then Set Verify
$     '_Command
$     VV='F$Verify(0)
$     Goto Makeit2
$ Endif
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
$
$MAKELIBOPT: SUBROUTINE   !SUBROUTINE TO BUILD A LIBRARY OPTIONS FILE
$ V = 'F$Verify(0)
$ goto Makeit2
$BailOut2:
$ tmp = $status ! 'F$Verify(0)
$ close foo
$ If V Then Set Verify
$ write sys$error "Exiting with status code ",tmp
$ Exit tmp
$Makeit2:
$ On Error Then Goto BailOut2
$ On Control_y Then Goto BailOut2
$ write sys$output "Building the library option file ",P1
$ if p3 .nes. "" then write foo p3
$ __result := XPMLIB:XPM.OLB
$ if f$search(__result) .eqs. "" then -
     __result := XPMLIB:LIBXPM.OLB
$ if EXT .eqs. "VAX_VAXC_" .and. -
     f$search("XPMLIB:LIBXPM.OLB_VAXC") .nes. "" then -
     __result := XPMLIB:LIBXPM.OLB_VAXC
$ if EXT .eqs. "VAX_" .and. -
     f$search("XPMLIB:LIBXPM.OLB_DECC") .nes. "" then -
     __result := XPMLIB:LIBXPM.OLB_DECC
$ if EXT .eqs. "AXP_" .and. -
     f$search("XPMLIB:LIBXPM.OLB_AXP") .nes. "" then -
     __result := XPMLIB:LIBXPM.OLB_AXP
$ if f$search(__result) .eqs. "" then -
     libr/list=nl: '__result'
$ open/write foo 'P1'
$ write foo __result,"/LIB"
$ write foo p2
$ write foo "SYS$SHARE:DECW$XTSHR/SHARE"
$ write foo "SYS$SHARE:DECW$XEXTLIBSHR/SHARE"
$ write foo "SYS$SHARE:DECW$XLIBSHR/SHARE"
$ VV='F$Verify(0)
$Exit2:
$ close foo
$ If V Then Set Verify
$ENDSUBROUTINE
$
$MAKEVEROPT: SUBROUTINE   !SUBROUTINE TO BUILD AN IDENTITY OPTIONS FILE
$ V = 'F$Verify(0)
$ goto Makeit3
$BailOut3:
$ tmp = $status ! 'F$Verify(0)
$ close foo
$ If V Then Set Verify
$ write sys$error "Exiting with status code ",tmp
$ Exit tmp
$Makeit3:
$ On Error Then Goto BailOut3
$ On Control_y Then Goto BailOut3
$ write sys$output "Building the identity option file VERSION.OPT
$ sear version.c "*VersionNumber"/out=version.tmp
$ open/read foo version.tmp
$ read foo line
$ close foo
$ delete version.tmp;*
$ version = f$extract(0,10,f$element(1,"""",line))
$ open/write foo 'p1'
$ write foo "NAME=""CTWM"""
$ write foo "IDENT=""V''version'"""
$ VV='F$Verify(0)
$Exit3:
$ close foo
$ If V Then Set Verify
$ENDSUBROUTINE
$
$MAKEOBJOPT: SUBROUTINE   !SUBROUTINE TO BUILD AN IDENTITY OPTIONS FILE
$ V = 'F$Verify(0)
$ goto OBJ_Makeit
$OBJ_BailOut:
$ tmp = $status ! 'F$Verify(0)
$ close foo
$ If V Then Set Verify
$ write sys$error "Exiting with status code ",tmp
$ Exit tmp
$OBJ_Makeit:
$ On Error Then Goto OBJ_BailOut
$ On Control_y Then Goto OBJ_BailOut
$ write sys$output "Building object option file ",p1
$ open/write foo 'p1'
$ OFile = ""
$OBJ_Loop:
$ AFile = F$Search(P2)
$! DEBUG: P2 = 'P2'
$! DEBUG: AFile = 'AFile'
$ if OFile .nes. ""
$ then
$     if AFile .nes. ""
$     then
$	  write foo OFile,",-"
$     else
$	  write foo OFile
$     endif
$ endif
$ OFile = F$Parse(AFile,,,"NAME") + F$Parse(AFile,,,"TYPE")
$ if AFile .nes. "" then goto OBJ_Loop
$ VV='F$Verify(0)
$OBJ_Exit:
$ close foo
$ If V Then Set Verify
$ENDSUBROUTINE
