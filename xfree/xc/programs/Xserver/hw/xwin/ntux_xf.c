/* $XFree86: xc/programs/Xserver/hw/xwin/ntux_xf.c,v 1.1 2000/08/10 17:40:38 dawes Exp $ */

#include <windows.h>
#include <winioctl.h>
#include <winsvc.h>
#include <memory.h>
#include "stdio.h"
#include "ntux_ddx.h"

/* #define TEST_STANDALONE 0 */
static 	HANDLE h_xfmap = NULL;


char	*map_dx_fb( char *adr, unsigned long size )
{
	char	*retval = NULL;
	unsigned long cb;	

	VIDEO_SHARE_MEMORY	vsm = {0};
	vsm.RequestedVirtualAddress = adr;
	vsm.ViewSize = size;

	if( h_xfmap == NULL )
	{
		h_xfmap = load_driver( XFMAP_SYS, XFMAP_IMG );
		if( h_xfmap == NULL )
			return NULL;
	}

	if( DeviceIoControl( h_xfmap,
						XFMAP_PMAP,
						(LPVOID)&vsm,
						sizeof(vsm),
						(LPVOID)&vsm,
						sizeof(vsm),
						&cb,
						NULL
						)
		 )
	{
		retval = (char*)vsm.RequestedVirtualAddress;
	}
	return retval;
}

#ifndef TEST_STANDALONE
static 
#endif
int main( int argc, char **argv )
{
	char *buf = NULL;
	char *buf2 = NULL;

	UnloadDeviceDriver( XFMAP_SYS );
	h_xfmap = load_driver( XFMAP_SYS, XFMAP_IMG );

	fprintf( stderr, "h_xfmap = %08x\n", h_xfmap );

	buf = (char*)malloc( 100000 );
	if( buf )
	{
		buf2 = map_dx_fb( buf, 100000 );
		free( buf );
	}


	if( h_xfmap )
		unload_driver( h_xfmap, XFMAP_SYS );
}




/*
* driver load/unload routines
*/
BOOL InstallDriver( SC_HANDLE SchSCManager, LPCSTR DriverName, LPCSTR ServiceExe )
{
    SC_HANDLE  schService;


    schService = CreateService( SchSCManager,          // SCManager database
                                DriverName,           // name of service
                                DriverName,           // name to display
                                SERVICE_ALL_ACCESS,    // desired access
                                SERVICE_KERNEL_DRIVER, // service type
                                SERVICE_DEMAND_START,  // start type
                                SERVICE_ERROR_NORMAL,  // error control type
                                ServiceExe,            // service's binary
                                NULL,                  // no load ordering group
                                NULL,                  // no tag identifier
                                NULL,                  // no dependencies
                                NULL,                  // LocalSystem account
                                NULL                   // no password
                                );

    if ( schService == NULL )
        return FALSE;

    CloseServiceHandle( schService );
	return TRUE;
}

BOOL StartDriver( SC_HANDLE SchSCManager, LPCSTR DriverName )
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );
    if ( schService == NULL )
        return FALSE;

    ret = StartService( schService, 0, NULL )
       || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;

    CloseServiceHandle( schService );

    return ret;
}

BOOL OpenDevice( LPCSTR DriverName, HANDLE * lphDevice )
{
    char	completeDeviceName[64];
    HANDLE  hDevice;


    sprintf( completeDeviceName, TEXT("\\\\.\\%s"), DriverName );

    hDevice = CreateFile( completeDeviceName,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL
                          );
    if ( hDevice == ((HANDLE)-1) )
        return FALSE;

	/* If user wants handle, give it to them.  Otherwise, just close it. */
	if ( lphDevice )
		*lphDevice = hDevice;
	else
	    CloseHandle( hDevice );
	return TRUE;
}

BOOL StopDriver( SC_HANDLE SchSCManager, LPCSTR DriverName )
{
    SC_HANDLE       schService;
    BOOL            ret;
    SERVICE_STATUS  serviceStatus;

    schService = OpenService( SchSCManager, DriverName, SERVICE_ALL_ACCESS );
    if ( schService == NULL )
        return FALSE;

    ret = ControlService( schService, SERVICE_CONTROL_STOP, &serviceStatus );

    CloseServiceHandle( schService );

    return ret;
}

BOOL RemoveDriver( SC_HANDLE SchSCManager, LPCSTR DriverName )
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( SchSCManager,
                              DriverName,
                              SERVICE_ALL_ACCESS
                              );

    if ( schService == NULL )
        return FALSE;

    ret = DeleteService( schService );

    CloseServiceHandle( schService );

    return ret;
}

BOOL LoadDeviceDriver( const char * Name, const char * Path, HANDLE * lphDevice )
{
	SC_HANDLE	schSCManager;
	BOOL		okay;

	schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	// Ignore success of installation: it may already be installed.
	InstallDriver( schSCManager, Name, Path );

	// Ignore success of start: it may already be started.
	StartDriver( schSCManager, Name );

	// Do make sure we can open it.
	okay = OpenDevice( Name, lphDevice );

 	CloseServiceHandle( schSCManager );

	return okay;
}

BOOL UnloadDeviceDriver( const char * Name )
{
	BOOL bResult = TRUE;
	SC_HANDLE	schSCManager;
	
	schSCManager = OpenSCManager(	NULL,                 // machine (NULL == local)
                              		NULL,                 // database (NULL == default)
									SC_MANAGER_ALL_ACCESS // access required
								);

	if( !schSCManager )
		return FALSE;

	bResult &= StopDriver( schSCManager, Name );
	bResult &= RemoveDriver( schSCManager, Name );
	 
	bResult &= CloseServiceHandle( schSCManager );

	return bResult;
}

HANDLE	WINAPI	LoadDriver( LPCSTR szPath, LPCSTR Name )
{
	HANDLE	sys_handle;



	if(	!LoadDeviceDriver( Name, szPath, &sys_handle ) )
		return NULL;
	else
		return sys_handle;
}

BOOL	WINAPI	UnloadDriver( HANDLE hDriver, LPCSTR szName )
{
	if( !CloseHandle( hDriver ) )
		return FALSE;
	return UnloadDeviceDriver( szName );
}

HANDLE	WINAPI load_driver( 
	char *driver_name,		/* driver name, example "xfmap" */
	char *driver_bin		/* driver binary, example "xfmap.sys" */
	)
{
	char Path[ MAX_PATH ];
	GetCurrentDirectory( sizeof Path, Path );

	sprintf( Path+lstrlen(Path), TEXT("\\%s"), driver_bin );
	return LoadDriver( Path, driver_name );
}

BOOL	WINAPI unload_driver( 
	HANDLE h_driver,		/* handle to loaded driver */
	char *driver_name		/* driver name, example : "xfmap" */
	)
{
	return UnloadDriver( h_driver, driver_name );
}
