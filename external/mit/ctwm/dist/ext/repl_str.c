/*
 * An implementation of replacing substrings in a string.
 *
 * Source: http://creativeandcritical.net/str-replace-c
 * Author: Laird Shaw
 * License: Public domain
 *
 * Brought on 2016-02-24
 */

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "repl_str.h"


/* An optimised string replacement function that caches string positions so that
 * strstr() doesn't need to be called twice for each position.
 * 
 * This code is entirely by me, and released into the public domain, so have no
 * doubts about your right to use it.
 */
char *replace_substr(const char *str, const char *old, const char *new) {

	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 3;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	ptrdiff_t *pos_cache = NULL;
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, newlen, oldlen = strlen(old);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, old)) != NULL) {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache == NULL) {
				goto end_repl_str;
			}
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + oldlen;
	}

	orglen = pstr - str + strlen(pstr);

	/* Allocate memory for the post-replacement string. */
	if (count > 0) {
		newlen = strlen(new);
		retlen = orglen + (newlen - oldlen) * count;
	} else	retlen = orglen;
	ret = malloc(retlen + 1);
	if (ret == NULL) {
		goto end_repl_str;
	}

	if (count == 0) {
		/* If no matches, then just duplicate the string. */
		strcpy(ret, str);
	} else {
		/* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
		pret = ret;
		memcpy(pret, str, pos_cache[0]);
		pret += pos_cache[0];
		for (i = 0; i < count; i++) {
			memcpy(pret, new, newlen);
			pret += newlen;
			pstr = str + pos_cache[i] + oldlen;
			cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - oldlen;
			memcpy(pret, pstr, cpylen);
			pret += cpylen;
		}
		ret[retlen] = '\0';
	}

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	free(pos_cache);
	return ret;
}
