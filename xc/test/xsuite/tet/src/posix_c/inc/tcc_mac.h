/*
 * Copyright 1990 Open Software Foundation (OSF)
 * Copyright 1990 Unix International (UI)
 * Copyright 1990 X/Open Company Limited (X/Open)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OSF, UI or X/Open not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  OSF, UI and X/Open make 
 * no representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
 * EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/************************************************************************

SCCS:          @(#)tcc_mac.h    1.9 03/09/92
NAME:          tcc_mac.h
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        OSF Validation & SQA
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd, July 1991.

	       Reformatted various error macros for better readability,
	       made error message formats consistent.
               David G. Sawyer, UniSoft Ltd, 12 Feb 1992.

	       Added TET_REALLOC, TET_REALLOC2, TET_MALLOC2 and TET_FREE.
               David G. Sawyer, UniSoft Ltd, 12 Feb 1992.

************************************************************************/




/* tet_mallloc will retry the malloc several times before giving up */
#define TET_MALLOC2(_val) tet_malloc((size_t)(_val), __FILE__, __LINE__, 1)
#define TET_REALLOC2(_ptr, _val) tet_realloc((void *) _ptr, (size_t)(_val), __FILE__, __LINE__, 1)
#define TET_MALLOC(_val) tet_malloc((size_t)(_val), __FILE__, __LINE__,0)
#define TET_REALLOC(_ptr, _val) tet_realloc((void *) _ptr, (size_t)(_val), __FILE__, __LINE__, 0)
#define TET_FREE(_ptr) tet_free((void *) (_ptr), __FILE__, __LINE__)


#define SUCCESS          0
#define FAILURE          -1
#define NONFATAL         1
#define FATAL 		 FAILURE


/* bail out just with message - always terminates */

#define BAIL_OUT(_msg) 		{ 					\
					int errnotmp; 			\
					errnotmp = errno;		\
					(void) fprintf(stderr,"Tcc fatal error reported from source file %s line %d :\n",__FILE__,__LINE__); 		\
					errno = errnotmp;		\
					perror(_msg);			\
					tet_shutdown(); 		\
				}

/* bail out on FATAL with a message - depending on _val, terminate or return */

#define BAIL_OUT_ON(_val,_msg) 	if (_val == FATAL) 			\
				{					\
					BAIL_OUT(_msg);			\
				}


/* bail out without a perror */

#define BAIL_OUT2(_msg) 	{					\
					(void) fprintf(stderr,"Tcc fatal error reported from source file %s line %d :\n%s\n",__FILE__,__LINE__,_msg); 	\
					tet_shutdown(); 		\
				}


/* just complain with a message*/

#define COMPLAIN(_msg) 		{					\
					int errnotmp;			\
					errnotmp = errno;		\
					(void) fprintf(stderr,"Tcc warning reported from source file %s line %d :\n",__FILE__,__LINE__);		\
					errno = errnotmp;		\
					perror(_msg); 			\
				}


/* just complain on FAILURE with a message*/

#define COMPLAIN_ON(_val,_msg) 	{ 					\
					if (_val == FAILURE) 		\
					{				\
						COMPLAIN(_msg);		\
					}				\
				}


/* display an error mesg on the stderr and in the journal file if its open */

#define ERROR(_msg) tet_error(_msg, __FILE__, __LINE__)


/* determines whether to call tet_error() which gives diagnostic reports to
   stderr and the journal file */

#define ERROR_ON(_val, _msg) 	if(_val == FAILURE)			\
				{					\
					ERROR(_msg);			\
				}


/* As with ERROR but shutdown afterwards */

#define FATAL_ERROR(_msg)	{					     \
					tet_error(_msg, __FILE__, __LINE__); \
					tet_shutdown(); 		     \
				}
