/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_inithw.c,v 1.7 2001/01/31 16:15:37 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include "gamma_init.h"
#include "glint_dri.h"

void gammaInitHW(gammaContextPrivate *gcp)
{
    __DRIscreenPrivate *driScrnPriv = gcp->gammaScrnPriv->driScrnPriv;
    GLINTDRIPtr         gDRIPriv = (GLINTDRIPtr)driScrnPriv->pDevPriv;

    if (gDRIPriv->numMultiDevices == 2) {
	/* Set up each MX's ScanLineOwnership for OpenGL */
	CHECK_DMA_BUFFER(nullCC, gcp, 4);
	WRITE(gcp->buf, BroadcastMask, 1);
	WRITE(gcp->buf, ScanLineOwnership, 5); /* Use bottom left as [0,0] */
	WRITE(gcp->buf, BroadcastMask, 2);
	WRITE(gcp->buf, ScanLineOwnership, 1); /* Use bottom left as [0,0] */

	/* Broadcast to both MX's */
	CHECK_DMA_BUFFER(nullCC, gcp, 1);
	WRITE(gcp->buf, BroadcastMask, 3);
    }

    /* Set MXs to known state */
    CHECK_DMA_BUFFER(nullCC, gcp, 27);
    WRITE(gcp->buf, RasterizerMode, 0);
    WRITE(gcp->buf, AreaStippleMode, 0);
    WRITE(gcp->buf, LineStippleMode, 0);
    WRITE(gcp->buf, ScissorMode, 0);
    WRITE(gcp->buf, RouterMode, 0);
    WRITE(gcp->buf, TextureAddressMode, 0);
    WRITE(gcp->buf, TextureReadMode, 0);
    WRITE(gcp->buf, TextureFilterMode, 0);
    WRITE(gcp->buf, ColorDDAMode, 0);
    WRITE(gcp->buf, TextureColorMode, 0);
    WRITE(gcp->buf, FogMode, 0);
    WRITE(gcp->buf, AntialiasMode, 0);
    WRITE(gcp->buf, AlphaTestMode, 0);
    WRITE(gcp->buf, LBReadMode, 0);
    WRITE(gcp->buf, GLINTWindow, 0);
    WRITE(gcp->buf, StencilMode, 0);
    WRITE(gcp->buf, DepthMode, 0);
    WRITE(gcp->buf, LBWriteMode, 0);
    WRITE(gcp->buf, FBReadMode, 0);
    WRITE(gcp->buf, PatternRamMode, 0);
    WRITE(gcp->buf, AlphaBlendMode, 0);
    WRITE(gcp->buf, ChromaTestMode, 0);
    WRITE(gcp->buf, DitherMode, 0);
    WRITE(gcp->buf, LogicalOpMode, 0);
    WRITE(gcp->buf, FBWriteMode, 0);
    WRITE(gcp->buf, StatisticMode, 0);
    WRITE(gcp->buf, PixelSize, 0);

    /* Set Gamma to known state */
    CHECK_DMA_BUFFER(nullCC, gcp, 10);
    WRITE(gcp->buf, TriangleMode, 0);
    WRITE(gcp->buf, GeometryMode, 0);
    WRITE(gcp->buf, NormalizeMode, 0);
    WRITE(gcp->buf, LightingMode, 0);
    WRITE(gcp->buf, ColorMaterialMode, 0);
    WRITE(gcp->buf, MaterialMode, 0);
    WRITE(gcp->buf, PointMode, 0);
    WRITE(gcp->buf, LineMode, 0);
    WRITE(gcp->buf, TransformMode, 0);
    WRITE(gcp->buf, DeltaMode, 0);

#ifdef FORCE_DEPTH32
    CHECK_DMA_BUFFER(nullCC, gcp, 2);
    WRITE(gcp->buf, LBWriteFormat, 0x00000d4a);
    WRITE(gcp->buf, LBReadFormat,  0x00000d4a);
#endif

    /* Framebuffer initialization */
    CHECK_DMA_BUFFER(nullCC, gcp, 10);
    WRITE(gcp->buf, FBSourceData, 0);
    WRITE(gcp->buf, FBReadMode, gcp->FBReadMode);
    if (gcp->EnabledFlags & GAMMA_BACK_BUFFER)
	WRITE(gcp->buf, FBPixelOffset,
	      (driScrnPriv->fbHeight/2)*driScrnPriv->fbWidth);
    else
	WRITE(gcp->buf, FBPixelOffset, 0);
    WRITE(gcp->buf, FBSourceOffset, 0);
    WRITE(gcp->buf, FBHardwareWriteMask, 0xffffffff);
    WRITE(gcp->buf, FBSoftwareWriteMask, 0xffffffff);
    WRITE(gcp->buf, FBWriteMode, FBWriteModeEnable);
    WRITE(gcp->buf, FBWindowBase, gcp->FBWindowBase);
    WRITE(gcp->buf, ScreenSize, ((driScrnPriv->fbHeight << 16) |
				 (driScrnPriv->fbWidth)));
    WRITE(gcp->buf, WindowOrigin, 0x00000000);

    /* Localbuffer initialization */
    CHECK_DMA_BUFFER(nullCC, gcp, 5);
    WRITE(gcp->buf, LBReadMode, gcp->LBReadMode);
    WRITE(gcp->buf, LBSourceOffset, 0);
    WRITE(gcp->buf, LBWriteMode, LBWriteModeEnable);
    WRITE(gcp->buf, LBWindowOffset, 0);
    WRITE(gcp->buf, LBWindowBase, gcp->LBWindowBase);

    CHECK_DMA_BUFFER(nullCC, gcp, 1);
    WRITE(gcp->buf, Rectangle2DControl, 1);

    CHECK_DMA_BUFFER(nullCC, gcp, 11);
    WRITE(gcp->buf, DepthMode, gcp->DepthMode);
    WRITE(gcp->buf, ColorDDAMode, gcp->ColorDDAMode);
    WRITE(gcp->buf, FBBlockColor, 0x00000000);
    WRITE(gcp->buf, ConstantColor, 0x00000000);
    WRITE(gcp->buf, AlphaTestMode, gcp->AlphaTestMode);
    WRITE(gcp->buf, AlphaBlendMode, gcp->AlphaBlendMode);
    WRITE(gcp->buf, DitherMode, DitherModeEnable | DM_ColorOrder_RGB);
    if (gDRIPriv->numMultiDevices == 2)
    	WRITE(gcp->buf, RasterizerMode, RM_MultiGLINT | RM_BiasCoordNearHalf);
    else
    	WRITE(gcp->buf, RasterizerMode, RM_BiasCoordNearHalf);
    WRITE(gcp->buf, GLINTWindow, gcp->Window);
    WRITE(gcp->buf, FastClearDepth, 0xffffffff);
    WRITE(gcp->buf, GLINTDepth, 0xffffffff);

    CHECK_DMA_BUFFER(nullCC, gcp, 1);
    WRITE(gcp->buf, TextureColorMode, gcp->curTexObj->TextureColorMode);

    CHECK_DMA_BUFFER(nullCC, gcp, 1);
    WRITE(gcp->buf, EdgeFlag, EdgeFlagEnable);

    CHECK_DMA_BUFFER(nullCC, gcp, 16);
    WRITEF(gcp->buf, ModelViewMatrix0,  1.0);
    WRITEF(gcp->buf, ModelViewMatrix1,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix2,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix3,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix4,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix5,  1.0);
    WRITEF(gcp->buf, ModelViewMatrix6,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix7,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix8,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix9,  0.0);
    WRITEF(gcp->buf, ModelViewMatrix10, 1.0);
    WRITEF(gcp->buf, ModelViewMatrix11, 0.0);
    WRITEF(gcp->buf, ModelViewMatrix12, 0.0);
    WRITEF(gcp->buf, ModelViewMatrix13, 0.0);
    WRITEF(gcp->buf, ModelViewMatrix14, 0.0);
    WRITEF(gcp->buf, ModelViewMatrix15, 1.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 16);
    WRITEF(gcp->buf, ModelViewProjectionMatrix0,  1.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix1,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix2,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix3,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix4,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix5,  1.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix6,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix7,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix8,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix9,  0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix10, 1.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix11, 0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix12, 0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix13, 0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix14, 0.0);
    WRITEF(gcp->buf, ModelViewProjectionMatrix15, 1.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 16);
    WRITEF(gcp->buf, TextureMatrix0,  1.0);
    WRITEF(gcp->buf, TextureMatrix1,  0.0);
    WRITEF(gcp->buf, TextureMatrix2,  0.0);
    WRITEF(gcp->buf, TextureMatrix3,  0.0);
    WRITEF(gcp->buf, TextureMatrix4,  0.0);
    WRITEF(gcp->buf, TextureMatrix5,  1.0);
    WRITEF(gcp->buf, TextureMatrix6,  0.0);
    WRITEF(gcp->buf, TextureMatrix7,  0.0);
    WRITEF(gcp->buf, TextureMatrix8,  0.0);
    WRITEF(gcp->buf, TextureMatrix9,  0.0);
    WRITEF(gcp->buf, TextureMatrix10, 1.0);
    WRITEF(gcp->buf, TextureMatrix11, 0.0);
    WRITEF(gcp->buf, TextureMatrix12, 0.0);
    WRITEF(gcp->buf, TextureMatrix13, 0.0);
    WRITEF(gcp->buf, TextureMatrix14, 0.0);
    WRITEF(gcp->buf, TextureMatrix15, 1.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 16);
    WRITEF(gcp->buf, TexGen0,  0.0);
    WRITEF(gcp->buf, TexGen1,  0.0);
    WRITEF(gcp->buf, TexGen2,  0.0);
    WRITEF(gcp->buf, TexGen3,  0.0);
    WRITEF(gcp->buf, TexGen4,  0.0);
    WRITEF(gcp->buf, TexGen5,  0.0);
    WRITEF(gcp->buf, TexGen6,  0.0);
    WRITEF(gcp->buf, TexGen7,  0.0);
    WRITEF(gcp->buf, TexGen8,  0.0);
    WRITEF(gcp->buf, TexGen9,  0.0);
    WRITEF(gcp->buf, TexGen10, 0.0);
    WRITEF(gcp->buf, TexGen11, 0.0);
    WRITEF(gcp->buf, TexGen12, 0.0);
    WRITEF(gcp->buf, TexGen13, 0.0);
    WRITEF(gcp->buf, TexGen14, 0.0);
    WRITEF(gcp->buf, TexGen15, 0.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 9);
    WRITEF(gcp->buf, NormalMatrix0, 1.0);
    WRITEF(gcp->buf, NormalMatrix1, 0.0);
    WRITEF(gcp->buf, NormalMatrix2, 0.0);
    WRITEF(gcp->buf, NormalMatrix3, 0.0);
    WRITEF(gcp->buf, NormalMatrix4, 1.0);
    WRITEF(gcp->buf, NormalMatrix5, 0.0);
    WRITEF(gcp->buf, NormalMatrix6, 0.0);
    WRITEF(gcp->buf, NormalMatrix7, 0.0);
    WRITEF(gcp->buf, NormalMatrix8, 1.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 3);
    WRITEF(gcp->buf, FogDensity, 0.0);
    WRITEF(gcp->buf, FogEnd,     0.0);
    WRITEF(gcp->buf, FogScale,   0.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 2);
    WRITEF(gcp->buf, LineClipLengthThreshold,   0.0);
    WRITEF(gcp->buf, TriangleClipAreaThreshold, 0.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 5);
    WRITE(gcp->buf, GeometryMode, gcp->GeometryMode);
    WRITE(gcp->buf, NormalizeMode, NormalizeModeDisable);
    WRITE(gcp->buf, LightingMode, gcp->LightingMode);
    WRITE(gcp->buf, ColorMaterialMode, ColorMaterialModeDisable);
    WRITE(gcp->buf, MaterialMode, MaterialModeDisable);

    CHECK_DMA_BUFFER(nullCC, gcp, 2);
    WRITE(gcp->buf, FrontSpecularExponent, 0); /* fixed point */
    WRITE(gcp->buf, BackSpecularExponent,  0); /* fixed point */

    CHECK_DMA_BUFFER(nullCC, gcp, 29);
    WRITEF(gcp->buf, FrontAmbientColorRed,    0.2);
    WRITEF(gcp->buf, FrontAmbientColorGreen,  0.2);
    WRITEF(gcp->buf, FrontAmbientColorBlue,   0.2);
    WRITEF(gcp->buf, BackAmbientColorRed,     0.2);
    WRITEF(gcp->buf, BackAmbientColorGreen,   0.2);
    WRITEF(gcp->buf, BackAmbientColorBlue,    0.2);
    WRITEF(gcp->buf, FrontDiffuseColorRed,    0.8);
    WRITEF(gcp->buf, FrontDiffuseColorGreen,  0.8);
    WRITEF(gcp->buf, FrontDiffuseColorBlue,   0.8);
    WRITEF(gcp->buf, BackDiffuseColorRed,     0.8);
    WRITEF(gcp->buf, BackDiffuseColorGreen,   0.8);
    WRITEF(gcp->buf, BackDiffuseColorBlue,    0.8);
    WRITEF(gcp->buf, FrontSpecularColorRed,   0.0);
    WRITEF(gcp->buf, FrontSpecularColorGreen, 0.0);
    WRITEF(gcp->buf, FrontSpecularColorBlue,  0.0);
    WRITEF(gcp->buf, BackSpecularColorRed,    0.0);
    WRITEF(gcp->buf, BackSpecularColorGreen,  0.0);
    WRITEF(gcp->buf, BackSpecularColorBlue,   0.0);
    WRITEF(gcp->buf, FrontEmissiveColorRed,   0.0);
    WRITEF(gcp->buf, FrontEmissiveColorGreen, 0.0);
    WRITEF(gcp->buf, FrontEmissiveColorBlue,  0.0);
    WRITEF(gcp->buf, BackEmissiveColorRed,    0.0);
    WRITEF(gcp->buf, BackEmissiveColorGreen,  0.0);
    WRITEF(gcp->buf, BackEmissiveColorBlue,   0.0);
    WRITEF(gcp->buf, SceneAmbientColorRed,    0.2);
    WRITEF(gcp->buf, SceneAmbientColorGreen,  0.2);
    WRITEF(gcp->buf, SceneAmbientColorBlue,   0.2);
    WRITEF(gcp->buf, FrontAlpha,              1.0);
    WRITEF(gcp->buf, BackAlpha,               1.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 8);
    WRITE(gcp->buf, PointMode, (PM_AntialiasDisable |
				PM_AntialiasQuality_4x4));
    WRITE(gcp->buf, PointSize, 1);
    WRITE(gcp->buf, LineMode, (LM_StippleDisable |
			       LM_MirrorDisable |
			       LM_AntialiasDisable |
			       LM_AntialiasQuality_4x4));
    WRITE(gcp->buf, LineWidth, 1);
    WRITE(gcp->buf, LineWidthOffset, 0);
    WRITE(gcp->buf, TriangleMode, (TM_AntialiasDisable |
				   TM_AntialiasQuality_4x4 |
				   TM_UseTriPacketInterface));
    WRITE(gcp->buf, TransformMode, (XM_UseModelViewProjMatrix |
				    XM_TexGenModeS_None |
				    XM_TexGenModeT_None |
				    XM_TexGenModeR_None |
				    XM_TexGenModeQ_None));
    WRITE(gcp->buf, DeltaMode, gcp->DeltaMode);

    CHECK_DMA_BUFFER(nullCC, gcp, 16);
    WRITE(gcp->buf, Light0Mode,  LNM_Off);
    WRITE(gcp->buf, Light1Mode,  LNM_Off);
    WRITE(gcp->buf, Light2Mode,  LNM_Off);
    WRITE(gcp->buf, Light3Mode,  LNM_Off);
    WRITE(gcp->buf, Light4Mode,  LNM_Off);
    WRITE(gcp->buf, Light5Mode,  LNM_Off);
    WRITE(gcp->buf, Light6Mode,  LNM_Off);
    WRITE(gcp->buf, Light7Mode,  LNM_Off);
    WRITE(gcp->buf, Light8Mode,  LNM_Off);
    WRITE(gcp->buf, Light9Mode,  LNM_Off);
    WRITE(gcp->buf, Light10Mode, LNM_Off);
    WRITE(gcp->buf, Light11Mode, LNM_Off);
    WRITE(gcp->buf, Light12Mode, LNM_Off);
    WRITE(gcp->buf, Light13Mode, LNM_Off);
    WRITE(gcp->buf, Light14Mode, LNM_Off);
    WRITE(gcp->buf, Light15Mode, LNM_Off);

    CHECK_DMA_BUFFER(nullCC, gcp, 22);
    WRITEF(gcp->buf, Light0AmbientIntensityBlue, 0.0);
    WRITEF(gcp->buf, Light0AmbientIntensityGreen, 0.0);
    WRITEF(gcp->buf, Light0AmbientIntensityRed, 0.0);
    WRITEF(gcp->buf, Light0DiffuseIntensityBlue, 1.0);
    WRITEF(gcp->buf, Light0DiffuseIntensityGreen, 1.0);
    WRITEF(gcp->buf, Light0DiffuseIntensityRed, 1.0);
    WRITEF(gcp->buf, Light0SpecularIntensityBlue, 1.0);
    WRITEF(gcp->buf, Light0SpecularIntensityGreen, 1.0);
    WRITEF(gcp->buf, Light0SpecularIntensityRed, 1.0);
    WRITEF(gcp->buf, Light0SpotlightDirectionZ, 0.0);
    WRITEF(gcp->buf, Light0SpotlightDirectionY, 0.0);
    WRITEF(gcp->buf, Light0SpotlightDirectionX, -1.0);
    WRITEF(gcp->buf, Light0SpotlightExponent, 0.0);
    WRITEF(gcp->buf, Light0PositionZ, 0.0);
    WRITEF(gcp->buf, Light0PositionY, 0.0);
    WRITEF(gcp->buf, Light0PositionX, 1.0);
    WRITEF(gcp->buf, Light0PositionW, 0.0);
    WRITEF(gcp->buf, Light0CosSpotlightCutoffAngle, -1.0);
    WRITEF(gcp->buf, Light0ConstantAttenuation, 1.0);
    WRITEF(gcp->buf, Light0LinearAttenuation,   0.0);
    WRITEF(gcp->buf, Light0QuadraticAttenuation,0.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 6);
    WRITEF(gcp->buf, ViewPortScaleX,  (gcp->w)/2.0);
    WRITEF(gcp->buf, ViewPortScaleY,  (gcp->h)/2.0);
    WRITEF(gcp->buf, ViewPortScaleZ,  (gcp->zFar-gcp->zNear)/2.0);
    WRITEF(gcp->buf, ViewPortOffsetX, gcp->x);
    WRITEF(gcp->buf, ViewPortOffsetY, gcp->y);
    WRITEF(gcp->buf, ViewPortOffsetZ, (gcp->zFar+gcp->zNear)/2.0);

    CHECK_DMA_BUFFER(nullCC, gcp, 3);
    WRITEF(gcp->buf, Nz, 1.0);
    WRITEF(gcp->buf, Ny, 0.0);
    WRITEF(gcp->buf, Nx, 0.0);

    /* Send the initialization commands to the HW */
    FLUSH_DMA_BUFFER(nullCC, gcp);
}

#endif
