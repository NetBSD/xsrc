#ifndef __GLX_unpack_h__
#define __GLX_unpack_h__

/* $XFree86: xc/programs/Xserver/GL/glx/unpack.h,v 1.2 1999/06/14 07:31:36 dawes Exp $ */
/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
** $SGI$
*/

#define __GLX_PAD(s) (((s)+3) & (GLuint)~3)

/*
** Fetch the context-id out of a SingleReq request pointed to by pc.
*/
#define __GLX_GET_SINGLE_CONTEXT_TAG(pc) (((xGLXSingleReq*)pc)->contextTag)
#define __GLX_GET_VENDPRIV_CONTEXT_TAG(pc) (((xGLXVendorPrivateReq*)pc)->contextTag)

/*
** Fetch a double from potentially unaligned memory.
*/
#ifdef __GLX_ALIGN64
#define __GLX_MEM_COPY(dst,src,n)	memcpy(dst,src,n)
#define __GLX_GET_DOUBLE(dst,src)	__GLX_MEM_COPY(&dst,src,8)
#else
#define __GLX_GET_DOUBLE(dst,src)	(dst) = *((GLdouble*)(src))
#endif

extern void __glXMemInit();

extern xGLXSingleReply __glXReply;

#define __GLX_BEGIN_REPLY(size) \
  	__glXReply.length = __GLX_PAD(size) >> 2;	\
  	__glXReply.type = X_Reply; 			\
  	__glXReply.sequenceNumber = client->sequence;

#define __GLX_SEND_HEADER() \
	WriteToClient( client, sz_xGLXSingleReply, (char *)&__glXReply);

#define __GLX_PUT_RETVAL(a) \
  	__glXReply.retval = (a);
  
#define __GLX_PUT_SIZE(a) \
  	__glXReply.size = (a);

#define __GLX_PUT_RENDERMODE(m) \
        __glXReply.pad3 = (m)

/*
** Get a buffer to hold returned data, with the given alignment.  If we have
** to realloc, allocate size+align, in case the pointer has to be bumped for
** alignment.  The answerBuffer should already be aligned.
**
** NOTE: the cast (long)res below assumes a long is large enough to hold a
** pointer.
*/
#define __GLX_GET_ANSWER_BUFFER(res,cl,size,align)			 \
    if ((size) > sizeof(answerBuffer)) {				 \
	int bump;							 \
	if ((cl)->returnBufSize < (size)+(align)) {			 \
	    (cl)->returnBuf = (GLbyte*)Xrealloc((cl)->returnBuf,	 \
						(size)+(align));         \
	    if (!(cl)->returnBuf) {					 \
		return BadAlloc;					 \
	    }								 \
	    (cl)->returnBufSize = (size)+(align);			 \
	}								 \
	res = (char*)cl->returnBuf;					 \
	bump = (long)(res) % (align);					 \
	if (bump) res += (align) - (bump);				 \
    } else {								 \
	res = (char *)answerBuffer;					 \
    }

#define __GLX_PUT_BYTE() \
  	*(GLbyte *)&__glXReply.pad3 = *(GLbyte *)answer
	  
#define __GLX_PUT_SHORT() \
  	*(GLshort *)&__glXReply.pad3 = *(GLshort *)answer
	  
#define __GLX_PUT_INT() \
  	*(GLint *)&__glXReply.pad3 = *(GLint *)answer
	  
#define __GLX_PUT_FLOAT() \
  	*(GLfloat *)&__glXReply.pad3 = *(GLfloat *)answer
	  
#define __GLX_PUT_DOUBLE() \
  	*(GLdouble *)&__glXReply.pad3 = *(GLdouble *)answer
	  
#define __GLX_SEND_BYTE_ARRAY(len) \
	WriteToClient(client, __GLX_PAD((len)*__GLX_SIZE_INT8), (char *)answer)

#define __GLX_SEND_SHORT_ARRAY(len) \
	WriteToClient(client, __GLX_PAD((len)*__GLX_SIZE_INT16), (char *)answer)
  
#define __GLX_SEND_INT_ARRAY(len) \
	WriteToClient(client, (len)*__GLX_SIZE_INT32, (char *)answer)
  
#define __GLX_SEND_FLOAT_ARRAY(len) \
	WriteToClient(client, (len)*__GLX_SIZE_FLOAT32, (char *)answer)
  
#define __GLX_SEND_DOUBLE_ARRAY(len) \
	WriteToClient(client, (len)*__GLX_SIZE_FLOAT64, (char *)answer)


#define __GLX_SEND_VOID_ARRAY(len)  __GLX_SEND_BYTE_ARRAY(len)
#define __GLX_SEND_UBYTE_ARRAY(len)  __GLX_SEND_BYTE_ARRAY(len)
#define __GLX_SEND_USHORT_ARRAY(len) __GLX_SEND_SHORT_ARRAY(len)
#define __GLX_SEND_UINT_ARRAY(len)  __GLX_SEND_INT_ARRAY(len)

/*
** PERFORMANCE NOTE:
** Machine dependent optimizations abound here; these swapping macros can
** conceivably be replaced with routines that do the job faster.
*/
#define __GLX_DECLARE_SWAP_VARIABLES \
	GLbyte sw;		\
  	GLbyte *swapPC;		\
  	GLbyte *swapEnd

#define __GLX_SWAP_INT(pc) 			\
  	sw = ((GLbyte *)(pc))[0]; 		\
  	((GLbyte *)(pc))[0] = ((GLbyte *)(pc))[3]; 	\
  	((GLbyte *)(pc))[3] = sw; 		\
  	sw = ((GLbyte *)(pc))[1]; 		\
  	((GLbyte *)(pc))[1] = ((GLbyte *)(pc))[2]; 	\
  	((GLbyte *)(pc))[2] = sw;	

#define __GLX_SWAP_SHORT(pc) \
  	sw = ((GLbyte *)(pc))[0]; 		\
  	((GLbyte *)(pc))[0] = ((GLbyte *)(pc))[1]; 	\
  	((GLbyte *)(pc))[1] = sw; 	

#define __GLX_SWAP_DOUBLE(pc) \
  	sw = ((GLbyte *)(pc))[0]; 		\
  	((GLbyte *)(pc))[0] = ((GLbyte *)(pc))[7]; 	\
  	((GLbyte *)(pc))[7] = sw; 		\
  	sw = ((GLbyte *)(pc))[1]; 		\
  	((GLbyte *)(pc))[1] = ((GLbyte *)(pc))[6]; 	\
  	((GLbyte *)(pc))[6] = sw;			\
  	sw = ((GLbyte *)(pc))[2]; 		\
  	((GLbyte *)(pc))[2] = ((GLbyte *)(pc))[5]; 	\
  	((GLbyte *)(pc))[5] = sw;			\
  	sw = ((GLbyte *)(pc))[3]; 		\
  	((GLbyte *)(pc))[3] = ((GLbyte *)(pc))[4]; 	\
  	((GLbyte *)(pc))[4] = sw;	

#define __GLX_SWAP_FLOAT(pc) \
  	sw = ((GLbyte *)(pc))[0]; 		\
  	((GLbyte *)(pc))[0] = ((GLbyte *)(pc))[3]; 	\
  	((GLbyte *)(pc))[3] = sw; 		\
  	sw = ((GLbyte *)(pc))[1]; 		\
  	((GLbyte *)(pc))[1] = ((GLbyte *)(pc))[2]; 	\
  	((GLbyte *)(pc))[2] = sw;	

#define __GLX_SWAP_INT_ARRAY(pc, count) \
  	swapPC = ((GLbyte *)(pc));		\
  	swapEnd = ((GLbyte *)(pc)) + (count)*__GLX_SIZE_INT32;\
  	while (swapPC < swapEnd) {		\
	    __GLX_SWAP_INT(swapPC);		\
	    swapPC += __GLX_SIZE_INT32;		\
	}
	
#define __GLX_SWAP_SHORT_ARRAY(pc, count) \
  	swapPC = ((GLbyte *)(pc));		\
  	swapEnd = ((GLbyte *)(pc)) + (count)*__GLX_SIZE_INT16;\
  	while (swapPC < swapEnd) {		\
	    __GLX_SWAP_SHORT(swapPC);		\
	    swapPC += __GLX_SIZE_INT16;		\
	}
	
#define __GLX_SWAP_DOUBLE_ARRAY(pc, count) \
  	swapPC = ((GLbyte *)(pc));		\
  	swapEnd = ((GLbyte *)(pc)) + (count)*__GLX_SIZE_FLOAT64;\
  	while (swapPC < swapEnd) {		\
	    __GLX_SWAP_DOUBLE(swapPC);		\
	    swapPC += __GLX_SIZE_FLOAT64;	\
	}
    
#define __GLX_SWAP_FLOAT_ARRAY(pc, count) \
  	swapPC = ((GLbyte *)(pc));		\
  	swapEnd = ((GLbyte *)(pc)) + (count)*__GLX_SIZE_FLOAT32;\
  	while (swapPC < swapEnd) {		\
	    __GLX_SWAP_FLOAT(swapPC);		\
	    swapPC += __GLX_SIZE_FLOAT32;	\
	}

#define __GLX_SWAP_REPLY_HEADER() \
  	__GLX_SWAP_SHORT(&__glXReply.sequenceNumber); \
  	__GLX_SWAP_INT(&__glXReply.length);

#define __GLX_SWAP_REPLY_RETVAL() \
  	__GLX_SWAP_INT(&__glXReply.retval)

#define __GLX_SWAP_REPLY_SIZE() \
  	__GLX_SWAP_INT(&__glXReply.size)

#endif /* !__GLX_unpack_h__ */





