/* OS/2 REXX script to create nonexisting directories
 * run with 'added dirlist_file'
 * where dirlist_file is a XFree86 added-* file
 *
 * $XFree86$
 */

file = arg(1)
linein(file,1,0)
curdir = directory()
do while lines(file)=1
  fs = strip(filespec("path",linein(file)),'t','/')
  newdir = directory(fs)
  if newdir='' then do
    say 'create directory='fs
    call directory(curdir)
    call 'xc\config\util\mkdirhier.cmd' fs
  end 
  else do
    say 'found directory='newdir
    call directory(curdir)
  end
end
