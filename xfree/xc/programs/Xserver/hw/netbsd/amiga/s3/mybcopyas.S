/*
 * This program is in the public domain and may be used freely by anyone
 * who wants to.
 * Author: Michael Teske
 * Created: 29-Feb-1996
 */

#include <m68k/asm.h> 

/*
;_mybcopyas - memory copy routine with no masking
;
;Arguments:
;a0.l Source address
;a1.l Destination address
;d0.l Size in bytes (unsigned)
;
;Notes:
;This routine gets fast with large buffer sizes and even faster if
;ecactly one of source and destination start address are longword-adjusted.
;This routine crashes when a 68000 is used and exactly one of the source
;and destination address is uneven.
*/

ENTRY_NOPROFILE(_mybcopyas)
ENTRY(mybcopyas)
	tstl	%d0
	bnes	Lcont
	rts
Lcont:
	movew	%a0,%d1
	andiw	#3,%d1
	beqs	Laligndone
	movew	%a1,%d1
	andiw	#3,%d1
	beqs	Laligndone

Lnotaligned:			|This part is reached if source and destination
	subqw	#1,%d1		|are not aligned
Lalignloop:
	moveb	%a0@+,%a1@+
	subql	#1,%d0
	beqs	Lalldone
	dbra	%d1,Lalignloop
	
Laligndone:
	movel	%d0,%d1
	lsrl	#5,%d0		|move8 loop runs
	andiw	#31,%d1		|Excess bytes

	subql	#1,%d0
	blts	Lmove8done
Lmove8loop:			|movem's doesn't make this faster on a 030+,
	movel	%a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
        movel   %a0@+,%a1@+
	dbra	%d0,Lmove8loop
	subl	#0x10000,%d0
	bccs	Lmove8loop	|d0 is unsigned, so carry bit is checked

Lmove8done:
	movew	%d1,%d0
	lsrw	#2,%d0		|move.l	loop runs
	andiw	#3,%d1		|Excess bytes
	subqw	#1,%d0
	blts	Lmove1done
Lmove1loop:			|copy excess longwords
	movel	%a0@+,%a1@+
	dbra	%d0,Lmove1loop

Lmove1done:
	subqw	#1,%d1
	blts	Lalldone
Lmovebloop:			|Copy excess bytes
	moveb	%a0@+,%a1@+
	dbra	%d1,Lmovebloop

Lalldone:
	rts
