/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/solx86/solx86_misc.c,v 1.2 1998/07/25 16:57:05 dawes Exp $ */

/*
 * Copyright 1995-1997 by The XFree86 Project, Inc
 */

#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_SECONDS	60	
#define USEC_IN_SEC     (unsigned long)1000000

int xf86_solx86usleep(unsigned long);
static void xf86_solx86sleep(int);

/*
 * xf86_solx86usleep() - Solaris 2.1 x86 does not have a suitable
 * 			 replacement (SYSV) for usleep. Although
 *			 usleep exists in the BSD compatiblity libs
 *			 I dont want to use those libs if possible.
 *
 *			 Doug Anson
 *			 danson@lgc.com
 */
int
xf86_solx86usleep(unsigned long usec)
{
    int		      retval = 0;
    struct itimerval  naptime;
    struct itimerval  savetime;
    unsigned long     useconds = 0;
    unsigned long     seconds = 0;
    int		      i;
    unsigned long     tmp;
	
/*
 * WHY DOESN'T THIS SIMPLY DO A select() WITH NO FILE DESCRIPTORS?
 */

    /* this time will allow a max of MAX_SECONDS seconds sleeping */
    for(i=MAX_SECONDS;i>=0;--i)
    {
	tmp = (unsigned long)((unsigned long)(i)*USEC_IN_SEC);
	if (tmp <= usec)
	{
	    seconds = i;
	    if (i == MAX_SECONDS)
		useconds = 0;
	    else
	        useconds = (unsigned long)(usec - tmp);
	    i = -1;
	}
    }

    /* get the current time */
    if ((retval=getitimer(ITIMER_REAL,&savetime)) == 0)
    {
    	/* set the itimer to reflect requested time to sleep */
	naptime.it_value.tv_sec = savetime.it_value.tv_sec + seconds;
    	naptime.it_value.tv_usec = savetime.it_value.tv_usec + useconds;
    
    	/* specify a one-shot clock */
    	naptime.it_interval.tv_usec = 0;
    	naptime.it_interval.tv_sec = 0;

	/* redisposition SIGALRM */
	signal(SIGALRM,xf86_solx86sleep);

    	/* use SIGLARM */
    	if ((retval=setitimer(ITIMER_REAL,&naptime,NULL)) == 0)
		/* now just pause */
		retval = pause();

	/* restore the timer */
	retval = setitimer(ITIMER_REAL,&savetime,NULL);

	/* restore the SIGALRM disposition */
	signal(SIGALRM,SIG_DFL);
    }

    /* return the return value */
    return retval;
}

/*
 * xf86_solx86sleep() - This function is a NOP disposition for 
 *			the SIGALRM that is used to implement 
 *			usleep() in Solaris 2.1 x86. 
 *
 *			Doug Anson
 *			danson@lgc.com
 */
static void
xf86_solx86sleep(int signo)
{
    /* do nothing */
    return;
}
