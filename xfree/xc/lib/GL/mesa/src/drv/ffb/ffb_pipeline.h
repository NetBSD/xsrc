/* $XFree86$ */

#ifndef _FFB_PIPELINE_H
#define _FFB_PIPELINE_H

extern GLboolean ffbDDBuildPrecalcPipeline(GLcontext *);
extern GLuint ffbDDRegisterPipelineStages(struct gl_pipeline_stage *,
					  const struct gl_pipeline_stage *,
					  GLuint);

#endif /* !(_FFB_PIPELINE_H) */
