! $Xorg: xdm-conf.cpp,v 1.3 2000/08/17 19:54:17 cpqbld Exp $
!
!
!
!
! $XFree86: xc/programs/xdm/config/xdm-conf.cpp,v 1.9 2001/11/25 12:49:19 herrb Exp $
!
DisplayManager.errorLogFile:	XDMLOGDIR/xdm-errors
DisplayManager.pidFile:		XDMPIDDIR/xdm-pid
DisplayManager.keyFile:		XDMDIR/xdm-keys
DisplayManager.servers:		XDMDIR/Xservers
DisplayManager.accessFile:	XDMDIR/Xaccess
DisplayManager.willing:		SU nobody -c XDMDIR/Xwilling
! All displays should use authorization, but we cannot be sure
! X terminals will be configured that way, so by default
! use authorization only for local displays :0, :1, etc.
DisplayManager._0.authorize:	true
DisplayManager._1.authorize:	true
! The following three resources set up display :0 as the console.
DisplayManager._0.setup:	XDMDIR/Xsetup_0
DisplayManager._0.startup:	XDMDIR/GiveConsole
DisplayManager._0.reset:	XDMDIR/TakeConsole
!
DisplayManager*resources:	XDMDIR/Xresources
DisplayManager*session:		XDMDIR/Xsession
DisplayManager*authComplain:	true
#ifdef XPM
! this is a new line Caolan, 9312811@ul.ie
DisplayManager*loginmoveInterval:      10
#endif /* XPM */
! SECURITY: do not listen for XDMCP or Chooser requests
! Comment out this line if you want to manage X terminals with xdm
DisplayManager.requestPort:	0
