/* REXX */
/* $XFree86: xc/config/util/buildos2.cmd,v 3.4 1996/03/10 11:50:46 dawes Exp $
 * this file is supposed to run from the xc/ dir.
 * you must copy it manually to there before using. It is just here
 * in order not to be in the root dir.
 *
 * copy some essential files to a location where we find them again
 */
'@echo off'
env = 'OS2ENVIRONMENT'
'copy config\util\indir.cmd \ > nul 2>&1'
'copy config\util\mkdirhier.cmd \ > nul 2>&1'
'copy config\imake\imakesvc.cmd \ > nul 2>&1'

IF \(exists('Makefile.os2')) THEN 'COPY Makefile Makefile.os2 >nul 2>&1'

CALL VALUE 'GCCOPT','-pipe',env
CALL VALUE 'EMXLOAD',5,env
CALL VALUE 'MAKEFLAGS','--no-print-directory',env
'emxload x11make.exe gcc.exe rm.exe mv.exe'

'x11make MAKE=x11make SHELL= MFLAGS="MAKE=x11make CC=gcc BOOTSTRAPCFLAGS=-DBSD43 SHELL= " World.OS2 2>&1 | tee buildxc.log'
/* cleanup the mess */
/* del \indir.cmd 
 * del \mkdirhier.cmd
 * del \imakesvc.cmd
 * del \imake.exe
 * del \makedepend.exe
 */

EXIT

/* returns 1, if file exists */
exists:
'DIR "'arg(1)'" >nul 2>&1'
IF rc = 0 THEN return 1
RETURN 0
