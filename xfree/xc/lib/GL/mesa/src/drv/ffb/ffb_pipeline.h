/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_pipeline.h,v 1.1 2000/06/20 05:08:39 dawes Exp $ */

#ifndef _FFB_PIPELINE_H
#define _FFB_PIPELINE_H

extern GLboolean ffbDDBuildPrecalcPipeline(GLcontext *);
extern GLuint ffbDDRegisterPipelineStages(struct gl_pipeline_stage *,
					  const struct gl_pipeline_stage *,
					  GLuint);

#endif /* !(_FFB_PIPELINE_H) */
