/*  #include "i810pipeline.h" */

#include <stdio.h>

#include "types.h"
#include "fog.h"

#include "i810vb.h"
#include "i810dd.h"
#include "i810tris.h"
#include "i810pipeline.h"


static struct gl_pipeline_stage i810_fast_stage = {
   "I810 fast path",
   (PIPE_OP_VERT_XFORM|PIPE_OP_RAST_SETUP_0|
    PIPE_OP_RAST_SETUP_1|PIPE_OP_RENDER),
   PIPE_PRECALC,
   0, 0, 0, 0, 0, 0, 0, 0, 0,	
   i810DDFastPath
};


#define ILLEGAL_ENABLES (TEXTURE0_3D|		\
			 TEXTURE1_3D|		\
			 ENABLE_TEXMAT0 |	\
			 ENABLE_TEXMAT1 |	\
			 ENABLE_TEXGEN0 |	\
			 ENABLE_TEXGEN1 |	\
			 ENABLE_USERCLIP |	\
			 ENABLE_LIGHT |		\
			 ENABLE_FOG)
			

/* The driver gets first shot at building the pipeline - make some
 * quick tests to see if we can use the fast path.
 */
GLboolean i810DDBuildPrecalcPipeline( GLcontext *ctx )
{   
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   struct gl_pipeline *pipe = &ctx->CVA.pre;
   
   if (imesa->renderindex == 0 && 
       (ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
       (ctx->Array.Flags & (VERT_OBJ_234|
			    VERT_TEX0_4|
			    VERT_TEX1_4|
			    VERT_ELT)) == (VERT_OBJ_23|VERT_ELT))
   {
      pipe->stages[0] = &i810_fast_stage;
      pipe->stages[1] = 0;
      pipe->new_inputs = ctx->RenderFlags & VERT_DATA;
      pipe->ops = pipe->stages[0]->ops;
      imesa->using_fast_path = 1;
      return 1;
   } 

   if (imesa->using_fast_path) 
   {
      imesa->using_fast_path = 0;
      ctx->CVA.VB->ClipOrMask = 0;
      ctx->CVA.VB->ClipAndMask = CLIP_ALL_BITS;
      ctx->Array.NewArrayState |= ctx->Array.Summary;
      return 0;
   }
   
   return 0;
}



GLuint i810DDRegisterPipelineStages( struct gl_pipeline_stage *out,
				    const struct gl_pipeline_stage *in,
				    GLuint nr )
{
   GLuint i, o;

   for (i = o = 0 ; i < nr ; i++) {
      switch (in[i].ops) {

      case PIPE_OP_RAST_SETUP_0:
	 out[o] = in[i];
	 out[o].cva_state_change = NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS;
	 out[o].state_change = ~0;
	 out[o].check = i810DDCheckPartialRasterSetup;
	 out[o].run = i810DDPartialRasterSetup;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1:
	 out[o] = in[i];
	 out[o].run = i810DDDoRasterSetup;
	 o++;
	 break;

	 /* Completely replace Mesa's fog processing to generate fog
	  * coordinates instead of messing with colors.
	  */
      case PIPE_OP_FOG:
	 out[o] = gl_fog_coord_stage;
	 o++;
	 break;


      default:
	 out[o++] = in[i];
	 break;
      }
   }

   return o;
}


