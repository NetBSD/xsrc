 /***************************************************************************\
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.  Users and possessors of this source code are     *|
|*     hereby granted a nonexclusive,  royalty-free copyright license to     *|
|*     use this code in individual and commercial software.                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*       Copyright 1993-1998 NVIDIA, Corporation.  All rights reserved.      *|
|*                                                                           *|
|*     NVIDIA, CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY     *|
|*     OF  THIS SOURCE  CODE  FOR ANY PURPOSE.  IT IS  PROVIDED  "AS IS"     *|
|*     WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORPOR-     *|
|*     ATION DISCLAIMS ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,     *|
|*     INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGE-     *|
|*     MENT,  AND FITNESS  FOR A PARTICULAR PURPOSE.   IN NO EVENT SHALL     *|
|*     NVIDIA, CORPORATION  BE LIABLE FOR ANY SPECIAL,  INDIRECT,  INCI-     *|
|*     DENTAL, OR CONSEQUENTIAL DAMAGES,  OR ANY DAMAGES  WHATSOEVER RE-     *|
|*     SULTING FROM LOSS OF USE,  DATA OR PROFITS,  WHETHER IN AN ACTION     *|
|*     OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  ARISING OUT OF     *|
|*     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOURCE CODE.     *|
|*                                                                           *|
|*     U.S. Government  End  Users.   This source code  is a "commercial     *|
|*     item,"  as that  term is  defined at  48 C.F.R. 2.101 (OCT 1995),     *|
|*     consisting  of "commercial  computer  software"  and  "commercial     *|
|*     computer  software  documentation,"  as such  terms  are  used in     *|
|*     48 C.F.R. 12.212 (SEPT 1995)  and is provided to the U.S. Govern-     *|
|*     ment only as  a commercial end item.   Consistent with  48 C.F.R.     *|
|*     12.212 and  48 C.F.R. 227.7202-1 through  227.7202-4 (JUNE 1995),     *|
|*     all U.S. Government End Users  acquire the source code  with only     *|
|*     those rights set forth herein.                                        *|
|*                                                                           *|
 \***************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv3arb.c,v 1.1.2.3 1998/11/18 16:38:44 hohndel Exp $ */
#include "nv3arb.h"
#include "nvreg.h"
static int iterate(fifo_info *res_info, sim_state * state, arb_info *ainfo)
{
    int iter = 0;
    int tmp, t;
    int vfsize, mfsize, gfsize;
    int mburst_size = 32;
    int mmisses, gmisses, vmisses;
    int misses;
    int vlwm, glwm, mlwm;
    int last, next, cur;
    int max_gfsize ;
    long ns;

    vlwm = 0;
    glwm = 0;
    mlwm = 0;
    vfsize = 0;
    gfsize = 0;
    cur = ainfo->cur;
    mmisses = 2;
    gmisses = 2;
    vmisses = 2;
    if (ainfo->gburst_size == 128) max_gfsize = 256 ;
    else  max_gfsize = 320 ;
    max_gfsize = 320 ;
    while (1)
    {
        if (ainfo->vid_en)
        {
            if (ainfo->wcvocc > ainfo->vocc) ainfo->wcvocc = ainfo->vocc;
            if (ainfo->wcvlwm > vlwm) ainfo->wcvlwm = vlwm ;
            ns = 1000000 * ainfo->vburst_size/(state->memory_width/8)/state->mclk_khz;
            vfsize = ns * ainfo->vdrain_rate / 1000000;
            vfsize =  ainfo->wcvlwm - ainfo->vburst_size + vfsize;
        }
        if (state->enable_mp)
        {
            if (ainfo->wcmocc > ainfo->mocc) ainfo->wcmocc = ainfo->mocc;
        }
        if (ainfo->gr_en)
        {
            if (ainfo->wcglwm > glwm) ainfo->wcglwm = glwm ;
            if (ainfo->wcgocc > ainfo->gocc) ainfo->wcgocc = ainfo->gocc;
            ns = 1000000 * (ainfo->gburst_size/(state->memory_width/8))/state->mclk_khz;
            gfsize = ns *ainfo->gdrain_rate/1000000;
            gfsize = ainfo->wcglwm - ainfo->gburst_size + gfsize;
        }
        mfsize = 0;
        if (!state->gr_during_vid && ainfo->vid_en)
            if (ainfo->vid_en && (ainfo->vocc < 0) && !ainfo->vid_only_once)
                next = 0 ;
            else if (ainfo->mocc < 0)
                next = 2 ;
            else if (ainfo->gocc< ainfo->by_gfacc)
                next = 1 ;
            else return (0);
        else switch (ainfo->priority)
            {
                case 0 :
                    if (ainfo->vid_en && ainfo->vocc<0 && !ainfo->vid_only_once)
                        next = 0 ;
                    else if (ainfo->gr_en && ainfo->gocc<0 && !ainfo->gr_only_once)
                        next = 1 ;
                    else if (ainfo->mocc<0)
                        next = 2 ;
                    else    return (0);
                    break;
                case 1 :
                    if (ainfo->gr_en && ainfo->gocc<0 && !ainfo->gr_only_once)
                        next = 1 ;
                    else if (ainfo->vid_en && ainfo->vocc<0 && !ainfo->vid_only_once)
                        next = 0 ;
                    else if (ainfo->mocc<0)
                        next = 2 ;
                    else    return (0);
                    break;
                default:
                    if (ainfo->mocc<0)
                        next = 2 ;
                    else if (ainfo->gr_en && ainfo->gocc<0 && !ainfo->gr_only_once)
                        next = 1 ;
                    else if (ainfo->vid_en && ainfo->vocc<0 && !ainfo->vid_only_once)
                        next = 0 ;
                    else    return (0);
                    break;
            }
        last = cur;
        cur = next;
        iter++;
        switch (cur)
        {
            case 0 :
                if (last==cur)    misses = 0;
                else if (ainfo->first_vacc)   misses = vmisses;
                else    misses = 1;
                ainfo->first_vacc = 0;
                if (last!=cur)
                {
                    ns =  1000000 * (vmisses*state->mem_page_miss + state->mem_latency)/state->mclk_khz; 
                    vlwm = ns * ainfo->vdrain_rate/ 1000000;
                    vlwm = ainfo->vocc - vlwm;
                }
                ns = 1000000*(misses*state->mem_page_miss + ainfo->vburst_size)/(state->memory_width/8)/state->mclk_khz;
                ainfo->vocc = ainfo->vocc + ainfo->vburst_size - ns*ainfo->vdrain_rate/1000000;
                ainfo->gocc = ainfo->gocc - ns*ainfo->gdrain_rate/1000000;
                ainfo->mocc = ainfo->mocc - ns*ainfo->mdrain_rate/1000000;
                break;
            case 1 :
                if (last==cur)    misses = 0;
                else if (ainfo->first_gacc)   misses = gmisses;
                else    misses = 1;
                ainfo->first_gacc = 0;
                if (last!=cur)
                {
                    ns = 1000000*(gmisses*state->mem_page_miss + state->mem_latency)/state->mclk_khz ;
                    glwm = ns * ainfo->gdrain_rate/1000000;
                    glwm = ainfo->gocc - glwm;
                }
                ns = 1000000*(misses*state->mem_page_miss + ainfo->gburst_size/(state->memory_width/8))/state->mclk_khz;
                ainfo->vocc = ainfo->vocc + 0 - ns*ainfo->vdrain_rate/1000000;
                ainfo->gocc = ainfo->gocc + ainfo->gburst_size - ns*ainfo->gdrain_rate/1000000;
                ainfo->mocc = ainfo->mocc + 0 - ns*ainfo->mdrain_rate/1000000;
                break;
            default:
                if (last==cur)    misses = 0;
                else if (ainfo->first_macc)   misses = mmisses;
                else    misses = 1;
                ainfo->first_macc = 0;
                ns = 1000000*(misses*state->mem_page_miss + mburst_size/(state->memory_width/8))/state->mclk_khz;
                ainfo->vocc = ainfo->vocc + 0 - ns*ainfo->vdrain_rate/1000000;
                ainfo->gocc = ainfo->gocc + 0 - ns*ainfo->gdrain_rate/1000000;
                ainfo->mocc = ainfo->mocc + mburst_size - ns*ainfo->mdrain_rate/1000000;
                break;
        }
        if (iter>100)
        {
            ainfo->converged = 0;
            return (1);
        }
        ns = 1000000*ainfo->gburst_size/(state->memory_width/8)/state->mclk_khz;
        tmp = ns * ainfo->gdrain_rate/1000000;
        if (( ainfo->gburst_size >0? ainfo->gburst_size :- ainfo->gburst_size )  + ((( ainfo->wcglwm >0? ainfo->wcglwm :- ainfo->wcglwm )  + 16 ) & ~0x7)    - tmp > max_gfsize)
        {
            ainfo->converged = 0;
            return (1);
        }
        ns = 1000000*ainfo->vburst_size/(state->memory_width/8)/state->mclk_khz;
        tmp = ns * ainfo->vdrain_rate/1000000;
        if (( ainfo->vburst_size >0? ainfo->vburst_size :- ainfo->vburst_size )  + (( ainfo->wcvlwm + 32 >0? ainfo->wcvlwm + 32 :- ainfo->wcvlwm + 32 )  & ~0xf)  - tmp> 256 )
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( ainfo->gocc >0? ainfo->gocc :- ainfo->gocc )  > max_gfsize)
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( ainfo->vocc >0? ainfo->vocc :- ainfo->vocc )  > 256 )
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( ainfo->mocc >0? ainfo->mocc :- ainfo->mocc )  > 120 )
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( vfsize >0? vfsize :- vfsize )  > 256 )
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( gfsize >0? gfsize :- gfsize )  > max_gfsize)
        {
            ainfo->converged = 0;
            return (1);
        }
        if (( mfsize >0? mfsize :- mfsize )  > 120 )
        {
            ainfo->converged = 0;
            return (1);
        }
    }
}
static char arb(fifo_info * res_info, sim_state * state,  arb_info *ainfo) 
{
    int  g, v, not_done;
    long ens, vns, mns, gns;
    int mmisses, gmisses, vmisses, eburst_size, mburst_size;
    int refresh_cycle;

    refresh_cycle = 0;
    refresh_cycle = 2*(state->mclk_khz/state->pclk_khz) + 5;
    mmisses = 2;
    if (state->mem_aligned) gmisses = 2;
    else    gmisses = 3;
    vmisses = 2;
    eburst_size = state->memory_width * 1;
    mburst_size = 32;
    gns = 1000000 * (gmisses*state->mem_page_miss + state->mem_latency)/state->mclk_khz;
    ainfo -> by_gfacc = gns*ainfo->gdrain_rate/1000000;
    ainfo -> wcmocc = 0;
    ainfo -> wcgocc = 0;
    ainfo -> wcvocc = 0;
    ainfo -> wcvlwm = 0;
    ainfo -> wcglwm = 0;
    ainfo -> engine_en = 1;
    ainfo -> converged = 1;
    if (ainfo->engine_en)
    {
        ens =  1000000*(state->mem_page_miss + eburst_size/(state->memory_width/8) +refresh_cycle)/state->mclk_khz;
        ainfo -> mocc = state->enable_mp ? 0-ens*ainfo->mdrain_rate/1000000 : 0;
        ainfo -> vocc = ainfo->vid_en ? 0-ens*ainfo->vdrain_rate/1000000 : 0;
        ainfo -> gocc = ainfo->gr_en ? 0-ens*ainfo->gdrain_rate/1000000 : 0;
        ainfo -> cur = 3 ;
        ainfo -> first_vacc = 1;
        ainfo -> first_gacc = 1;
        ainfo -> first_macc = 1;
        iterate(res_info, state,ainfo);
    }
    if (state->enable_mp)
    {
        mns = 1000000 * (mmisses*state->mem_page_miss + mburst_size/(state->memory_width/8) + refresh_cycle)/state->mclk_khz;
        ainfo -> mocc = state->enable_mp ? 0 : mburst_size - mns*ainfo->mdrain_rate/1000000;
        ainfo -> vocc = ainfo->vid_en ? 0 : 0- mns*ainfo->vdrain_rate/1000000;
        ainfo -> gocc = ainfo->gr_en ? 0: 0- mns*ainfo->gdrain_rate/1000000;
        ainfo -> cur = 2 ;
        ainfo -> first_vacc = 1;
        ainfo -> first_gacc = 1;
        ainfo -> first_macc = 0;
        iterate(res_info, state,ainfo);
    }
    if (ainfo->gr_en)
    {
        ainfo->first_vacc = 1;
        ainfo->first_gacc = 0;
        ainfo->first_macc = 1;
        gns = 1000000*(gmisses*state->mem_page_miss + ainfo->gburst_size/(state->memory_width/8) + refresh_cycle)/state->mclk_khz;
        ainfo->gocc = ainfo->gburst_size - gns*ainfo->gdrain_rate/1000000;
        ainfo->vocc = ainfo->vid_en? 0-gns*ainfo->vdrain_rate/1000000 : 0;
        ainfo->mocc = state->enable_mp ?  0-gns*ainfo->mdrain_rate/1000000: 0;
        ainfo->cur = 1 ;
        iterate(res_info, state,ainfo);
    }
    if (ainfo->vid_en)
    {
        ainfo->first_vacc = 0;
        ainfo->first_gacc = 1;
        ainfo->first_macc = 1;
        vns = 1000000*(vmisses*state->mem_page_miss + ainfo->vburst_size/(state->memory_width/8) + refresh_cycle)/state->mclk_khz;
        ainfo->vocc = ainfo->vburst_size - vns*ainfo->vdrain_rate/1000000;
        ainfo->gocc = ainfo->gr_en? (0-vns*ainfo->gdrain_rate/1000000) : 0;
        ainfo->mocc = state->enable_mp? 0-vns*ainfo->mdrain_rate/1000000 :0 ;
        ainfo->cur = 0 ;
        iterate(res_info, state, ainfo);
    }
    if (ainfo->converged)
    {
        res_info->graphics_lwm = (int)( ainfo->wcglwm >0? ainfo->wcglwm :- ainfo->wcglwm )  + 16;
        res_info->video_lwm = (int)( ainfo->wcvlwm >0? ainfo->wcvlwm :- ainfo->wcvlwm )  + 32;
        res_info->graphics_burst_size = ainfo->gburst_size;
        res_info->video_burst_size = ainfo->vburst_size;
        res_info->graphics_hi_priority = (ainfo->priority == 1 );
        res_info->media_hi_priority = (ainfo->priority == 2 );
        if (res_info->video_lwm > 160)
        {
            res_info->graphics_lwm = 256;
            res_info->video_lwm = 128;
            res_info->graphics_burst_size = 64;
            res_info->video_burst_size = 64;
            res_info->graphics_hi_priority = 0;
            res_info->media_hi_priority = 0;
            ainfo->converged = 0;
            return (0);
        }
        if (res_info->video_lwm > 128)
        {
            res_info->video_lwm = 128;
        }
        return (1);
    }
    else
    {
        res_info->graphics_lwm = 256;
        res_info->video_lwm = 128;
        res_info->graphics_burst_size = 64;
        res_info->video_burst_size = 64;
        res_info->graphics_hi_priority = 0;
        res_info->media_hi_priority = 0;
        return (0);
    }
}
static char get_param(fifo_info *res_info, sim_state * state, arb_info *ainfo)
{
    int done, g,v, p;
    int priority, gburst_size, vburst_size, iter;
    
    done = 0;
    if (state->gr_during_vid && ainfo->vid_en)
        ainfo->priority = 2 ;
    else
        ainfo->priority = ainfo->gdrain_rate < ainfo->vdrain_rate ? 0 : 1 ;
    for (p=0; p < 2 && done != 1; p++)
    {
        for (g=128 ; (g > 32) && (done != 1); g= g>> 1)
        {
            for (v=128; (v >=32) && (done !=1); v = v>> 1)
            {
                ainfo->priority = p;
                ainfo->gburst_size = g;     
                ainfo->vburst_size = v;
                done = arb(res_info, state,ainfo);
                if (g==128)
                {
                    if ((res_info->graphics_lwm + g) > 256)
                        done = 0;
                }
            }
        }
    }
    if (!done)
        return (0);
    else
        return (1);
}
static void CalcArbitration 
(
    fifo_info * res_info,
    sim_state * state
)
{
    fifo_info save_info;
    arb_info ainfo;
    char   res_gr, res_vid;

    ainfo.gr_en = 1;
    ainfo.vid_en = state->enable_video;
    ainfo.vid_only_once = 0;
    ainfo.gr_only_once = 0;
    ainfo.gdrain_rate = (int) state->pclk_khz * state -> pix_bpp/8;
    ainfo.vdrain_rate = (int) state->pclk_khz * 2;
    if (state->video_scale != 0)
        ainfo.vdrain_rate = ainfo.vdrain_rate/state->video_scale;
    ainfo.mdrain_rate = 33000;
    res_info->rtl_values = 0;
    if (!state->gr_during_vid && state->enable_video)
    {
        ainfo.gr_only_once = 1;
        ainfo.gr_en = 1;
        ainfo.gdrain_rate = 0;
        res_vid = get_param(res_info, state,  &ainfo);
        res_vid = ainfo.converged;
        save_info.video_lwm = res_info->video_lwm;
        save_info.video_burst_size = res_info->video_burst_size;
        ainfo.vid_en = 1;
        ainfo.vid_only_once = 1;
        ainfo.gr_en = 1;
        ainfo.gdrain_rate = (int) state->pclk_khz * state -> pix_bpp/8;
        ainfo.vdrain_rate = 0;
        res_gr = get_param(res_info, state,  &ainfo);
        res_gr = ainfo.converged;
        res_info->video_lwm = save_info.video_lwm;
        res_info->video_burst_size = save_info.video_burst_size;
        res_info->valid = res_gr & res_vid;
    }
    else
    {
        if (!ainfo.gr_en) ainfo.gdrain_rate = 0;
        if (!ainfo.vid_en) ainfo.vdrain_rate = 0;
        res_gr = get_param(res_info, state,  &ainfo);
        res_info->valid = ainfo.converged;
    }
}
void nv3UpdateArbitrationSettings
(
    unsigned int   VClk, 
    unsigned int   pixelDepth, 
    unsigned int   crystal,
    unsigned char *lwm,
    unsigned char *burst
)
{
    fifo_info fifo_data;
    sim_state sim_data;
    unsigned int M, N, P, pll, MClk, cfg1;
    
    pll = nvPRAMDACPort[((0x00680504    )- (0?0x00680FFF:0x00680000    ) )/4]   ;
    M = (pll >> 0)  & 0xFF;
    N = (pll >> 8)  & 0xFF;
    P = (pll >> 16) & 0x0F;
    MClk  = (N * crystal / M) >> P;
    cfg1 = nvPFBPort[((0x00100204    )- (0?0x00100FFF:0x00100000    ) )/4]   ;
    sim_data.pix_bpp        = (char)pixelDepth;
    sim_data.enable_video   = 0;
    sim_data.enable_mp      = 0;
    sim_data.video_scale    = 1;
    sim_data.memory_width   = (nvPEXTDEVPort[((0x00101000    )- (0?0x00101FFF:0x00101000    ) )/4]    & 0x10) ? 128 : 64;
    sim_data.mem_latency    = 11 ;
    sim_data.mem_aligned    = 1;
    sim_data.mem_page_miss  = 9 ;
    sim_data.gr_during_vid  = 0;
    sim_data.pclk_khz       = VClk;
    sim_data.mclk_khz       = MClk;
    CalcArbitration(&fifo_data, &sim_data);
    if (fifo_data.valid)
    {
        *lwm = fifo_data.graphics_lwm >> 3;
        switch (fifo_data.graphics_burst_size)
        {
            case 256:
                *burst = 4;
                 
                break;
            case 128:
                *burst = 3;
                break;
            case 64:
                *burst = 2;
                break;
            case 32:
                *burst = 1;
                break;
            case 16:
                *burst = 0;
                break;
        }
    }
    else
    {
        *lwm   = 0x24;
        *burst = 0x02;
    }
}

