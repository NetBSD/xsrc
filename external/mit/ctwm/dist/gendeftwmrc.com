$	__save_ver = 'f$verify(0)
$! GENDEFTWMRC.COM -- Generates a new DEFTWMRC.C from SYSTEM.CTWMRC
$!
$	SET SYMBOL/SCOPE=NOGLOBAL
$!	DELETE DEFTWMRC.C;*
$	OPEN/WRITE DEFTWMRC DEFTWMRC.C
$	WRITE DEFTWMRC "/* "
$	WRITE DEFTWMRC -
	      " * This file is generated automatically from the default"
$	WRITE DEFTWMRC -
	      " * twm bindings file system.ctwmrc by the twm Imakefile."
$	WRITE DEFTWMRC " */"
$	WRITE DEFTWMRC ""
$	WRITE DEFTWMRC "char *defTwmrc[] = {"
$	OPEN/READ CTWMRC SYSTEM.CTWMRC
$ LOOP_SYSTEM: 
$	READ/END=LOOP_SYSTEM_END/ERR=LOOP_SYSTEM_END CTWMRC LINE
$	IF F$EXTRACT(0,1,LINE) .EQS. "#" THEN GOTO LOOP_SYSTEM
$	RESULT = ""
$ LOOP_QUOTE: 
$	E = F$ELEMENT(0,"""",LINE)
$	RESULT = RESULT + E
$	LINE = LINE - E
$	IF LINE .NES. ""
$	THEN
$	    LINE = LINE - """"
$	    RESULT = RESULT + "\"""
$	    GOTO LOOP_QUOTE
$	ENDIF
$	WRITE DEFTWMRC "    """,RESULT,""","
$	GOTO LOOP_SYSTEM
$ LOOP_SYSTEM_END:
$	CLOSE CTWMRC
$	WRITE DEFTWMRC "    (char *) 0 };"
$	CLOSE DEFTWMRC
$	EXIT 1+0*f$verify(__save_ver)
