XCOMM $XFree86: xc/lib/GL/mesa/src/Imakefile.inc,v 1.5 2001/04/02 22:15:47 dawes Exp $

#ifndef MesaBuildDir
#define MesaBuildDir $(GLXLIBSRC)/mesa/src/
#endif

MESABUILDDIR = MesaBuildDir

#ifdef NeedAllMesaSrc
 DISPATCHSRCS = $(MESABUILDDIR)dispatch.c
    GLAPISRCS = $(MESABUILDDIR)glapi.c
GLAPINOOPSRCS = $(MESABUILDDIR)glapinoop.c
 GLTHREADSRCS = $(MESABUILDDIR)glthread.c
#endif

COREMESASRCS =	$(MESABUILDDIR)aatriangle.c \
		$(MESABUILDDIR)accum.c \
		$(MESABUILDDIR)alpha.c \
		$(MESABUILDDIR)alphabuf.c \
		$(MESABUILDDIR)attrib.c \
		$(MESABUILDDIR)bbox.c \
		$(MESABUILDDIR)bitmap.c \
		$(MESABUILDDIR)blend.c \
		$(MESABUILDDIR)buffers.c \
		$(MESABUILDDIR)clip.c \
		$(MESABUILDDIR)colortab.c \
		$(MESABUILDDIR)config.c \
		$(MESABUILDDIR)context.c \
		$(MESABUILDDIR)copypix.c \
		$(MESABUILDDIR)cva.c \
		$(MESABUILDDIR)debug_xform.c \
		$(MESABUILDDIR)depth.c \
		$(DISPATCHSRCS) \
		$(MESABUILDDIR)dlist.c \
		$(MESABUILDDIR)drawpix.c \
		$(MESABUILDDIR)enable.c \
		$(MESABUILDDIR)enums.c \
		$(MESABUILDDIR)eval.c \
		$(MESABUILDDIR)extensions.c \
		$(MESABUILDDIR)feedback.c \
		$(MESABUILDDIR)fog.c \
		$(MESABUILDDIR)get.c \
		$(GLAPISRCS) \
		$(GLAPINOOPSRCS) \
		$(GLTHREADSRCS) \
		$(MESABUILDDIR)hash.c \
		$(MESABUILDDIR)hint.c \
		$(MESABUILDDIR)image.c \
		$(MESABUILDDIR)imaging.c \
		$(MESABUILDDIR)light.c \
		$(MESABUILDDIR)lines.c \
		$(MESABUILDDIR)logic.c \
		$(MESABUILDDIR)masking.c \
		$(MESABUILDDIR)matrix.c \
		$(MESABUILDDIR)mem.c \
		$(MESABUILDDIR)mmath.c \
		$(MESABUILDDIR)pb.c \
		$(MESABUILDDIR)pipeline.c \
		$(MESABUILDDIR)pixel.c \
		$(MESABUILDDIR)pixeltex.c \
		$(MESABUILDDIR)points.c \
		$(MESABUILDDIR)polygon.c \
		$(MESABUILDDIR)quads.c \
		$(MESABUILDDIR)rastpos.c \
		$(MESABUILDDIR)readpix.c \
		$(MESABUILDDIR)rect.c \
		$(MESABUILDDIR)scissor.c \
		$(MESABUILDDIR)shade.c \
		$(MESABUILDDIR)span.c \
		$(MESABUILDDIR)stages.c \
		$(MESABUILDDIR)state.c \
		$(MESABUILDDIR)stencil.c \
		$(MESABUILDDIR)texformat.c \
		$(MESABUILDDIR)teximage.c \
		$(MESABUILDDIR)texobj.c \
		$(MESABUILDDIR)texstate.c \
		$(MESABUILDDIR)texture.c \
		$(MESABUILDDIR)texutil.c \
		$(MESABUILDDIR)translate.c \
		$(MESABUILDDIR)triangle.c \
		$(MESABUILDDIR)varray.c \
		$(MESABUILDDIR)vb.c \
		$(MESABUILDDIR)vbcull.c \
		$(MESABUILDDIR)vbfill.c \
		$(MESABUILDDIR)vbindirect.c \
		$(MESABUILDDIR)vbrender.c \
		$(MESABUILDDIR)vbxform.c \
		$(MESABUILDDIR)vector.c \
		$(MESABUILDDIR)vertices.c \
		$(MESABUILDDIR)winpos.c \
		$(MESABUILDDIR)xform.c \
		$(MESABUILDDIR)zoom.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(aatriangle.c, $(MESASRCDIR)/src)
LinkSourceFile(accum.c, $(MESASRCDIR)/src)
LinkSourceFile(alpha.c, $(MESASRCDIR)/src)
LinkSourceFile(alphabuf.c, $(MESASRCDIR)/src)
LinkSourceFile(attrib.c, $(MESASRCDIR)/src)
LinkSourceFile(bbox.c, $(MESASRCDIR)/src)
LinkSourceFile(bitmap.c, $(MESASRCDIR)/src)
LinkSourceFile(blend.c, $(MESASRCDIR)/src)
LinkSourceFile(buffers.c, $(MESASRCDIR)/src)
LinkSourceFile(clip.c, $(MESASRCDIR)/src)
LinkSourceFile(colortab.c, $(MESASRCDIR)/src)
LinkSourceFile(config.c, $(MESASRCDIR)/src)
LinkSourceFile(context.c, $(MESASRCDIR)/src)
LinkSourceFile(copypix.c, $(MESASRCDIR)/src)
LinkSourceFile(cva.c, $(MESASRCDIR)/src)
LinkSourceFile(debug_xform.c, $(MESASRCDIR)/src)
LinkSourceFile(depth.c, $(MESASRCDIR)/src)
#ifdef NeedAllMesaSrc
LinkSourceFile(dispatch.c, $(MESASRCDIR)/src)
#endif
LinkSourceFile(dlist.c, $(MESASRCDIR)/src)
LinkSourceFile(drawpix.c, $(MESASRCDIR)/src)
LinkSourceFile(enable.c, $(MESASRCDIR)/src)
LinkSourceFile(enums.c, $(MESASRCDIR)/src)
LinkSourceFile(eval.c, $(MESASRCDIR)/src)
LinkSourceFile(extensions.c, $(MESASRCDIR)/src)
LinkSourceFile(feedback.c, $(MESASRCDIR)/src)
LinkSourceFile(fog.c, $(MESASRCDIR)/src)
LinkSourceFile(get.c, $(MESASRCDIR)/src)
#ifdef NeedAllMesaSrc
LinkSourceFile(glapi.c, $(MESASRCDIR)/src)
LinkSourceFile(glapinoop.c, $(MESASRCDIR)/src)
LinkSourceFile(glthread.c, $(MESASRCDIR)/src)
#endif
LinkSourceFile(hash.c, $(MESASRCDIR)/src)
LinkSourceFile(hint.c, $(MESASRCDIR)/src)
LinkSourceFile(image.c, $(MESASRCDIR)/src)
LinkSourceFile(imaging.c, $(MESASRCDIR)/src)
LinkSourceFile(light.c, $(MESASRCDIR)/src)
LinkSourceFile(lines.c, $(MESASRCDIR)/src)
LinkSourceFile(logic.c, $(MESASRCDIR)/src)
LinkSourceFile(masking.c, $(MESASRCDIR)/src)
LinkSourceFile(matrix.c, $(MESASRCDIR)/src)
LinkSourceFile(mem.c, $(MESASRCDIR)/src)
LinkSourceFile(mmath.c, $(MESASRCDIR)/src)
LinkSourceFile(pb.c, $(MESASRCDIR)/src)
LinkSourceFile(pipeline.c, $(MESASRCDIR)/src)
LinkSourceFile(pixel.c, $(MESASRCDIR)/src)
LinkSourceFile(pixeltex.c, $(MESASRCDIR)/src)
LinkSourceFile(points.c, $(MESASRCDIR)/src)
LinkSourceFile(polygon.c, $(MESASRCDIR)/src)
LinkSourceFile(quads.c, $(MESASRCDIR)/src)
LinkSourceFile(rastpos.c, $(MESASRCDIR)/src)
LinkSourceFile(readpix.c, $(MESASRCDIR)/src)
LinkSourceFile(rect.c, $(MESASRCDIR)/src)
LinkSourceFile(scissor.c, $(MESASRCDIR)/src)
LinkSourceFile(shade.c, $(MESASRCDIR)/src)
LinkSourceFile(span.c, $(MESASRCDIR)/src)
LinkSourceFile(stages.c, $(MESASRCDIR)/src)
LinkSourceFile(state.c, $(MESASRCDIR)/src)
LinkSourceFile(stencil.c, $(MESASRCDIR)/src)
LinkSourceFile(texformat.c, $(MESASRCDIR)/src)
LinkSourceFile(teximage.c, $(MESASRCDIR)/src)
LinkSourceFile(texobj.c, $(MESASRCDIR)/src)
LinkSourceFile(texstate.c, $(MESASRCDIR)/src)
LinkSourceFile(texture.c, $(MESASRCDIR)/src)
LinkSourceFile(texutil.c, $(MESASRCDIR)/src)
LinkSourceFile(translate.c, $(MESASRCDIR)/src)
LinkSourceFile(triangle.c, $(MESASRCDIR)/src)
LinkSourceFile(varray.c, $(MESASRCDIR)/src)
LinkSourceFile(vb.c, $(MESASRCDIR)/src)
LinkSourceFile(vbcull.c, $(MESASRCDIR)/src)
LinkSourceFile(vbfill.c, $(MESASRCDIR)/src)
LinkSourceFile(vbindirect.c, $(MESASRCDIR)/src)
LinkSourceFile(vbrender.c, $(MESASRCDIR)/src)
LinkSourceFile(vbxform.c, $(MESASRCDIR)/src)
LinkSourceFile(vector.c, $(MESASRCDIR)/src)
LinkSourceFile(vertices.c, $(MESASRCDIR)/src)
LinkSourceFile(winpos.c, $(MESASRCDIR)/src)
LinkSourceFile(xform.c, $(MESASRCDIR)/src)
LinkSourceFile(zoom.c, $(MESASRCDIR)/src)
#endif

#ifdef NeedAllMesaSrc
 DISPATCHOBJS = $(MESABUILDDIR)dispatch.o
    GLAPIOBJS = $(MESABUILDDIR)glapi.o
GLAPINOOPOBJS = $(MESABUILDDIR)glapinoop.o
 GLTHREADOBJS = $(MESABUILDDIR)glthread.o
#endif

COREMESAOBJS =	$(MESABUILDDIR)aatriangle.o \
		$(MESABUILDDIR)accum.o \
		$(MESABUILDDIR)alpha.o \
		$(MESABUILDDIR)alphabuf.o \
		$(MESABUILDDIR)attrib.o \
		$(MESABUILDDIR)bbox.o \
		$(MESABUILDDIR)bitmap.o \
		$(MESABUILDDIR)blend.o \
		$(MESABUILDDIR)buffers.o \
		$(MESABUILDDIR)clip.o \
		$(MESABUILDDIR)colortab.o \
		$(MESABUILDDIR)config.o \
		$(MESABUILDDIR)context.o \
		$(MESABUILDDIR)copypix.o \
		$(MESABUILDDIR)cva.o \
		$(MESABUILDDIR)debug_xform.o \
		$(MESABUILDDIR)depth.o \
		$(MESABUILDDIR)dlist.o \
		$(DISPATCHOBJS) \
		$(MESABUILDDIR)drawpix.o \
		$(MESABUILDDIR)enable.o \
		$(MESABUILDDIR)enums.o \
		$(MESABUILDDIR)eval.o \
		$(MESABUILDDIR)extensions.o \
		$(MESABUILDDIR)feedback.o \
		$(MESABUILDDIR)fog.o \
		$(MESABUILDDIR)get.o \
		$(GLAPIOBJS) \
		$(GLAPINOOPOBJS) \
		$(GLTHREADOBJS) \
		$(MESABUILDDIR)hash.o \
		$(MESABUILDDIR)hint.o \
		$(MESABUILDDIR)image.o \
		$(MESABUILDDIR)imaging.o \
		$(MESABUILDDIR)light.o \
		$(MESABUILDDIR)lines.o \
		$(MESABUILDDIR)logic.o \
		$(MESABUILDDIR)masking.o \
		$(MESABUILDDIR)matrix.o \
		$(MESABUILDDIR)mem.o \
		$(MESABUILDDIR)mmath.o \
		$(MESABUILDDIR)pb.o \
		$(MESABUILDDIR)pipeline.o \
		$(MESABUILDDIR)pixel.o \
		$(MESABUILDDIR)pixeltex.o \
		$(MESABUILDDIR)points.o \
		$(MESABUILDDIR)polygon.o \
		$(MESABUILDDIR)quads.o \
		$(MESABUILDDIR)rastpos.o \
		$(MESABUILDDIR)readpix.o \
		$(MESABUILDDIR)rect.o \
		$(MESABUILDDIR)scissor.o \
		$(MESABUILDDIR)shade.o \
		$(MESABUILDDIR)span.o \
		$(MESABUILDDIR)stages.o \
		$(MESABUILDDIR)state.o \
		$(MESABUILDDIR)stencil.o \
		$(MESABUILDDIR)texformat.o \
		$(MESABUILDDIR)teximage.o \
		$(MESABUILDDIR)texobj.o \
		$(MESABUILDDIR)texstate.o \
		$(MESABUILDDIR)texture.o \
		$(MESABUILDDIR)texutil.o \
		$(MESABUILDDIR)translate.o \
		$(MESABUILDDIR)triangle.o \
		$(MESABUILDDIR)varray.o \
		$(MESABUILDDIR)vb.o \
		$(MESABUILDDIR)vbcull.o \
		$(MESABUILDDIR)vbfill.o \
		$(MESABUILDDIR)vbindirect.o \
		$(MESABUILDDIR)vbrender.o \
		$(MESABUILDDIR)vbxform.o \
		$(MESABUILDDIR)vector.o \
		$(MESABUILDDIR)vertices.o \
		$(MESABUILDDIR)winpos.o \
		$(MESABUILDDIR)xform.o \
		$(MESABUILDDIR)zoom.o

#ifdef NeedAllMesaSrc
 DISPATCHUOBJS = $(MESABUILDDIR)unshared/dispatch.o
    GLAPIUOBJS = $(MESABUILDDIR)unshared/glapi.o
GLAPINOOPUOBJS = $(MESABUILDDIR)unshared/glapinoop.o
 GLTHREADUOBJS = $(MESABUILDDIR)unshared/glthread.o
#endif

COREMESAUOBJS =	$(MESABUILDDIR)unshared/aatriangle.o \
		$(MESABUILDDIR)unshared/accum.o \
		$(MESABUILDDIR)unshared/alpha.o \
		$(MESABUILDDIR)unshared/alphabuf.o \
		$(MESABUILDDIR)unshared/attrib.o \
		$(MESABUILDDIR)unshared/bbox.o \
		$(MESABUILDDIR)unshared/bitmap.o \
		$(MESABUILDDIR)unshared/blend.o \
		$(MESABUILDDIR)unshared/buffers.o \
		$(MESABUILDDIR)unshared/clip.o \
		$(MESABUILDDIR)unshared/colortab.o \
		$(MESABUILDDIR)unshared/config.o \
		$(MESABUILDDIR)unshared/context.o \
		$(MESABUILDDIR)unshared/copypix.o \
		$(MESABUILDDIR)unshared/cva.o \
		$(MESABUILDDIR)unshared/debug_xform.o \
		$(MESABUILDDIR)unshared/depth.o \
		$(DISPATCHUOBJS) \
		$(MESABUILDDIR)unshared/dlist.o \
		$(MESABUILDDIR)unshared/drawpix.o \
		$(MESABUILDDIR)unshared/enable.o \
		$(MESABUILDDIR)unshared/enums.o \
		$(MESABUILDDIR)unshared/eval.o \
		$(MESABUILDDIR)unshared/extensions.o \
		$(MESABUILDDIR)unshared/feedback.o \
		$(MESABUILDDIR)unshared/fog.o \
		$(MESABUILDDIR)unshared/get.o \
		$(GLAPIUOBJS) \
		$(GLAPINOOPUOBJS) \
		$(GLTHREADUOBJS) \
		$(MESABUILDDIR)unshared/hash.o \
		$(MESABUILDDIR)unshared/hint.o \
		$(MESABUILDDIR)unshared/image.o \
		$(MESABUILDDIR)unshared/imaging.o \
		$(MESABUILDDIR)unshared/light.o \
		$(MESABUILDDIR)unshared/lines.o \
		$(MESABUILDDIR)unshared/logic.o \
		$(MESABUILDDIR)unshared/masking.o \
		$(MESABUILDDIR)unshared/matrix.o \
		$(MESABUILDDIR)unshared/mem.o \
		$(MESABUILDDIR)unshared/mmath.o \
		$(MESABUILDDIR)unshared/pb.o \
		$(MESABUILDDIR)unshared/pipeline.o \
		$(MESABUILDDIR)unshared/pixel.o \
		$(MESABUILDDIR)unshared/pixeltex.o \
		$(MESABUILDDIR)unshared/points.o \
		$(MESABUILDDIR)unshared/polygon.o \
		$(MESABUILDDIR)unshared/quads.o \
		$(MESABUILDDIR)unshared/rastpos.o \
		$(MESABUILDDIR)unshared/readpix.o \
		$(MESABUILDDIR)unshared/rect.o \
		$(MESABUILDDIR)unshared/scissor.o \
		$(MESABUILDDIR)unshared/shade.o \
		$(MESABUILDDIR)unshared/span.o \
		$(MESABUILDDIR)unshared/stages.o \
		$(MESABUILDDIR)unshared/state.o \
		$(MESABUILDDIR)unshared/stencil.o \
		$(MESABUILDDIR)unshared/texformat.o \
		$(MESABUILDDIR)unshared/teximage.o \
		$(MESABUILDDIR)unshared/texobj.o \
		$(MESABUILDDIR)unshared/texstate.o \
		$(MESABUILDDIR)unshared/texture.o \
		$(MESABUILDDIR)unshared/texutil.o \
		$(MESABUILDDIR)unshared/translate.o \
		$(MESABUILDDIR)unshared/triangle.o \
		$(MESABUILDDIR)unshared/varray.o \
		$(MESABUILDDIR)unshared/vb.o \
		$(MESABUILDDIR)unshared/vbcull.o \
		$(MESABUILDDIR)unshared/vbfill.o \
		$(MESABUILDDIR)unshared/vbindirect.o \
		$(MESABUILDDIR)unshared/vbrender.o \
		$(MESABUILDDIR)unshared/vbxform.o \
		$(MESABUILDDIR)unshared/vector.o \
		$(MESABUILDDIR)unshared/vertices.o \
		$(MESABUILDDIR)unshared/winpos.o \
		$(MESABUILDDIR)unshared/xform.o \
		$(MESABUILDDIR)unshared/zoom.o

#ifdef NeedAllMesaSrc
 DISPATCHDOBJS = $(MESABUILDDIR)debugger/dispatch.o
    GLAPIDOBJS = $(MESABUILDDIR)debugger/glapi.o
GLAPINOOPDOBJS = $(MESABUILDDIR)debugger/glapinoop.o
 GLTHREADDOBJS = $(MESABUILDDIR)debugger/glthread.o
#endif

COREMESADOBJS =	$(MESABUILDDIR)debugger/aatriangle.o \
		$(MESABUILDDIR)debugger/unshared/accum.o \
		$(MESABUILDDIR)debugger/alpha.o \
		$(MESABUILDDIR)debugger/alphabuf.o \
		$(MESABUILDDIR)debugger/attrib.o \
		$(MESABUILDDIR)debugger/bbox.o \
		$(MESABUILDDIR)debugger/bitmap.o \
		$(MESABUILDDIR)debugger/blend.o \
		$(MESABUILDDIR)debugger/buffers.o \
		$(MESABUILDDIR)debugger/clip.o \
		$(MESABUILDDIR)debugger/colortab.o \
		$(MESABUILDDIR)debugger/config.o \
		$(MESABUILDDIR)debugger/context.o \
		$(MESABUILDDIR)debugger/copypix.o \
		$(MESABUILDDIR)debugger/cva.o \
		$(MESABUILDDIR)debugger/debug_xform.o \
		$(MESABUILDDIR)debugger/depth.o \
		$(DISPATCHDOBJS) \
		$(MESABUILDDIR)debugger/dlist.o \
		$(MESABUILDDIR)debugger/drawpix.o \
		$(MESABUILDDIR)debugger/enable.o \
		$(MESABUILDDIR)debugger/enums.o \
		$(MESABUILDDIR)debugger/eval.o \
		$(MESABUILDDIR)debugger/extensions.o \
		$(MESABUILDDIR)debugger/feedback.o \
		$(MESABUILDDIR)debugger/fog.o \
		$(MESABUILDDIR)debugger/get.o \
		$(GLAPIUOBJS) \
		$(GLAPINOOPUOBJS) \
		$(GLTHREADUOBJS) \
		$(MESABUILDDIR)debugger/hash.o \
		$(MESABUILDDIR)debugger/hint.o \
		$(MESABUILDDIR)debugger/image.o \
		$(MESABUILDDIR)debugger/imaging.o \
		$(MESABUILDDIR)debugger/light.o \
		$(MESABUILDDIR)debugger/lines.o \
		$(MESABUILDDIR)debugger/logic.o \
		$(MESABUILDDIR)debugger/masking.o \
		$(MESABUILDDIR)debugger/matrix.o \
		$(MESABUILDDIR)debugger/mem.o \
		$(MESABUILDDIR)debugger/mmath.o \
		$(MESABUILDDIR)debugger/pb.o \
		$(MESABUILDDIR)debugger/pipeline.o \
		$(MESABUILDDIR)debugger/pixel.o \
		$(MESABUILDDIR)debugger/pixeltex.o \
		$(MESABUILDDIR)debugger/points.o \
		$(MESABUILDDIR)debugger/polygon.o \
		$(MESABUILDDIR)debugger/quads.o \
		$(MESABUILDDIR)debugger/rastpos.o \
		$(MESABUILDDIR)debugger/readpix.o \
		$(MESABUILDDIR)debugger/rect.o \
		$(MESABUILDDIR)debugger/scissor.o \
		$(MESABUILDDIR)debugger/shade.o \
		$(MESABUILDDIR)debugger/span.o \
		$(MESABUILDDIR)debugger/stages.o \
		$(MESABUILDDIR)debugger/state.o \
		$(MESABUILDDIR)debugger/stencil.o \
		$(MESABUILDDIR)debugger/texformat.o \
		$(MESABUILDDIR)debugger/teximage.o \
		$(MESABUILDDIR)debugger/texobj.o \
		$(MESABUILDDIR)debugger/texstate.o \
		$(MESABUILDDIR)debugger/texture.o \
		$(MESABUILDDIR)debugger/texutil.o \
		$(MESABUILDDIR)debugger/translate.o \
		$(MESABUILDDIR)debugger/triangle.o \
		$(MESABUILDDIR)debugger/varray.o \
		$(MESABUILDDIR)debugger/vb.o \
		$(MESABUILDDIR)debugger/vbcull.o \
		$(MESABUILDDIR)debugger/vbfill.o \
		$(MESABUILDDIR)debugger/vbindirect.o \
		$(MESABUILDDIR)debugger/vbrender.o \
		$(MESABUILDDIR)debugger/vbxform.o \
		$(MESABUILDDIR)debugger/vector.o \
		$(MESABUILDDIR)debugger/vertices.o \
		$(MESABUILDDIR)debugger/winpos.o \
		$(MESABUILDDIR)debugger/xform.o \
		$(MESABUILDDIR)debugger/zoom.o

#ifdef NeedAllMesaSrc
 DISPATCHPOBJS = $(MESABUILDDIR)profiled/dispatch.o
    GLAPIPOBJS = $(MESABUILDDIR)profiled/glapi.o
GLAPINOOPPOBJS = $(MESABUILDDIR)profiled/glapinoop.o
 GLTHREADPOBJS = $(MESABUILDDIR)profiled/glthread.o
#endif

COREMESAPOBJS =	$(MESABUILDDIR)profiled/aatriangle.o \
		$(MESABUILDDIR)profiled/unshared/accum.o \
		$(MESABUILDDIR)profiled/alpha.o \
		$(MESABUILDDIR)profiled/alphabuf.o \
		$(MESABUILDDIR)profiled/attrib.o \
		$(MESABUILDDIR)profiled/bbox.o \
		$(MESABUILDDIR)profiled/bitmap.o \
		$(MESABUILDDIR)profiled/blend.o \
		$(MESABUILDDIR)profiled/buffers.o \
		$(MESABUILDDIR)profiled/clip.o \
		$(MESABUILDDIR)profiled/colortab.o \
		$(MESABUILDDIR)profiled/config.o \
		$(MESABUILDDIR)profiled/context.o \
		$(MESABUILDDIR)profiled/copypix.o \
		$(MESABUILDDIR)profiled/cva.o \
		$(MESABUILDDIR)profiled/debug_xform.o \
		$(MESABUILDDIR)profiled/depth.o \
		$(DISPATCHPOBJS) \
		$(MESABUILDDIR)profiled/dlist.o \
		$(MESABUILDDIR)profiled/drawpix.o \
		$(MESABUILDDIR)profiled/enable.o \
		$(MESABUILDDIR)profiled/enums.o \
		$(MESABUILDDIR)profiled/eval.o \
		$(MESABUILDDIR)profiled/extensions.o \
		$(MESABUILDDIR)profiled/feedback.o \
		$(MESABUILDDIR)profiled/fog.o \
		$(MESABUILDDIR)profiled/get.o \
		$(GLAPIPOBJS) \
		$(GLAPINOOPPOBJS) \
		$(GLTHREADPOBJS) \
		$(MESABUILDDIR)profiled/hash.o \
		$(MESABUILDDIR)profiled/hint.o \
		$(MESABUILDDIR)profiled/image.o \
		$(MESABUILDDIR)profiled/imaging.o \
		$(MESABUILDDIR)profiled/light.o \
		$(MESABUILDDIR)profiled/lines.o \
		$(MESABUILDDIR)profiled/logic.o \
		$(MESABUILDDIR)profiled/masking.o \
		$(MESABUILDDIR)profiled/matrix.o \
		$(MESABUILDDIR)profiled/mem.o \
		$(MESABUILDDIR)profiled/mmath.o \
		$(MESABUILDDIR)profiled/pb.o \
		$(MESABUILDDIR)profiled/pipeline.o \
		$(MESABUILDDIR)profiled/pixel.o \
		$(MESABUILDDIR)profiled/pixeltex.o \
		$(MESABUILDDIR)profiled/points.o \
		$(MESABUILDDIR)profiled/polygon.o \
		$(MESABUILDDIR)profiled/quads.o \
		$(MESABUILDDIR)profiled/rastpos.o \
		$(MESABUILDDIR)profiled/readpix.o \
		$(MESABUILDDIR)profiled/rect.o \
		$(MESABUILDDIR)profiled/scissor.o \
		$(MESABUILDDIR)profiled/shade.o \
		$(MESABUILDDIR)profiled/span.o \
		$(MESABUILDDIR)profiled/stages.o \
		$(MESABUILDDIR)profiled/state.o \
		$(MESABUILDDIR)profiled/stencil.o \
		$(MESABUILDDIR)profiled/texformat.o \
		$(MESABUILDDIR)profiled/teximage.o \
		$(MESABUILDDIR)profiled/texobj.o \
		$(MESABUILDDIR)profiled/texstate.o \
		$(MESABUILDDIR)profiled/texture.o \
		$(MESABUILDDIR)profiled/texutil.o \
		$(MESABUILDDIR)profiled/translate.o \
		$(MESABUILDDIR)profiled/triangle.o \
		$(MESABUILDDIR)profiled/varray.o \
		$(MESABUILDDIR)profiled/vb.o \
		$(MESABUILDDIR)profiled/vbcull.o \
		$(MESABUILDDIR)profiled/vbfill.o \
		$(MESABUILDDIR)profiled/vbindirect.o \
		$(MESABUILDDIR)profiled/vbrender.o \
		$(MESABUILDDIR)profiled/vbxform.o \
		$(MESABUILDDIR)profiled/vector.o \
		$(MESABUILDDIR)profiled/vertices.o \
		$(MESABUILDDIR)profiled/winpos.o \
		$(MESABUILDDIR)profiled/xform.o \
		$(MESABUILDDIR)profiled/zoom.o

