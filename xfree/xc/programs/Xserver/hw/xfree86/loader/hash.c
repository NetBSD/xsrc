/* $XFree86: xc/programs/Xserver/hw/xfree86/loader/hash.c,v 1.13 2000/09/26 15:57:18 tsi Exp $ */

/*
 *
 * Copyright 1995-1998 by Metro Link, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Metro Link, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Metro Link, Inc. makes no
 * representations about the suitability of this software for any purpose.
 *  It is provided "as is" without express or implied warranty.
 *
 * METRO LINK, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL METRO LINK, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "os.h"
#include "Xos.h"
#ifndef X_NOT_STDC_ENV
#undef abs
#include <stdlib.h>
#else
extern void free();
#endif
#include "sym.h"
#include "loader.h"
#include "hash.h"

#if defined(Lynx)
#define MAXINT	32000
#else
#include <limits.h>
#undef MAXINT
#define MAXINT INT_MAX
#endif

/* Prototypes for static functions. */
static unsigned int hashFunc(const char *);
static itemPtr LoaderHashFindNearest(
#if NeedFunctionPrototypes
unsigned long
#endif
);

static itemPtr LoaderhashTable[ HASHSIZE ] ;

#ifdef DEBUG
static int hashhits[ HASHSIZE ] ;

void
DumpHashHits(void)
{
    int	i;
    int	depth=0;
    int	dev=0;

    for(i=0;i<HASHSIZE;i++) {
	ErrorF("hashhits[%d]=%d\n", i, hashhits[i] );
	depth += hashhits[i];
    }

    depth /= HASHSIZE;
    ErrorF("Average hash depth=%d\n", depth );

    for(i=0;i<HASHSIZE;i++) {
	if( hashhits[i] < depth )
	    dev += depth-hashhits[i];
	else
	    dev += hashhits[i]-depth;
    }

    dev /=HASHSIZE;
    ErrorF("Average hash deviation=%d\n", dev );
}
#endif


static unsigned int
hashFunc(string)
const char *string;
{
    int i=0; 

    while ( i < 10 && string[i] )
      i ++ ;

    if ( i < 5 ) {
#ifdef DEBUG
	hashhits[i]++;
#endif
	return i ;
    }

/*
 * Original has function
#define HASH ((string[ i-4 ] * string[i-3] + string[i-2] ) & (HASHSIZE-1))
 */

#define HASH ((string[i-5] * string[ i-4 ] + string[i-3] * string[i-2] ) & (HASHSIZE-1))

#ifdef DEBUG
    hashhits[HASH]++;
#endif

    return HASH;
}

void
LoaderHashAdd( entry )
    itemPtr entry ;
{
  int bucket = hashFunc( entry->name ) ;
  itemPtr oentry;

  if ((oentry = LoaderHashFind(entry->name)) != NULL)
	LoaderDuplicateSymbol(entry->name, oentry->handle);

  entry->next = LoaderhashTable[ bucket ] ;
  LoaderhashTable[ bucket ] = entry ;
  return;
}

void
LoaderAddSymbols(handle, module, list)
int	handle;
int	module;
LOOKUP	*list ;
{
    LOOKUP	*l = list;
    itemPtr	i;
    char		*modname;

    if (!list)
	return;
    /* Visit every symbol in the lookup table,
     * and add it to the given namespace.
     */
    while ( l->symName ) {
	i = xf86loadermalloc( sizeof( itemRec )) ;
	i->name = l->symName ;
	if( strcmp(i->name,"ModuleInit") == 0
#if defined(__powerpc__) && defined(Lynx)
	  || strcmp(i->name,".ModuleInit") == 0
#endif
	  )
	{
		char *origname=i->name;
		/*
		 * special handling for symbol name "ModuleInit"
		 */
		modname = _LoaderHandleToCanonicalName(handle);
		if (modname) 
		{
			i->name = xf86loadermalloc(strlen(modname) +
						strlen(origname) + 1);
			if( i->name )
			    {
				/* XXX Is this right for PPC? */
				strcpy(i->name,modname);
				strcat(i->name,origname);
			    }
		}
#ifdef DEBUG
		ErrorF("Add module init function %s at %lx\n",i->name, l->offset);
#endif
	}
	i->address = (char *) l->offset ;
	i->handle = handle ;
	i->module = module ;
	LoaderHashAdd( i );
	l ++ ;
    }
}

itemPtr
LoaderHashDelete(string)
const char *string;
{
  int bucket = hashFunc( string ) ;
  itemPtr entry;
  itemPtr *entry2;

  entry = LoaderhashTable[ bucket ] ;
  entry2= &(LoaderhashTable[ bucket ]);
  while ( entry ) {
    if (! strcmp( entry->name, string )) {
      *entry2=entry->next;
      xf86loaderfree(entry->name);
      xf86loaderfree( entry ) ;
      return 0 ;
    }
    entry2 = &(entry->next) ;
    entry = entry->next ;
  }
  return 0 ;
}

itemPtr
LoaderHashFind(string)
const char *string;
{
    int bucket = hashFunc( string ) ;
    itemPtr entry ;
	entry = LoaderhashTable[ bucket ] ;
	while ( entry ) {
	    if (!strcmp(entry->name, string)) {
		return entry;
	    }
	    entry = entry->next;
	}
    return 0;
}

static itemPtr
LoaderHashFindNearest(address)
unsigned long address;
{
  int i ;
  itemPtr entry, best_entry = 0 ;
  long best_difference = MAXINT;

  for ( i = 0 ; i < HASHSIZE ; i ++ ) {
    entry = LoaderhashTable[ i ] ;
    while ( entry ) {
      long difference = (long) address - (long) entry->address ;
      if ( difference >= 0 ) {
	if ( best_entry ) {
	  if ( difference < best_difference ) {
	    best_entry = entry ;
	    best_difference = difference ;
	  }
	}
	else {
	  best_entry = entry ;
	  best_difference = difference ;
	}
      }
      entry = entry->next ;
    }
  }
  return best_entry ;
}

void
LoaderPrintSymbol(address)
unsigned long address;
{
    itemPtr	entry;
    entry=LoaderHashFindNearest(address);
    if (entry) {
	const char *module, *section;
#if defined(__alpha__) || defined(__ia64__)
	ErrorF("0x%016lx %s+%lx\n", entry->address, entry->name,
		   address - (unsigned long) entry->address);
#else
	ErrorF("0x%x %s+%x\n", entry->address, entry->name,
		   address - (unsigned long) entry->address);
#endif

	if ( _LoaderAddressToSection(address, &module, &section) )
		ErrorF("\tModule \"%s\"\n\tSection \"%s\"\n",module, section );
    } else {
	ErrorF("(null)\n");
    }
}

void
LoaderPrintItem(itemPtr pItem)
{
    if (pItem) {
	const char *module, *section;
#if defined(__alpha__) || defined(__ia64__)
	ErrorF("0x%016lx %s\n", pItem->address, pItem->name);
#else
	ErrorF("0x%lx %s\n", pItem->address, pItem->name);
#endif
	if ( _LoaderAddressToSection((unsigned long)pItem->address,
				     &module, &section) )
		ErrorF("\tModule \"%s\"\n\tSection \"%s\"\n",module, section );
    } else
	ErrorF("(null)\n");
}
	
void
LoaderPrintAddress(symbol)
const char *symbol;
{
    itemPtr	entry;
    entry = LoaderHashFind(symbol);
    LoaderPrintItem(entry);
}

void
LoaderHashTraverse(card, fnp)
    void *card;
    int (*fnp)(void *, itemPtr);
{
  int i ;
  itemPtr entry, last_entry = 0 ;

  for ( i = 0 ; i < HASHSIZE ; i ++ ) {
    last_entry = 0 ;
    entry = LoaderhashTable[ i ] ;
    while ( entry ) {
      if (( * fnp )( card, entry )) {
	if ( last_entry ) {
	  last_entry->next = entry->next ;
	  xf86loaderfree( entry->name ) ;
	  xf86loaderfree( entry ) ;
	  entry = last_entry->next ;
	}
	else {
	  LoaderhashTable[ i ] = entry->next ;
	  xf86loaderfree( entry->name ) ;
	  xf86loaderfree( entry ) ;
	  entry = LoaderhashTable[ i ] ;
	}
      }
      else {
	last_entry = entry ;
	entry = entry->next ;
      }
    }
  }
}

void
LoaderDumpSymbols()
{
	itemPtr       entry;
	int           j;
		
	for (j=0; j<HASHSIZE; j++) {
		entry = LoaderhashTable[j];
		while (entry) {
			LoaderPrintItem(entry);
			entry = entry->next;
		}
	}
		
}
