/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_rop.h,v 1.1.2.1 1999/07/30 11:21:36 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
/* This file contains the data structures which map the X ROPs to the
 * ViRGE ROPs. It also contains other mappings which are used when supporting
 * planemasks and transparency.
 *
 * Created by Sebastien Marineau, 29/03/97.
 * This file should be included only from s3v_accel.c to avoid 
 * duplicate symbols. 
 * 
 */

#include "regs3sav.h"

static int s3vAlu[16] =
{
   ROP_0,               /* GXclear */
   ROP_DSa,             /* GXand */
   ROP_SDna,            /* GXandReverse */
   ROP_S,               /* GXcopy */
   ROP_DSna,            /* GXandInverted */
   ROP_D,               /* GXnoop */
   ROP_DSx,             /* GXxor */
   ROP_DSo,             /* GXor */
   ROP_DSon,            /* GXnor */
   ROP_DSxn,            /* GXequiv */
   ROP_Dn,              /* GXinvert*/
   ROP_SDno,            /* GXorReverse */
   ROP_Sn,              /* GXcopyInverted */
   ROP_DSno,            /* GXorInverted */
   ROP_DSan,            /* GXnand */
   ROP_1                /* GXset */
};

/* S -> P, for solid and pattern fills. */
static int s3vAlu_sp[16]=
{
   ROP_0,
   ROP_DPa,
   ROP_PDna,
   ROP_P,
   ROP_DPna,
   ROP_D,
   ROP_DPx,
   ROP_DPo,
   ROP_DPon,
   ROP_DPxn,
   ROP_Dn,
   ROP_PDno,
   ROP_Pn,
   ROP_DPno,
   ROP_DPan,
   ROP_1
};

/* ROP  ->  (ROP & P) | (D & ~P) */
/* These are used to support a planemask for S->D ops */
static int s3vAlu_pat[16] =
{
   ROP_0_PaDPnao,
   ROP_DSa_PaDPnao,
   ROP_SDna_PaDPnao,
   ROP_S_PaDPnao,
   ROP_DSna_PaDPnao,
   ROP_D_PaDPnao,
   ROP_DSx_PaDPnao,
   ROP_DSo_PaDPnao,
   ROP_DSon_PaDPnao,
   ROP_DSxn_PaDPnao,
   ROP_Dn_PaDPnao,
   ROP_SDno_PaDPnao,
   ROP_Sn_PaDPnao,
   ROP_DSno_PaDPnao,
   ROP_DSan_PaDPnao,
   ROP_1_PaDPnao
};

/* ROP_sp -> (ROP_sp & S) | (D & ~S) */
/* This is used for our transparent mono pattern fills to support trans/plane*/
static int s3vAlu_MonoTrans[16] =
{
   ROP_0_SaDSnao,
   ROP_DPa_SaDSnao,
   ROP_PDna_SaDSnao,
   ROP_P_SaDSnao,
   ROP_DPna_SaDSnao,
   ROP_D_SaDSnao,
   ROP_DPx_SaDSnao,
   ROP_DPo_SaDSnao,
   ROP_DPon_SaDSnao,
   ROP_DPxn_SaDSnao,
   ROP_Dn_SaDSnao,
   ROP_PDno_SaDSnao,
   ROP_Pn_SaDSnao,
   ROP_DPno_SaDSnao,
   ROP_DPan_SaDSnao,
   ROP_1_SaDSnao
};
