/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: Timer.c,v 1.11 94/04/17 21:01:33 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/* UNIX/DYNIX library for X Server tests.  
 *
 * Contains routines useful for isolating operating system dependencies.
 * DYNIX routines should be identical to UNIX in general.  VMS or other
 * operating systems will need an equivalent library.
 * Set_Timer, Get_Timer, and Stop_Timer handle a set of interval timers.
 * NOTE:  The implementation of these timers uses the interval timer and 
 * SIGALRM.  They are thus not available for other purposes, such as sleep(3).
 */

#include "XstlibInt.h"
#include <signal.h>

/*
 * Data structures and variables
 */

/*
 * "head" is the head of a linked list of timers.  it contains the "current"
 * time and points to the first timer in the list.  "Current" time is zero
 * until the first timer is set, at which time it starts getting incremented 
 * by 1 every second. The list is kept in order, with the timer having the
 * least time left to go being first after the head.
 */

static struct timer {
    int     timer;		/* timer id */
    int     time;		/* time until expiration */
    void   (*routine) ();	/* routine to call at expiration */
    struct timer   *next;	/* keeps a linked list in expiration order */
}                   head;

static  init = 0;		/* Have we set the ALRM signal handler? */
static  inwork = 0;		/* Are we fussing with the queue? */

/*ARGSUSED*/
static void
dispatch (sig)
int 	sig;
{
    struct timer   *next = NULL;

    head.time++;
    signal(SIGALRM,dispatch);		/* need to re-establish handler */

 /* We'd rather expire this timer a second late than be messing with the
    queue while another routine is also */
    if ((inwork) || (head.next == NULL)) {
	alarm(1);			/* make sure we wake again */
	return;
    }

    next = head.next;
    while ((next != NULL) && (head.time >= next -> time)) {
	head.next = next -> next;	/* delink this entry */
	(*(next -> routine)) ();
	free ((char *) next);
	next = head.next;
    }
    if (head.next != NULL) {
	alarm(1);
    }
}

/* 
 * This function sets the signal handler for SIGALRM signals.
 * This is done on initialisation in Set_Timer(), and 
 * should also be called directly after tet_fork() when it has
 * already been done BEFORE the fork(). This only affects 
 * test cases which use tpfontstartup() at present.
 */

int
Set_Init_Timer()
{
    Log_Debug("Set_Init_Timer called");
    if (signal (SIGALRM, dispatch) == (void (*)())-1) {
        Log_Del ("SIGNAL FAILED in Set_Timer, errno=%d\n", errno);
        return (-1);
    }
    head.next = NULL;
    return(0);
}

int
Set_Timer (timer, time, routine)
    int     timer;       /* unique non-zero timer id */
    int     time;        /* number of seconds until expiration */
    void (*routine) ();  /* routine to call when the timer expires */
{
    struct timer   *prev = NULL; /* pointer to previous timer on list */
    struct timer   *next = NULL; /* pointer to next timer on list */

    if (timer <= 0) {		/* 	non-zero, positive ids only */
	return (-1);
    }

    /* initialization: set signal handler, nothing on the list */

    Log_Debug("Set_Timer called");
    if (!init) {
	if(Set_Init_Timer() != 0)
		return(-1);
	init = 1;
    }

    time += head.time; /* assumes head of the list has the "current" time */

    if (head.next == NULL) {
	alarm(1);		/* start ticking */
    }

    inwork = 1;			/* let the signal handler know we're
				   fussing */

    prev = &head;
    next = prev -> next;

    /* find this timer in the list (next gets NULL if not found) */

    while ((next != NULL) &&
	    (next -> timer != timer)) {/* existing entry for this id? */
	prev = next;
	next = next -> next;
    }

    /* if found, take it out of the list */

    if (next != NULL) {		/* found an existing */
	prev -> next = next -> next;/* unlink this entry */
    }
    else {			/* need a new node */
	if ((next = (struct timer  *) Xstmalloc (sizeof (struct timer))) == NULL) {
	    Log_Del ("MALLOC FAILED in Set_Timer, errno=%d\n", errno);
	    inwork = 0;
	    return (-1);
	}
    }

/*
 *	Now find where this entry should be in time order
 */

    prev = &head;
    while ((prev -> next != NULL) && ((prev -> next) -> time < time)) {
	prev = prev -> next;
    }
/*
 *	Link it in here 
 */
    next -> next = prev -> next;
    prev -> next = next;
    next -> timer = timer;
    next -> time = time;
    next -> routine = routine;
    inwork = 0;
    return (0);
}

int
Get_Timer (timer)
int     timer;
{
    struct timer   *prev;
    struct timer   *next;

    prev = &head;
    next = prev -> next;

    while ((next != NULL) && (next -> timer != timer)) {
	prev = next;
	next = prev -> next;
    }

    if (next != NULL) {
	return (next -> time - head.time);
    }
    else {
	return (-1);
    }
}

int
        Stop_Timer (timer)
int     timer;
{
    struct timer   *prev = NULL;
    struct timer   *next = NULL;

    inwork = 1;

    prev = &head;
    next = prev -> next;
    while ((next != NULL) &&
	    (next -> timer != timer)) {/* existing entry for this id? */
	prev = next;
	next = next -> next;
    }

    if (next == NULL) {
	return (-1);
    }

    prev -> next = next -> next;	/* delink this entry */
    free ((char *) next);

    if (head.next == NULL) {
	alarm(0);			/* stop ticking */
    }
    inwork = 0;
    return (0);
}


static int ringring;

static void
vis_chk_exp()
{
	ringring = 1;
}

void
Visual_Check()
{
	if (Xst_visual_check <= 0)
		return;

	if (Set_Timer(VISUAL_CHECK_TIMER, Xst_visual_check, vis_chk_exp) < 0)
		return;

	ringring = 0;
	while (!ringring)
		pause();
}
