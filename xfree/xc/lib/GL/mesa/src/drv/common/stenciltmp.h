/* $XFree86: xc/lib/GL/mesa/src/drv/common/stenciltmp.h,v 1.2 2000/09/26 15:56:46 tsi Exp $ */

#ifndef DBG
#define DBG 0
#endif


static void TAG(WriteStencilSpan)( GLcontext *ctx,
				   GLuint n, GLint x, GLint y,
				   const GLstencil *stencil, 
				   const GLubyte mask[] )
{
   HW_LOCK()
      {
	 GLint x1;
	 GLint n1;
	 LOCAL_STENCIL_VARS;

	 y = Y_FLIP(y);

	 HW_CLIPLOOP() 
	    {
	       GLint i = 0;
	       CLIPSPAN(x,y,n,x1,n1,i);

	       if (DBG) fprintf(stderr, "WriteStencilSpan %d..%d (x1 %d)\n",
				(int)i, (int)n1, (int)x1);

	       if (mask)
	       {
		  for (;i<n1;i++,x1++)
		     if (mask[i])
			WRITE_STENCIL( x1, y, stencil[i] );
	       }
	       else
	       {
		  for (;i<n1;i++,x1++)
		     WRITE_STENCIL( x1, y, stencil[i] );
	       }
	    }
	 HW_ENDCLIPLOOP();
      }
   HW_UNLOCK();
}


static void TAG(WriteStencilPixels)( GLcontext *ctx,
				     GLuint n, 
				     const GLint x[], 
				     const GLint y[],
				     const GLstencil stencil[], 
				     const GLubyte mask[] )
{
   HW_LOCK()
      {
	 GLint i;
	 LOCAL_STENCIL_VARS;

	 if (DBG) fprintf(stderr, "WriteStencilPixels\n");

	 HW_CLIPLOOP()
	    {
	       for (i=0;i<n;i++)
	       {
		  if (mask[i]) {
		     const int fy = Y_FLIP(y[i]);
		     if (CLIPPIXEL(x[i],fy))
			WRITE_STENCIL( x[i], fy, stencil[i] );
		  }
	       }
	    }
	 HW_ENDCLIPLOOP();
      }
   HW_UNLOCK();
}


/* Read stencil spans and pixels
 */
static void TAG(ReadStencilSpan)( GLcontext *ctx,
				  GLuint n, GLint x, GLint y,
				  GLstencil stencil[])
{
   HW_LOCK()
      {
	 GLint x1,n1;
	 LOCAL_STENCIL_VARS;

	 y = Y_FLIP(y);

	 if (DBG) fprintf(stderr, "ReadStencilSpan\n");

	 HW_CLIPLOOP() 
	    {
	       GLint i = 0;
	       CLIPSPAN(x,y,n,x1,n1,i);
	       for (;i<n1;i++)
		  READ_STENCIL( stencil[i], (x1+i), y );
	    }
	 HW_ENDCLIPLOOP();
      }
   HW_UNLOCK();
}

static void TAG(ReadStencilPixels)( GLcontext *ctx, GLuint n, 
				    const GLint x[], const GLint y[],
				    GLstencil stencil[] )
{
   HW_LOCK()
      {
	 GLint i;
	 LOCAL_STENCIL_VARS;

	 if (DBG) fprintf(stderr, "ReadStencilPixels\n");
 
	 HW_CLIPLOOP()
	    {
	       for (i=0;i<n;i++) {
		  int fy = Y_FLIP( y[i] );
		  if (CLIPPIXEL( x[i], fy ))
		     READ_STENCIL( stencil[i], x[i], fy );
	       }
	    }
	 HW_ENDCLIPLOOP();
      }
   HW_UNLOCK();
}




#undef WRITE_STENCIL
#undef READ_STENCIL
#undef TAG
