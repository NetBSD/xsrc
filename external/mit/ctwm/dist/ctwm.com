$! put in the right path
$ progdir := program:[x_vaxbin]
$! Make the following empty if you want no welcoming picture
$ welcome_file := program:[lib.xpm]welcome.xpm		! .xwd works as well
$
$! No more customization ------------------------------------------------
$
$ if f$getsyi("CPU") .ge. 128
$ then
$     exe := axp_exe
$ else
$     exe := vax_exe
$ endif
$ ctwm:=$'progdir'ctwm.'exe'
$ ShowWelcomeSwitch = "-W"
$ if welcome_file .nes. ""
$ then
$     t = f$edit(f$parse(welcome_file,,,"type") - ".", "lowercase")
$     define ctwm_welcome_file "''t':/''welcome_file'"
$     ShowWelcomeSwitch = ""
$ endif
$ vue$suppress_output_popup
$
$restart:
$ args = ""
$ if ShowWelcomeSwitch .nes. "" then args = " """+ShowWelcomeSwitch+""""
$ if p1 .nes. "" then args = args + " """+p1+""""
$ if p2 .nes. "" then args = args + " """+p2+""""
$ if p3 .nes. "" then args = args + " """+p3+""""
$ if p4 .nes. "" then args = args + " """+p4+""""
$ if p5 .nes. "" then args = args + " """+p5+""""
$ if p6 .nes. "" then args = args + " """+p6+""""
$ if p7 .nes. "" then args = args + " """+p7+""""
$ if p8 .nes. "" then args = args + " """+p8+""""
$ set noon
$ ctwm 'args'
$ if $status .eq. 1
$ then	! We know this is a restart
$     ShowWelcomeSwitch = "-W"	! the restart does not need a welcome
$     goto restart
$ endif
