/*-
 * Copyright (C) 1994 Bradley A. Grantham and Lawrence A. Kesteloot
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the Alice Group or any of its members may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ALICE GROUP ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE ALICE GROUP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "mac68k.h"
#include "dixstruct.h"
#include "dix.h"
#include "opaque.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


Bool mac68kMonoSave(
	ScreenPtr	screen,
	int	on)
{
	/* if(on == SCREEN_SAVER_FORCER)
		Do something with event time */
	return(FALSE);
}



Bool mac68kMonoInit(
	int	index,
	ScreenPtr	screen,
	int	argc,
	char	**argv)
{
	struct grfmode *id;
	char scrstr[150];

	if(mac_fbs[index].added)
		return(TRUE);

	screen->SaveScreen = mac68kMonoSave;
	screen->whitePixel = 0;
	screen->blackPixel = 1;

	id = &mac_fbs[index].idata;
	printf("Calling ScreenInit to add screen %d...\n", index);
	sprintf(scrstr, "Screen %d at %#08x, %d by %d, rowB %d, fbbase %#x.\n",
		index, mac_fbs[index].vaddr, id->width,
		id->height, id->rowbytes, id->fbbase);
	ErrorF(scrstr);
	if(!mfbScreenInit(screen,
		mac_fbs[index].vaddr,		/* BARF */
		id->width,
		id->height,
		/* id->vRes >> 16 */ 75,	/* BARF */
		/* id->vRes >> 16 */ 75,	/* BARF */
		id->rowbytes*8))
			return(FALSE);
	mac_fbs[index].added = 1;
	return(mac68k_screeninit(screen) && mfbCreateDefColormap(screen));
}
