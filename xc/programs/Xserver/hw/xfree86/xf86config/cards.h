/* $XFree86: xc/programs/Xserver/hw/xfree86/xf86config/cards.h,v 3.2 1996/02/04 09:15:39 dawes Exp $ */





/* $XConsortium: cards.h /main/2 1995/11/13 09:50:02 kaleb $ */

#ifndef CARD_DATABASE_FILE
#define CARD_DATABASE_FILE "Cards"
#endif

#define MAX_CARDS 1000

typedef struct {
	char *name;		/* Name of the card. */
	char *chipset;		/* Chipset (decriptive). */
	char *server;		/* Server identifier. */
	char *ramdac;		/* Ramdac identifier. */
	char *clockchip;	/* Clockchip identifier. */
	char *dacspeed;		/* DAC speed rating. */
	int flags;
	char *lines;		/* Additional Device section lines. */
} Card;

/* Flags: */
#define NOCLOCKPROBE	0x1	/* Never probe clocks of the card. */
#define UNSUPPORTED	0x2	/* Card is not supported (only VGA). */

extern int lastcard;

extern Card card[MAX_CARDS];


int parse_database();

