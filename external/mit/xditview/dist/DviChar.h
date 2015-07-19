/*
 * DviChar.h
 *
 * descriptions for mapping dvi names to
 * font indexes and back.  Dvi fonts are all
 * 256 elements (actually only 256-32 are usable).
 *
 * The encoding names are taken from X -
 * case insensitive, a dash seperating the
 * CharSetRegistry from the CharSetEncoding
 */

#ifndef _DVICHAR_H_
#define _DVICHAR_H_

#include "Dvi.h"

# define DVI_MAX_SYNONYMS	10
# define DVI_MAP_SIZE		256
# define DVI_HASH_SIZE		256
# define DVI_MAX_LIGATURES	16

typedef struct _dviCharNameHash {
	struct _dviCharNameHash	*next;
	const char		*name;
	int			position;
} DviCharNameHash;

typedef struct _dviCharNameMap {
    const char *	encoding;
    int			special;
    const char * const	dvi_names[DVI_MAP_SIZE][DVI_MAX_SYNONYMS];
    const char * const	ligatures[DVI_MAX_LIGATURES][2];
    DviCharNameHash	*buckets[DVI_HASH_SIZE];
} DviCharNameMap;

extern DviCharNameMap	*DviFindMap (const char *);
extern void		DviRegisterMap (DviCharNameMap *);
#define DviCharName(map,index,synonym)	((map)->dvi_names[index][synonym])
extern int		DviCharIndex (DviCharNameMap *, const char *);
extern unsigned char	*DviCharIsLigature (DviCharNameMap *, const char *);
extern void		ResetFonts (DviWidget);

#endif
