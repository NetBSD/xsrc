/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Regs.h,v 1.1.2.3 1999/07/23 13:22:58 hohndel Exp $ */

#ifndef P9X00REGS_H
#define P9X00REGS_H

#include "p9x00Includes.h"

/*
on P9100 the following registers can be found at 0x0004
on P9000 at 0x100004 relative to linear region and swapping bits
*/
typedef struct
   {
   volatile CARD32 system_config;
#define P9X_RV_REV_MASK       ((CARD32)0x00000007L)
#define P9X_RV_WRITE_TO_BUFFER(b)   (((CARD32)(b))<<9)
#define P9X_RV_READ_FROM_BUFFER(b)  (((CARD32)(b))<<10)
#define P9X_RV_MEM_SWAP_BITS        0x00000800
#define P9X_RV_MEM_SWAP_BYTES       0x00001000
#define P9X_RV_MEM_SWAP_WORDS       0x00002000
#define P9X_RV_SHIFT(r,a)           (((CARD32)(a))<<r)
#define P9X_RV_DOUBLE_LOAD          0x01000000
#define P9X_RV_COLOR_MODE(c)        (((CARD32)(c))<<26)
#define P9X_RV_SHIFT_3(a)	     (((CARD32)(a))<<29)

   volatile CARD32 vinterrupt;

#define P9X_RV_CLEAR_IDLE           0x00000002
#define P9X_RV_CLEAR_PICK           0x00000008
#define P9X_RV_CLEAR_BLANK          0x00000020

   volatile CARD32 interrupt_enable;

#define P9X_RV_CLEAR_IDLE_INT       0x00000002L
#define P9X_RV_SET_IDLE_INT         0x00000003L
#define P9X_RV_CLEAR_PICK_INT       0x00000008L
#define P9X_RV_SET_PICK_INT         0x0000000CL
#define P9X_RV_CLEAR_BLANK_INT      0x00000020L
#define P9X_RV_SET_BLANK_INT        0x00000030L
#define P9X_RV_DISABLE_INT          0x00000080L
#define P9X_RV_ENABLE_INT           0x000000C0L

   volatile CARD32 alternate_write;

#define P9X_RV_WRITE_BANK(x)        (((CARD32)(x))<<16)
    
   volatile CARD32 alternate_read;

#define P9X_RV_READ_BANK(x)         (((CARD32)(x))<<16)

   } type_system_ctrl;
   

/*
on P9100 the following registers can be found at 0x0104
on P9000 at 0x100104 relative to linear region and swapping bits
*/
typedef struct
   {
   volatile CARD32 hor_counter;
   volatile CARD32 hor_length;
   volatile CARD32 hor_sync_rize;   
   volatile CARD32 hor_blank_rize;
   volatile CARD32 hor_blank_fall;
   volatile CARD32 hor_counter_pre;
   volatile CARD32 ver_counter;
   volatile CARD32 ver_length;
   volatile CARD32 ver_sync_rize;
   volatile CARD32 ver_blank_rize;
   volatile CARD32 ver_blank_fall;
   volatile CARD32 ver_counter_pre;
   volatile CARD32 scr_repaint_addr;
   volatile CARD32 scr_repaint_time;

#define P9X_RV_BUFFER1              0x00000008L
#define P9X_RV_RESTRICTED_MODE      0x00000010L
#define P9X_RV_NORMAL_OP            0x00000020L
#define P9X_RV_COMP_SYNC            0x00000040L
#define P9X_RV_INTERNAL_HSYNC       0x00000080L
#define P9X_RV_INTERNAL_VSYNC       0x00000100L

   volatile CARD32 scr_inc_qsf;
   volatile CARD32 sync_polarity;   

#define P9X_RV_HOR_SYNC_NEG             0x00000001L
#define P9X_RV_VER_SYNC_NEG             0x00000004L
#define P9X_RV_PM_STANDBY               0x00000002L
#define P9X_RV_PM_SUSPEND               0x00000008L
#define P9X_RV_PM_OFF                   0x0000000AL

   } type_video_ctrl;


/*
on P9100 the following registers can be found at 0x0184
on P9000 at 0x100184 relative to linear region and swapping bits
*/ 
typedef struct
   {
   volatile CARD32 memory_ctrl;    
   volatile CARD32 refresh_period;
   volatile CARD32 refresh_counter; 
   volatile CARD32 ras_low_max;
   volatile CARD32 ras_low_counter;
   volatile CARD32 power_up_config;

#define P9X_RV_DACTYPE           0x0000F000L
#define P9X_RV_CLOCKTYPE         0x000000E0L
#    define P9X_RV_ICD2061A      0x00000000L
#    define P9X_RV_OSC50MHZ      0x00000020L
#define P9X_RV_VRAMTYPE          0x00000010L
#    define P9X_RV_VRAM256       0x00000000L
#    define P9X_RV_VRAM128       0x00000010L
#define P9X_RV_SAMSIZE           0x00000008L
#    define P9X_RV_FULLSIZE      0x00000000L
#    define P9X_RV_HALFSIZE      0x00000008L
#define ADDIN                    0x00000001L
#    define P9X_RV_MOTHER        0x00000000L
#    define P9X_RV_CARD          0x00000001L

   } type_hardware_ctrl;


/*
on P9100 the following registers can be found at 0x2000
on p9000 at 0x180000 relative to linear region and swapping bits
*/
typedef struct
   {
   volatile CARD32 status;

#define P9X_RV_QUAD_STRADDLE  0x00000001L
#define P9X_RV_QUAD_INSIDE    0x00000002L
#define P9X_RV_QUAD_OUTSIDE   0x00000004L
#define P9X_RV_QUAD_CONCAVE   0x00000008L
#define P9X_RV_QUAD_ERROR     0x00000010L
#define P9X_RV_BLIT_ERROR     0x00000020L
#define P9X_RV_PIXEL_ERROR    0x00000040L
#define P9X_RV_PICK_DETECT    0x00000080L
#define P9X_RV_DRAW_BUSY      0x40000000L
#define P9X_RV_QUAD_BUSY      0x80000000L

   volatile CARD32 blit;
   volatile CARD32 quad;
   volatile CARD32 pixel8;
   volatile CARD32 nu2010h;
   volatile CARD32 next_pixel;

   CARD32 nu2018h;
   CARD32 nu201Ch;
   CARD32 nu2020h;
   CARD32 nu2024h;
   CARD32 nu2028h;
   CARD32 nu202Ch;
   CARD32 nu2030h;
   CARD32 nu2034h;
   CARD32 nu2038h;
   CARD32 nu203Ch;
   CARD32 nu2040h;
   CARD32 nu2044h;
   CARD32 nu2048h;
   CARD32 nu204Ch;
   CARD32 nu2050h;
   CARD32 nu2054h;
   CARD32 nu2058h;
   CARD32 nu205Ch;
   CARD32 nu2060h;
   CARD32 nu2064h;
   CARD32 nu2068h;
   CARD32 nu206Ch;
   CARD32 nu2070h;
   CARD32 nu2074h;
   CARD32 nu2078h;
   CARD32 nu207Ch;

   volatile CARD32 pixel1;
   } type_command;


/*
on P9100 the following registers can be found at 0x2184
on P9000 at 0x180184 relative to linear region and swapping bits
*/
typedef struct
   {
   volatile CARD32 out_of_range;
   CARD32 nu2188h;
   volatile CARD32 coord_index;
   volatile CARD32 window_offset;
   volatile CARD32 window_min_read;
   volatile CARD32 window_max_read;
   volatile CARD32 nu219Ch;
   volatile CARD32 x_clipping;
   volatile CARD32 y_clipping;
   volatile CARD32 x_edge_less;
   volatile CARD32 x_edge_greater;
   volatile CARD32 y_edge_less;
   volatile CARD32 y_edge_greater; 
   } type_parameter_ctrl;

 
   
typedef struct
   {
   volatile CARD32 color0;
   volatile CARD32 color1;
   volatile CARD32 plane_mask;
   volatile CARD32 drawing_mode;
   volatile CARD32 pattern_origin_x;
   volatile CARD32 pattern_origin_y;
   volatile CARD32 raster;

#define P9X_RV_NO_SOLID   0x00002000L
#define P9X_RV_PIX1_TRANS 0x00008000L
#define P9X_RV_OVERSIZE   0x00010000L
#define P9X_RV_TRANS      0x00020000L

   volatile CARD32 pixel8;
   volatile CARD32 window_min;
   volatile CARD32 window_max;

   CARD32 nu2228h;
   CARD32 nu222Ch;
   CARD32 nu2230h;
   CARD32 nu2234h;

   volatile CARD32 color2;
   volatile CARD32 color3;

   CARD32 nu2240h;
   CARD32 nu2244h;
   CARD32 nu2248h;
   CARD32 nu224Ch;
   CARD32 nu2250h;
   CARD32 nu2254h;
   CARD32 nu2258h;
   CARD32 nu225Ch;
   CARD32 nu2260h;
   CARD32 nu2264h;
   CARD32 nu2268h;
   CARD32 nu226Ch;
   CARD32 nu2270h;
   CARD32 nu2274h;
   CARD32 nu2278h;
   CARD32 nu227Ch;
   
   volatile CARD32 pattern0;
   volatile CARD32 pattern1;
   volatile CARD32 pattern2;
   volatile CARD32 pattern3;
   volatile CARD32 patt4soft0;
   volatile CARD32 patt5soft1;
   volatile CARD32 patt6soft2;
   volatile CARD32 patt7soft3;
   volatile CARD32 byte_win_min;
   volatile CARD32 byte_win_max;   
   } type_drawing_ctrl;


/*
This is the P9x00 base structure for all coordinate registers
*/   
typedef struct {
  /*
    Bit 0-2 of the Meta Coord Register must be zero.
    Bit 0-1 are set to zero by 32Bit word-addressing.
    Bit 2 is set to zero by simulating 64Bit word-ddressing.
    We do this by inserting not used 32Bit words.
  */
  CARD32 not_used;

  CARD32 Bit2to0_0;

  volatile CARD32 x;

  CARD32 Bit2to0_1;

  volatile CARD32 y;

  CARD32 Bit2to0_2;

  volatile CARD32 xy;

  CARD32 Bit2to0_3;
} type_coord;
   
/*
on P9100 the following registers can be found at 0x3000
on P9000 at 0x181000 relative to linear region and swapping bits
*/ 
  typedef struct {
    type_coord abs_to_scr;
    type_coord abs_to_win;
  } absolute_type;

typedef struct {
  absolute_type edge0;
  absolute_type edge1;
  absolute_type edge2;
  absolute_type edge3;
}type_device_coord;

/*
on P9100 the following registers can be found at 0x3200
on P9000 at 0x181200 relative to linear region and swapping bits
*/
  typedef struct {
    type_coord rel_to_win;
    type_coord rel_to_prev;
  } relative_type;

typedef struct {
  relative_type point;
  relative_type line;
  relative_type tri;
  relative_type quad;
  relative_type rect;
} type_meta_coord;

         

typedef struct
   {
   type_system_ctrl    *system_ctrl;
   type_video_ctrl     *video_ctrl;
   type_hardware_ctrl  *hardware_ctrl;
   volatile CARD32     *dac;
   volatile CARD32     *copro;
   type_command        *command;
   type_parameter_ctrl *parameter_ctrl;
   type_drawing_ctrl   *drawing_ctrl;
   type_device_coord   *device_coord;
   type_meta_coord     *meta_coord;
   volatile CARD32     *vram;
   } type_p9x00_registers;


#ifndef  P9X00DRIVER_C

  extern type_p9x00_registers p9x00regs;

#else /* P9X00DRIVER_C */

  type_p9x00_registers p9x00regs;

#endif /* P9X00DRIVER_C */


#define P9X_R_SYSCFG p9x00regs.system_ctrl->system_config
#define P9X_R_INT    p9x00regs.system_ctrl->vinterrupt
#define P9X_R_INTEN  p9x00regs.system_ctrl->interrupt_enable
#define P9X_R_AWRITE p9x00regs.system_ctrl->alternate_write
#define P9X_R_AREAD  p9x00regs.system_ctrl->alternate_read

#define P9X_R_HCOUNT  p9x00regs.video_ctrl->hor_counter
#define P9X_R_HT      p9x00regs.video_ctrl->hor_length
#define P9X_R_HSR     p9x00regs.video_ctrl->hor_sync_rize
#define P9X_R_HBR     p9x00regs.video_ctrl->hor_blank_rize
#define P9X_R_HBF     p9x00regs.video_ctrl->hor_blank_fall
#define P9X_R_HCP     p9x00regs.video_ctrl->hor_counter_pre
#define P9X_R_VCOUNT  p9x00regs.video_ctrl->ver_counter
#define P9X_R_VT      p9x00regs.video_ctrl->ver_length
#define P9X_R_VSR     p9x00regs.video_ctrl->ver_sync_rize
#define P9X_R_VBR     p9x00regs.video_ctrl->ver_blank_rize
#define P9X_R_VBF     p9x00regs.video_ctrl->ver_blank_fall
#define P9X_R_VCP     p9x00regs.video_ctrl->ver_counter_pre
#define P9X_R_SCRADDR p9x00regs.video_ctrl->scr_repaint_addr
#define P9X_R_SCRTIME p9x00regs.video_ctrl->scr_repaint_time
#define P9X_R_SCRINC  p9x00regs.video_ctrl->scr_inc_qsf
#define P9X_R_SYNC    p9x00regs.video_ctrl->sync_polarity

#define P9X_R_MEMCFG   p9x00regs.hardware_ctrl->memory_ctrl
#define P9X_R_REFPER   p9x00regs.hardware_ctrl->refresh_period
#define P9X_R_REFCOUNT p9x00regs.hardware_ctrl->refresh_counter
#define P9X_R_RASLOW   p9x00regs.hardware_ctrl->ras_low_max
#define P9X_R_RASCOUNT p9x00regs.hardware_ctrl->ras_low_counter
#define P9X_R_PUCFG    p9x00regs.hardware_ctrl->power_up_config

#define P9X_R_DAC(x)    p9x00regs.dac[x]

#define P9X_R_COPRO(x)  p9x00regs.copro[x]

#define P9X_R_STATUS    p9x00regs.command->status
#define P9X_R_BLIT      p9x00regs.command->blit
#define P9X_R_QUAD      p9x00regs.command->quad
#define P9X_R_PIXEL8    p9x00regs.command->pixel8
#define P9X_R_NPIXEL    p9x00regs.command->next_pixel
#define P9X_R_PIXEL1(x) (((CARD32 *)(&p9x00regs.command->pixel1))[x])

#define P9X_R_OOR     p9x00regs.parameter_ctrl->out_of_range
#define P9X_R_CINDEX  p9x00regs.parameter_ctrl->coord_index
#define P9X_R_WOFFSET p9x00regs.parameter_ctrl->window_offset
#define P9X_R_WMINR   p9x00regs.parameter_ctrl->win_min_read
#define P9X_R_WMAXR   p9x00regs.parameter_ctrl->win_max_read
#define P9X_R_XCLIP   p9x00regs.parameter_ctrl->x_clipping
#define P9X_R_YCLIP   p9x00regs.parameter_ctrl->y_clipping
#define P9X_R_XEDGEL  p9x00regs.parameter_ctrl->x_edge_less
#define P9X_R_YEDGEL  p9x00regs.parameter_ctrl->y_edge_less
#define P9X_R_XEDGEG  p9x00regs.parameter_ctrl->x_edge_greater
#define P9X_R_YEDGEG  p9x00regs.parameter_ctrl->y_edge_greater

#define P9X_R_COLOR0     p9x00regs.drawing_ctrl->color0
#define P9X_R_COLOR1     p9x00regs.drawing_ctrl->color1
#define P9X_R_PMASK      p9x00regs.drawing_ctrl->plane_mask
#define P9X_R_MODE       p9x00regs.drawing_ctrl->drawing_mode
#define P9X_R_PORIGX     p9x00regs.drawing_ctrl->pattern_origin_x
#define P9X_R_PORIGY     p9x00regs.drawing_ctrl->pattern_origin_y
#define P9X_R_RASTER     p9x00regs.drawing_ctrl->raster
#define P9X_R_LPIXEL8    p9x00regs.drawing_ctrl->pixel8
#define P9X_R_WMIN       p9x00regs.drawing_ctrl->window_min
#define P9X_R_WMAX       p9x00regs.drawing_ctrl->window_max
#define P9X_R_COLOR2     p9x00regs.drawing_ctrl->color2
#define P9X_R_COLOR3     p9x00regs.drawing_ctrl->color3
#define P9X_R_PATTERN0   p9x00regs.drawing_ctrl->pattern0
#define P9X_R_PATTERN1   p9x00regs.drawing_ctrl->pattern1
#define P9X_R_PATTERN2   p9x00regs.drawing_ctrl->pattern2
#define P9X_R_PATTERN3   p9x00regs.drawing_ctrl->pattern3
#define P9X_R_PATTERN4   p9x00regs.drawing_ctrl->patt4soft0
#define P9X_R_PATTERN5   p9x00regs.drawing_ctrl->patt5soft1
#define P9X_R_PATTERN6   p9x00regs.drawing_ctrl->patt6soft2
#define P9X_R_PATTERN7   p9x00regs.drawing_ctrl->patt7soft3
#define P9X_R_WMINB      p9x00regs.drawing_ctrl->byte_win_min
#define P9X_R_WMAXB      p9x00regs.drawing_ctrl->byte_win_max

#define P9X_R_EDGE0_ABS_SCR_XY p9x00regs.device_coord->edge0.abs_to_scr.xy
#define P9X_R_EDGE0_ABS_SCR_X  p9x00regs.device_coord->edge0.abs_to_scr.x
#define P9X_R_EDGE0_ABS_SCR_Y  p9x00regs.device_coord->edge0.abs_to_scr.y
#define P9X_R_EDGE0_ABS_WIN_XY p9x00regs.device_coord->edge0.abs_to_win.xy
#define P9X_R_EDGE0_ABS_WIN_X  p9x00regs.device_coord->edge0.abs_to_win.x
#define P9X_R_EDGE0_ABS_WIN_Y  p9x00regs.device_coord->edge0.abs_to_win.y
#define P9X_R_EDGE1_ABS_SCR_XY p9x00regs.device_coord->edge1.abs_to_scr.xy
#define P9X_R_EDGE1_ABS_SCR_X  p9x00regs.device_coord->edge1.abs_to_scr.x
#define P9X_R_EDGE1_ABS_SCR_Y  p9x00regs.device_coord->edge1.abs_to_scr.y
#define P9X_R_EDGE1_ABS_WIN_XY p9x00regs.device_coord->edge1.abs_to_win.xy
#define P9X_R_EDGE1_ABS_WIN_X  p9x00regs.device_coord->edge1.abs_to_win.x
#define P9X_R_EDGE1_ABS_WIN_Y  p9x00regs.device_coord->edge1.abs_to_win.y
#define P9X_R_EDGE2_ABS_SCR_XY p9x00regs.device_coord->edge2.abs_to_scr.xy
#define P9X_R_EDGE2_ABS_SCR_X  p9x00regs.device_coord->edge2.abs_to_scr.x
#define P9X_R_EDGE2_ABS_SCR_Y  p9x00regs.device_coord->edge2.abs_to_scr.y
#define P9X_R_EDGE2_ABS_WIN_XY p9x00regs.device_coord->edge2.abs_to_win.xy
#define P9X_R_EDGE2_ABS_WIN_X  p9x00regs.device_coord->edge2.abs_to_win.x
#define P9X_R_EDGE2_ABS_WIN_Y  p9x00regs.device_coord->edge2.abs_to_win.y
#define P9X_R_EDGE3_ABS_SCR_XY p9x00regs.device_coord->edge3.abs_to_scr.xy
#define P9X_R_EDGE3_ABS_SCR_X  p9x00regs.device_coord->edge3.abs_to_scr.x
#define P9X_R_EDGE3_ABS_SCR_Y  p9x00regs.device_coord->edge3.abs_to_scr.y
#define P9X_R_EDGE3_ABS_WIN_XY p9x00regs.device_coord->edge3.abs_to_win.xy
#define P9X_R_EDGE3_ABS_WIN_X  p9x00regs.device_coord->edge3.abs_to_win.x
#define P9X_R_EDGE3_ABS_WIN_Y  p9x00regs.device_coord->edge3.abs_to_win.y

#define P9X_R_POINT_WIN_XY p9x00regs.meta_coord->point.rel_to_win.xy
#define P9X_R_POINT_WIN_X  p9x00regs.meta_coord->point.rel_to_win.x
#define P9X_R_POINT_WIN_Y  p9x00regs.meta_coord->point.rel_to_win.y

#define P9X_R_POINT_PRE_XY p9x00regs.meta_coord->point.rel_to_prev.xy
#define P9X_R_POINT_PRE_X  p9x00regs.meta_coord->point.rel_to_prev.x
#define P9X_R_POINT_PRE_Y  p9x00regs.meta_coord->point.rel_to_prev.y

#define P9X_R_LINE_WIN_XY p9x00regs.meta_coord->line.rel_to_win.xy
#define P9X_R_LINE_WIN_X  p9x00regs.meta_coord->line.rel_to_win.x
#define P9X_R_LINE_WIN_Y  p9x00regs.meta_coord->line.rel_to_win.y

#define P9X_R_LINE_PRE_XY p9x00regs.meta_coord->line.rel_to_prev.xy
#define P9X_R_LINE_PRE_X  p9x00regs.meta_coord->line.rel_to_prev.x
#define P9X_R_LINE_PRE_Y  p9x00regs.meta_coord->line.rel_to_prev.y

#define P9X_R_TRI_WIN_XY p9x00regs.meta_coord->tri.rel_to_win.xy
#define P9X_R_TRI_WIN_X  p9x00regs.meta_coord->tri.rel_to_win.x
#define P9X_R_TRI_WIN_Y  p9x00regs.meta_coord->tri.rel_to_win.y

#define P9X_R_TRI_PRE_XY p9x00regs.meta_coord->tri.rel_to_prev.xy
#define P9X_R_TRI_PRE_X  p9x00regs.meta_coord->tri.rel_to_prev.x
#define P9X_R_TRI_PRE_Y  p9x00regs.meta_coord->tri.rel_to_prev.y

#define P9X_R_QUAD_WIN_XY p9x00regs.meta_coord->quad.rel_to_win.xy
#define P9X_R_QUAD_WIN_X  p9x00regs.meta_coord->quad.rel_to_win.x
#define P9X_R_QUAD_WIN_Y  p9x00regs.meta_coord->quad.rel_to_win.y

#define P9X_R_QUAD_PRE_XY p9x00regs.meta_coord->quad.rel_to_prev.xy
#define P9X_R_QUAD_PRE_X  p9x00regs.meta_coord->quad.rel_to_prev.x
#define P9X_R_QUAD_PRE_Y  p9x00regs.meta_coord->quad.rel_to_prev.y

#define P9X_R_RECT_WIN_XY p9x00regs.meta_coord->rect.rel_to_win.xy
#define P9X_R_RECT_WIN_X  p9x00regs.meta_coord->rect.rel_to_win.x
#define P9X_R_RECT_WIN_Y  p9x00regs.meta_coord->rect.rel_to_win.y

#define P9X_R_RECT_PRE_XY p9x00regs.meta_coord->rect.rel_to_prev.xy
#define P9X_R_RECT_PRE_X  p9x00regs.meta_coord->rect.rel_to_prev.x
#define P9X_R_RECT_PRE_Y  p9x00regs.meta_coord->rect.rel_to_prev.y

#define P9X_R_VRAM p9x00regs.vram

#endif /* P9X00REGS_H */
