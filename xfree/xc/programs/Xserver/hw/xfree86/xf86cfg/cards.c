/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/cards.c,v 1.1 2000/04/04 22:36:57 dawes Exp $
 */

#include "cards.h"

/* return values from ReadCardsLine. */
#define ERROR		-3
#define UNKNOWN		-2
#define END		-1
#define	NOTUSEFUL	0
#define	NAME		1
#define	CHIPSET		2
#define	SERVER		3
#define	DRIVER		4
#define	RAMDAC		5
#define	CLOCKCHIP	6
#define	DACSPEED	7
#define	NOCLOCKPROBE	8
#define	UNSUPPORTED	9
#define	SEE		10
#define	LINE		11

/*
 * Prototypes
 */
static int ReadCardsLine(FILE*, char*);	/* must have 256 bytes */
static int CompareCards(_Xconst void *left, _Xconst void *right);
static int BCompareCards(_Xconst void *left, _Xconst void *right);

/*
 * Initialization
 */
static int linenum = 0;
static char *Cards = "lib/X11/Cards";
CardsEntry **CardsDB;
int NumCardsEntry;

/*
 * Implementation
 */
void
ReadCardsDatabase(void)
{
    char buffer[256];
    FILE *fp = fopen(Cards, "r");
    int i, result;
    CardsEntry *entry = NULL;
    static char *CardsError = "Error reading Cards database, at line %d (%s).\n";

    if (fp == NULL) {
	fprintf(stderr, "Cannot open Cards database.\n");
	exit(1);
    }

    while ((result = ReadCardsLine(fp, buffer)) != END) {
	switch (result) {
	    case ERROR:
		fprintf(stderr, CardsError, linenum, buffer);
		break;
	    case UNKNOWN:
		fprintf(stderr,
			"Unknown field type in Cards database, at line %d (%s).\n",
			linenum, buffer);
		break;
	    case NAME:
		entry = calloc(1, sizeof(CardsEntry));
		if (NumCardsEntry % 16 == 0) {
		    CardsDB = realloc(CardsDB, sizeof(CardsEntry*) *
				      (NumCardsEntry + 16));
		    if (CardsDB == NULL) {
			fprintf(stderr, "Out of memory reading Cards database.\n");
			exit(1);
		    }
		}
		CardsDB[NumCardsEntry++] = entry;
		entry->name = strdup(buffer);
		break;
	    case CHIPSET:
		if (entry == NULL || entry->chipset != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->chipset = strdup(buffer);
		break;
	    case SERVER:
		if (entry == NULL || entry->server != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->server = strdup(buffer);
		break;
	    case DRIVER:
		if (entry == NULL || entry->driver != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->driver = strdup(buffer);
		break;
	    case RAMDAC:
		if (entry == NULL || entry->ramdac != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->ramdac = strdup(buffer);
		break;
	    case CLOCKCHIP:
		if (entry == NULL || entry->clockchip != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->clockchip = strdup(buffer);
		break;
	    case DACSPEED:
		if (entry == NULL || entry->dacspeed != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->dacspeed = strdup(buffer);
		break;
	    case NOCLOCKPROBE:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->flags |= F_NOCLOCKPROBE;
		break;
	    case UNSUPPORTED:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->flags |= F_UNSUPPORTED;
		break;
	    case SEE:
		if (entry == NULL || entry->see != NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else
		    entry->see = strdup(buffer);
		break;
	    case LINE:
		if (entry == NULL) {
		    fprintf(stderr, CardsError, linenum, buffer);
		}
		else if (entry->lines == NULL)
		    entry->lines = strdup(buffer);
		else {
		    char *str = malloc(strlen(entry->lines) + strlen(buffer) + 2);

		    sprintf(str, "%s\n%s", entry->lines, buffer);
		    free(entry->lines);
		    entry->lines = str;
		}
		break;
	}
    }

    fclose(fp);

    qsort(CardsDB, NumCardsEntry, sizeof(CardsEntry*), CompareCards);

#ifdef DEBUG
    for (i = 0; i < NumCardsEntry - 1; i++) {
	if (strcmp(CardsDB[i]->name, CardsDB[i+1]->name) == 0)
	    fprintf(stderr, "Duplicate entry in Cards database: (%s).\n",
		    CardsDB[i]->name);
    }
#endif

    for (i = 0; i < NumCardsEntry - 1; i++) {
	if (CardsDB[i]->see != NULL) {
	    if ((entry = LookupCard(CardsDB[i]->see)) == NULL) {
		fprintf(stderr, "Cannot find card '%s' for filling defaults.\n",
			CardsDB[i]->see);
		continue;
	    }
	    if (CardsDB[i]->chipset == NULL && entry->chipset != NULL)
		CardsDB[i]->chipset = strdup(entry->chipset);
	    if (CardsDB[i]->server == NULL && entry->server != NULL)
		CardsDB[i]->server = strdup(entry->server);
	    if (CardsDB[i]->driver == NULL && entry->driver != NULL)
		CardsDB[i]->driver = strdup(entry->driver);
	    if (CardsDB[i]->ramdac == NULL && entry->ramdac != NULL)
		CardsDB[i]->ramdac = strdup(entry->ramdac);
	    if (CardsDB[i]->clockchip == NULL && entry->clockchip != NULL)
		CardsDB[i]->clockchip = strdup(entry->clockchip);
	    if (CardsDB[i]->dacspeed == NULL && entry->dacspeed != NULL)
		CardsDB[i]->dacspeed = strdup(entry->dacspeed);
	    if (CardsDB[i]->flags & F_NOCLOCKPROBE)
		CardsDB[i]->flags |= F_NOCLOCKPROBE;
	    if (CardsDB[i]->flags & F_UNSUPPORTED)
		CardsDB[i]->flags |= F_UNSUPPORTED;
	    if (entry->lines != NULL) {
		if (CardsDB[i]->lines == NULL)
		    CardsDB[i]->lines = strdup(entry->lines);
		else {
		    char *str = malloc(strlen(entry->lines) +
					      strlen(CardsDB[i]->lines) + 2);

		    sprintf(str, "%s\n%s", CardsDB[i]->lines, entry->lines);
		    free(CardsDB[i]->lines);
		    CardsDB[i]->lines = str;
		}
	    }
	    if (entry->see != NULL) {
#ifdef DEBUG
		fprintf(stderr, "Nested SEE entry: %s -> %s -> %s\n",
			CardsDB[i]->name, CardsDB[i]->see, entry->see);
#endif
		CardsDB[i]->see = strdup(entry->see);
		--i;
		continue;
	    }
	    free(CardsDB[i]->see);
	    CardsDB[i]->see = NULL;
	}
    }
}

CardsEntry *
LookupCard(char *name)
{
    CardsEntry **ptr;

    ptr = (CardsEntry**)bsearch(name, CardsDB, NumCardsEntry,
				sizeof(CardsEntry*), BCompareCards);

    return (ptr != NULL ? *ptr : NULL);
}

char **
GetCardNames(int *result)
{
    char **cards = NULL;
    int ncards;

    for (ncards = 0; ncards < NumCardsEntry; ncards++) {
	if (ncards % 16 == 0) {
	    if ((cards = (char**)realloc(cards, sizeof(char*) *
					 (ncards + 16))) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	    }
	}
	cards[ncards] = strdup(CardsDB[ncards]->name);
    }

    *result = ncards;

    return (cards);
}

char **
FilterCardNames(char *pattern, int *result)
{
    FILE *fp;
    char **cards = NULL;
    int len, ncards = 0;
    char *cmd, *ptr, buffer[256];

    cmd = malloc(32 + (strlen(pattern) * 2) + strlen(Cards));

    strcpy(cmd, "egrep -i '^NAME\\ .*");
    len = strlen(cmd);
    ptr = pattern;
    while (*ptr) {
	if (!isalnum(*ptr)) {
	    cmd[len++] = '\\';
	}
	cmd[len++] = *ptr++;
    }
    cmd[len] = '\0';
    strcat(cmd, ".*$' ");
    strcat(cmd, Cards);
    strcat(cmd, " | sort");
    /*sprintf(cmd, "egrep -i '^NAME\\ .*%s.*$' %s | sort", pattern, Cards);*/

    if ((fp = popen(cmd, "r")) == NULL) {
	fprintf(stderr, "Cannot read Cards database.\n");
	exit(1);
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
	ptr = buffer + strlen(buffer) - 1;
	while (isspace(*ptr) && ptr > buffer)
	    --ptr;
	if (!isspace(*ptr) && ptr > buffer)
	    ptr[1] = '\0';
	ptr = buffer;
	while (!isspace(*ptr) && *ptr)	/* skip NAME */
	    ++ptr;
	while (isspace(*ptr) && *ptr)
	    ++ptr;
	if (ncards % 16 == 0) {
	    if ((cards = (char**)realloc(cards, sizeof(char*) *
					 (ncards + 16))) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	    }
	}
	cards[ncards++] = strdup(ptr);
    }
    free(cmd);

    *result = ncards;

    return (cards);
}

static int
ReadCardsLine(FILE *fp, char *value)
{
    char name[32], buffer[256], *ptr, *end;
    int result = NOTUSEFUL;

    ++linenum;

    if (fgets(buffer, sizeof(buffer), fp) == NULL)
	return (END);

    ptr = buffer;
    /* skip initial spaces; should'nt bother about this.. */
    while (isspace(*ptr) && *ptr)
	++ptr;

    if (*ptr == '#' || *ptr == '\0')
	return (NOTUSEFUL);

    end = ptr;
    while (!isspace(*end) && *end)
	++end;
    if (end - ptr > sizeof(buffer) - 1) {
	strncpy(value, buffer, 255);
	value[255] = '\0';
	return (ERROR);
    }
    strncpy(name, ptr, end - ptr);
    name[end - ptr] = '\0';

    /* read the optional arguments */
    ptr = end;
    while (isspace(*ptr) && *ptr)
	++ptr;

    end = ptr + strlen(ptr) - 1;
    while (isspace(*end) && end > ptr)
	--end;
    if (!isspace(*end))
	++end;
    *end = '\0';

    if (strcmp(name, "NAME") == 0)
	result = NAME;
    else if (strcmp(name, "CHIPSET") == 0)
	result = CHIPSET;
    else if (strcmp(name, "SERVER") == 0)
	result = SERVER;
    else if (strcmp(name, "DRIVER") == 0)
	result = DRIVER;
    else if (strcmp(name, "RAMDAC") == 0)
	result = RAMDAC;
    else if (strcmp(name, "CLOCKCHIP") == 0)
	result = CLOCKCHIP;
    else if (strcmp(name, "DACSPEED") == 0)
	result = DACSPEED;
    else if (strcmp(name, "NOCLOCKPROBE") == 0)
	result = NOCLOCKPROBE;
    else if (strcmp(name, "UNSUPPORTED") == 0)
	result = UNSUPPORTED;
    else if (strcmp(name, "SEE") == 0)
	result = SEE;
    else if (strcmp(name, "LINE") == 0)
	result = LINE;
    else if (strcmp(name, "END") == 0)
	result = END;
    else {
	strcpy(value, name);
	return (UNKNOWN);
    }

    /* value *must* have at least 256 bytes */
    strcpy(value, ptr);

    return (result);
}

static int
CompareCards(_Xconst void *left, _Xconst void *right)
{
    return strcmp((*(CardsEntry**)left)->name, (*(CardsEntry**)right)->name);
}

static int
BCompareCards(_Xconst void *name, _Xconst void *card)
{
  return (strcmp((char*)name, (*(CardsEntry**)card)->name));
}

#ifdef TEST_CARDS
int
main(int argc, char *argv[])
{
    int i;

    chdir("/usr/X11R6");
    ReadCardsDatabase();

    for (i = 0; i < NumCardsEntry; i++) {
	CardsEntry *entry = CardsDB[i];

	printf("name: %s\n", entry->name);
	if (entry->chipset)
	    printf("chipset: %s\n", entry->chipset);
	if (entry->server)
	    printf("server: %s\n", entry->server);
	if (entry->driver)
	    printf("driver: %s\n", entry->driver);
	if (entry->ramdac)
	    printf("%s\n", entry->ramdac);
	if (entry->clockchip)
	    printf("%s\n", entry->clockchip);
	if (entry->dacspeed)
	    printf("%s\n", entry->dacspeed);
	if (entry->flags & F_NOCLOCKPROBE)
	    printf("NOCLOCKPROBE\n");
	if (entry->flags & F_UNSUPPORTED)
	    printf("UNSUPPORTED\n");
	if (entry->lines)
	    printf("%s\n", entry->lines);
	printf("\n");
    }

    return (0);
}
#endif /* TEST_CARDS */
