#ifdef __CYGWIN__
LIBRARY ZLIB
EXPORTS
	adler32 @ 1 ;
	compress @ 2 ;
	compress2 @ 3 ;
	crc32 @ 4 ;
	deflate @ 5 ;
	deflateCopy @ 6 ;
	deflateEnd @ 7 ;
	deflateInit2_ @ 8 ;
	deflateInit_ @ 9 ;
	deflateParams @ 10 ;
	deflateReset @ 11 ;
	deflateSetDictionary @ 12 ;
	get_crc_table @ 13 ;
	gzclose @ 14 ;
	gzdopen @ 15 ;
	gzeof @ 16 ;
	gzerror @ 17 ;
	gzflush @ 18 ;
	gzgetc @ 19 ;
;	gzgets @ 20 ;
	gzopen @ 21 ;
	gzprintf @ 22 ;
	gzputc @ 23 ;
;	gzputs @ 24 ;
	gzread @ 25 ;
	gzrewind @ 26 ;
	gzseek @ 27 ;
	gzsetparams @ 28 ;
	gztell @ 29 ;
	gzwrite @ 30 ;
	inflate @ 31 ;
	inflateEnd @ 32 ;
	inflateInit2_ @ 33 ;
	inflateInit_ @ 34 ;
	inflateReset @ 35 ;
	inflateSetDictionary @ 36 ;
	inflateSync @ 37 ;
	inflateSyncPoint @ 38 ;
	uncompress @ 39 ;
	zError @ 40 ;
	zlibVersion @ 41 ;
#endif

/* $XFree86: xc/lib/zlib/z-def.cpp,v 1.1 2000/08/09 23:40:16 dawes Exp $ */
