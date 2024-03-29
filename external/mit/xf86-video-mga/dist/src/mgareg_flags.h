/* author: stephen crowley, crow@debian.org */

/*
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * STEPHEN CROWLEY, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _MGAREGS_H_
#define _MGAREGS_H_

/*************** (START) AUTOMATICALLY GENERATED REGISTER FILE ***************/
/*
 * Generated on Sat Nov 20 21:25:36 CST 1999
 */



/*
 * Power Graphic Mode Memory Space Registers
 */

#    define AGP_PLL_agp2xpllen_MASK 	0xfffffffe 	/* bit 0 */
#    define AGP_PLL_agp2xpllen_disable 	0x0 		
#    define AGP_PLL_agp2xpllen_enable 	0x1 		

#    define AC_src_MASK 		0xfffffff0 	/* bits 0-3 */
#    define AC_src_zero 		0x0 		/* val 0, shift 0 */
#    define AC_src_one 			0x1 		/* val 1, shift 0 */
#    define AC_src_dst_color 		0x2 		/* val 2, shift 0 */
#    define AC_src_om_dst_color 	0x3 		/* val 3, shift 0 */
#    define AC_src_src_alpha 		0x4 		/* val 4, shift 0 */
#    define AC_src_om_src_alpha 	0x5 		/* val 5, shift 0 */
#    define AC_src_dst_alpha 		0x6 		/* val 6, shift 0 */
#    define AC_src_om_dst_alpha 	0x7 		/* val 7, shift 0 */
#    define AC_src_src_alpha_sat 	0x8 		/* val 8, shift 0 */
#    define AC_dst_MASK 		0xffffff0f 	/* bits 4-7 */
#    define AC_dst_zero 		0x0 		/* val 0, shift 4 */
#    define AC_dst_one 			0x10 		/* val 1, shift 4 */
#    define AC_dst_src_color 		0x20 		/* val 2, shift 4 */
#    define AC_dst_om_src_color 	0x30 		/* val 3, shift 4 */
#    define AC_dst_src_alpha 		0x40 		/* val 4, shift 4 */
#    define AC_dst_om_src_alpha 	0x50 		/* val 5, shift 4 */
#    define AC_dst_dst_alpha 		0x60 		/* val 6, shift 4 */
#    define AC_dst_om_dst_alpha 	0x70 		/* val 7, shift 4 */
#    define AC_amode_MASK 		0xfffffcff 	/* bits 8-9 */
#    define AC_amode_FCOL 		0x0 		/* val 0, shift 8 */
#    define AC_amode_alpha_channel 	0x100 		/* val 1, shift 8 */
#    define AC_amode_video_alpha 	0x200 		/* val 2, shift 8 */
#    define AC_amode_RSVD 		0x300 		/* val 3, shift 8 */
#    define AC_astipple_MASK 		0xfffff7ff 	/* bit 11 */
#    define AC_astipple_disable 	0x0 		
#    define AC_astipple_enable 		0x800 		
#    define AC_aten_MASK 		0xffffefff 	/* bit 12 */
#    define AC_aten_disable 		0x0 		
#    define AC_aten_enable 		0x1000 		
#    define AC_atmode_MASK 		0xffff1fff 	/* bits 13-15 */
#    define AC_atmode_noacmp 		0x0 		/* val 0, shift 13 */
#    define AC_atmode_ae 		0x4000 		/* val 2, shift 13 */
#    define AC_atmode_ane 		0x6000 		/* val 3, shift 13 */
#    define AC_atmode_alt 		0x8000 		/* val 4, shift 13 */
#    define AC_atmode_alte 		0xa000 		/* val 5, shift 13 */
#    define AC_atmode_agt 		0xc000 		/* val 6, shift 13 */
#    define AC_atmode_agte 		0xe000 		/* val 7, shift 13 */
#    define AC_atref_MASK 		0xff00ffff 	/* bits 16-23 */
#    define AC_atref_SHIFT 		16 		
#    define AC_alphasel_MASK 		0xfcffffff 	/* bits 24-25 */
#    define AC_alphasel_fromtex 	0x0 		/* val 0, shift 24 */
#    define AC_alphasel_diffused 	0x1000000 	/* val 1, shift 24 */
#    define AC_alphasel_modulated 	0x2000000 	/* val 2, shift 24 */
#    define AC_alphasel_trans 		0x3000000 	/* val 3, shift 24 */

#    define AR0_ar0_MASK 		0xfffc0000 	/* bits 0-17 */
#    define AR0_ar0_SHIFT 		0 		

#    define AR1_ar1_MASK 		0xff000000 	/* bits 0-23 */
#    define AR1_ar1_SHIFT 		0 		

#    define AR2_ar2_MASK 		0xfffc0000 	/* bits 0-17 */
#    define AR2_ar2_SHIFT 		0 		

#    define AR3_ar3_MASK 		0xff000000 	/* bits 0-23 */
#    define AR3_ar3_SHIFT 		0 		
#    define AR3_spage_MASK 		0xf8ffffff 	/* bits 24-26 */
#    define AR3_spage_SHIFT 		24 		

#    define AR4_ar4_MASK 		0xfffc0000 	/* bits 0-17 */
#    define AR4_ar4_SHIFT 		0 		

#    define AR5_ar5_MASK 		0xfffc0000 	/* bits 0-17 */
#    define AR5_ar5_SHIFT 		0 		

#    define AR6_ar6_MASK 		0xfffc0000 	/* bits 0-17 */
#    define AR6_ar6_SHIFT 		0 		

#    define BC_besen_MASK 		0xfffffffe 	/* bit 0 */
#    define BC_besen_disable 		0x0 		
#    define BC_besen_enable 		0x1 		
#    define BC_besv1srcstp_MASK 	0xffffffbf 	/* bit 6 */
#    define BC_besv1srcstp_even 	0x0 		
#    define BC_besv1srcstp_odd 		0x40 		
#    define BC_besv2srcstp_MASK 	0xfffffeff 	/* bit 8 */
#    define BC_besv2srcstp_disable 	0x0 		
#    define BC_besv2srcstp_enable 	0x100 		
#    define BC_beshfen_MASK 		0xfffffbff 	/* bit 10 */
#    define BC_beshfen_disable 		0x0 		
#    define BC_beshfen_enable 		0x400 		
#    define BC_besvfen_MASK 		0xfffff7ff 	/* bit 11 */
#    define BC_besvfen_disable 		0x0 		
#    define BC_besvfen_enable 		0x800 		
#    define BC_beshfixc_MASK 		0xffffefff 	/* bit 12 */
#    define BC_beshfixc_weight 		0x0 		
#    define BC_beshfixc_coeff 		0x1000 		
#    define BC_bescups_MASK 		0xfffeffff 	/* bit 16 */
#    define BC_bescups_disable 		0x0 		
#    define BC_bescups_enable 		0x10000 	
#    define BC_bes420pl_MASK 		0xfffdffff 	/* bit 17 */
#    define BC_bes420pl_422 		0x0 		
#    define BC_bes420pl_420 		0x20000 	
#    define BC_besdith_MASK 		0xfffbffff 	/* bit 18 */
#    define BC_besdith_disable 		0x0 		
#    define BC_besdith_enable 		0x40000 	
#    define BC_beshmir_MASK 		0xfff7ffff 	/* bit 19 */
#    define BC_beshmir_disable 		0x0 		
#    define BC_beshmir_enable 		0x80000 	
#    define BC_besbwen_MASK 		0xffefffff 	/* bit 20 */
#    define BC_besbwen_color 		0x0 		
#    define BC_besbwen_bw 		0x100000 	
#    define BC_besblank_MASK 		0xffdfffff 	/* bit 21 */
#    define BC_besblank_disable 	0x0 		
#    define BC_besblank_enable 		0x200000 	
#    define BC_besfselm_MASK 		0xfeffffff 	/* bit 24 */
#    define BC_besfselm_soft 		0x0 		
#    define BC_besfselm_hard 		0x1000000 	
#    define BC_besfsel_MASK 		0xf9ffffff 	/* bits 25-26 */
#    define BC_besfsel_a1 		0x0 		/* val 0, shift 25 */
#    define BC_besfsel_a2 		0x2000000 	/* val 1, shift 25 */
#    define BC_besfsel_b1 		0x4000000 	/* val 2, shift 25 */
#    define BC_besfsel_b2 		0x6000000 	/* val 3, shift 25 */

#    define BGC_beshzoom_MASK 		0xfffffffe 	/* bit 0 */
#    define BGC_beshzoom_disable 	0x0 		
#    define BGC_beshzoom_enable 	0x1 		
#    define BGC_beshzoomf_MASK 		0xfffffffd 	/* bit 1 */
#    define BGC_beshzoomf_disable 	0x0 		
#    define BGC_beshzoomf_enable 	0x2 		
#    define BGC_bescorder_MASK 		0xfffffff7 	/* bit 3 */
#    define BGC_bescorder_even 		0x0 		
#    define BGC_bescorder_odd 		0x8 		
#    define BGC_besreghup_MASK 		0xffffffef 	/* bit 4 */
#    define BGC_besreghup_disable 	0x0 		
#    define BGC_besreghup_enable 	0x10 		
#    define BGC_besvcnt_MASK 		0xf000ffff 	/* bits 16-27 */
#    define BGC_besvcnt_SHIFT 		16 		

#    define BHC_besright_MASK 		0xfffff800 	/* bits 0-10 */
#    define BHC_besright_SHIFT 		0 		
#    define BHC_besleft_MASK 		0xf800ffff 	/* bits 16-26 */
#    define BHC_besleft_SHIFT 		16 		

#    define BHISF_beshiscal_MASK 	0xffe00003 	/* bits 2-20 */
#    define BHISF_beshiscal_SHIFT 	2 		

#    define BHSE_beshsrcend_MASK 	0xfc000003 	/* bits 2-25 */
#    define BHSE_beshsrcend_SHIFT 	2 		

#    define BHSL_beshsrclst_MASK 	0xfc00ffff 	/* bits 16-25 */
#    define BHSL_beshsrclst_SHIFT 	16 		

#    define BHSS_beshsrcst_MASK 	0xfc000003 	/* bits 2-25 */
#    define BHSS_beshsrcst_SHIFT 	2 		

#    define BP_bespitch_MASK 		0xfffff000 	/* bits 0-11 */
#    define BP_bespitch_SHIFT 		0 		

#    define BS_besstat_MASK 		0xfffffffc 	/* bits 0-1 */
#    define BS_besstat_a1 		0x0 		/* val 0, shift 0 */
#    define BS_besstat_a2 		0x1 		/* val 1, shift 0 */
#    define BS_besstat_b1 		0x2 		/* val 2, shift 0 */
#    define BS_besstat_b2 		0x3 		/* val 3, shift 0 */

#    define BSF_besv1srclast_MASK 	0xfffffc00 	/* bits 0-9 */
#    define BSF_besv1srclast_SHIFT 	0 		

#    define BSF_besv2srclst_MASK 	0xfffffc00 	/* bits 0-9 */
#    define BSF_besv2srclst_SHIFT 	0 		

#    define BSF_besv1wght_MASK 		0xffff0003 	/* bits 2-15 */
#    define BSF_besv1wght_SHIFT 	2 		
#    define BSF_besv1wghts_MASK 	0xfffeffff 	/* bit 16 */
#    define BSF_besv1wghts_disable 	0x0 		
#    define BSF_besv1wghts_enable 	0x10000 	

#    define BSF_besv2wght_MASK 		0xffff0003 	/* bits 2-15 */
#    define BSF_besv2wght_SHIFT 	2 		
#    define BSF_besv2wghts_MASK 	0xfffeffff 	/* bit 16 */
#    define BSF_besv2wghts_disable 	0x0 		
#    define BSF_besv2wghts_enable 	0x10000 	

#    define BVC_besbot_MASK 		0xfffff800 	/* bits 0-10 */
#    define BVC_besbot_SHIFT 		0 		
#    define BVC_bestop_MASK 		0xf800ffff 	/* bits 16-26 */
#    define BVC_bestop_SHIFT 		16 		

#    define BVISF_besviscal_MASK 	0xffe00003 	/* bits 2-20 */
#    define BVISF_besviscal_SHIFT 	2 		

#    define CXB_cxleft_MASK 		0xfffff000 	/* bits 0-11 */
#    define CXB_cxleft_SHIFT 		0 		
#    define CXB_cxright_MASK 		0xf000ffff 	/* bits 16-27 */
#    define CXB_cxright_SHIFT 		16 		

#    define DO_dstmap_MASK 		0xfffffffe 	/* bit 0 */
#    define DO_dstmap_fb 		0x0 		
#    define DO_dstmap_sys 		0x1 		
#    define DO_dstacc_MASK 		0xfffffffd 	/* bit 1 */
#    define DO_dstacc_pci 		0x0 		
#    define DO_dstacc_agp 		0x2 		
#    define DO_dstorg_MASK 		0x7 		/* bits 3-31 */
#    define DO_dstorg_SHIFT 		3 		

#    define DC_opcod_MASK 		0xfffffff0 	/* bits 0-3 */
#    define DC_opcod_line_open 		0x0 		/* val 0, shift 0 */
#    define DC_opcod_autoline_open 	0x1 		/* val 1, shift 0 */
#    define DC_opcod_line_close 	0x2 		/* val 2, shift 0 */
#    define DC_opcod_autoline_close 	0x3 		/* val 3, shift 0 */
#    define DC_opcod_trap 		0x4 		/* val 4, shift 0 */
#    define DC_opcod_texture_trap 	0x6 		/* val 6, shift 0 */
#    define DC_opcod_bitblt 		0x8 		/* val 8, shift 0 */
#    define DC_opcod_iload 		0x9 		/* val 9, shift 0 */
#    define DC_atype_MASK 		0xffffff8f 	/* bits 4-6 */
#    define DC_atype_rpl 		0x0 		/* val 0, shift 4 */
#    define DC_atype_rstr 		0x10 		/* val 1, shift 4 */
#    define DC_atype_zi 		0x30 		/* val 3, shift 4 */
#    define DC_atype_blk 		0x40 		/* val 4, shift 4 */
#    define DC_atype_i 			0x70 		/* val 7, shift 4 */
#    define DC_linear_MASK 		0xffffff7f 	/* bit 7 */
#    define DC_linear_xy 		0x0 		
#    define DC_linear_linear 		0x80 		
#    define DC_zmode_MASK 		0xfffff8ff 	/* bits 8-10 */
#    define DC_zmode_nozcmp 		0x0 		/* val 0, shift 8 */
#    define DC_zmode_ze 		0x200 		/* val 2, shift 8 */
#    define DC_zmode_zne 		0x300 		/* val 3, shift 8 */
#    define DC_zmode_zlt 		0x400 		/* val 4, shift 8 */
#    define DC_zmode_zlte 		0x500 		/* val 5, shift 8 */
#    define DC_zmode_zgt 		0x600 		/* val 6, shift 8 */
#    define DC_zmode_zgte 		0x700 		/* val 7, shift 8 */
#    define DC_solid_MASK 		0xfffff7ff 	/* bit 11 */
#    define DC_solid_disable 		0x0 		
#    define DC_solid_enable 		0x800 		
#    define DC_arzero_MASK 		0xffffefff 	/* bit 12 */
#    define DC_arzero_disable 		0x0 		
#    define DC_arzero_enable 		0x1000 		
#    define DC_sgnzero_MASK 		0xffffdfff 	/* bit 13 */
#    define DC_sgnzero_disable 		0x0 		
#    define DC_sgnzero_enable 		0x2000 		
#    define DC_shftzero_MASK 		0xffffbfff 	/* bit 14 */
#    define DC_shftzero_disable 	0x0 		
#    define DC_shftzero_enable 		0x4000 		
#    define DC_bop_MASK 		0xfff0ffff 	/* bits 16-19 */
#    define DC_bop_SHIFT 		16 		
#    define DC_trans_MASK 		0xff0fffff 	/* bits 20-23 */
#    define DC_trans_SHIFT 		20 		
#    define DC_bltmod_MASK 		0xe1ffffff 	/* bits 25-28 */
#    define DC_bltmod_bmonolef 		0x0 		/* val 0, shift 25 */
#    define DC_bltmod_bmonowf 		0x8000000 	/* val 4, shift 25 */
#    define DC_bltmod_bplan 		0x2000000 	/* val 1, shift 25 */
#    define DC_bltmod_bfcol 		0x4000000 	/* val 2, shift 25 */
#    define DC_bltmod_bu32bgr 		0x6000000 	/* val 3, shift 25 */
#    define DC_bltmod_bu32rgb 		0xe000000 	/* val 7, shift 25 */
#    define DC_bltmod_bu24bgr 		0x16000000 	/* val 11, shift 25 */
#    define DC_bltmod_bu24rgb 		0x1e000000 	/* val 15, shift 25 */
#    define DC_pattern_MASK 		0xdfffffff 	/* bit 29 */
#    define DC_pattern_disable 		0x0 		
#    define DC_pattern_enable 		0x20000000 	
#    define DC_transc_MASK 		0xbfffffff 	/* bit 30 */
#    define DC_transc_disable 		0x0 		
#    define DC_transc_enable 		0x40000000 	
#    define DC_clipdis_MASK 		0x7fffffff 	/* bit 31 */
#    define DC_clipdis_disable 		0x0 		
#    define DC_clipdis_enable 		0x80000000 	

#    define DS_dwgsyncaddr_MASK 	0x3 		/* bits 2-31 */
#    define DS_dwgsyncaddr_SHIFT 	2 		

#    define FS_fifocount_MASK 		0xffffff80 	/* bits 0-6 */
#    define FS_fifocount_SHIFT 		0 		
#    define FS_bfull_MASK 		0xfffffeff 	/* bit 8 */
#    define FS_bfull_disable 		0x0 		
#    define FS_bfull_enable 		0x100 		
#    define FS_bempty_MASK 		0xfffffdff 	/* bit 9 */
#    define FS_bempty_disable 		0x0 		
#    define FS_bempty_enable 		0x200 		

#    define XA_fxleft_MASK 		0xffff0000 	/* bits 0-15 */
#    define XA_fxleft_SHIFT 		0 		
#    define XA_fxright_MASK 		0xffff 		/* bits 16-31 */
#    define XA_fxright_SHIFT 		16 		

#    define IC_softrapiclr_MASK 	0xfffffffe 	/* bit 0 */
#    define IC_softrapiclr_disable 	0x0 		
#    define IC_softrapiclr_enable 	0x1 		
#    define IC_pickiclr_MASK 		0xfffffffb 	/* bit 2 */
#    define IC_pickiclr_disable 	0x0 		
#    define IC_pickiclr_enable 		0x4 		
#    define IC_vlineiclr_MASK 		0xffffffdf 	/* bit 5 */
#    define IC_vlineiclr_disable 	0x0 		
#    define IC_vlineiclr_enable 	0x20 		
#    define IC_wiclr_MASK 		0xffffff7f 	/* bit 7 */
#    define IC_wiclr_disable 		0x0 		
#    define IC_wiclr_enable 		0x80 		
#    define IC_wciclr_MASK 		0xfffffeff 	/* bit 8 */
#    define IC_wciclr_disable 		0x0 		
#    define IC_wciclr_enable 		0x100 		

#    define IE_softrapien_MASK 		0xfffffffe 	/* bit 0 */
#    define IE_softrapien_disable 	0x0 		
#    define IE_softrapien_enable 	0x1 		
#    define IE_pickien_MASK 		0xfffffffb 	/* bit 2 */
#    define IE_pickien_disable 		0x0 		
#    define IE_pickien_enable 		0x4 		
#    define IE_vlineien_MASK 		0xffffffdf 	/* bit 5 */
#    define IE_vlineien_disable 	0x0 		
#    define IE_vlineien_enable 		0x20 		
#    define IE_extien_MASK 		0xffffffbf 	/* bit 6 */
#    define IE_extien_disable 		0x0 		
#    define IE_extien_enable 		0x40 		
#    define IE_wien_MASK 		0xffffff7f 	/* bit 7 */
#    define IE_wien_disable 		0x0 		
#    define IE_wien_enable 		0x80 		
#    define IE_wcien_MASK 		0xfffffeff 	/* bit 8 */
#    define IE_wcien_disable 		0x0 		
#    define IE_wcien_enable 		0x100 		

#    define MA_pwidth_MASK 		0xfffffffc 	/* bits 0-1 */
#    define MA_pwidth_8 		0x0 		/* val 0, shift 0 */
#    define MA_pwidth_16 		0x1 		/* val 1, shift 0 */
#    define MA_pwidth_32 		0x2 		/* val 2, shift 0 */
#    define MA_pwidth_24 		0x3 		/* val 3, shift 0 */
#    define MA_zwidth_MASK 		0xffffffe7 	/* bits 3-4 */
#    define MA_zwidth_16 		0x0 		/* val 0, shift 3 */
#    define MA_zwidth_32 		0x8 		/* val 1, shift 3 */
#    define MA_zwidth_15 		0x10 		/* val 2, shift 3 */
#    define MA_zwidth_24 		0x18 		/* val 3, shift 3 */
#    define MA_memreset_MASK 		0xffff7fff 	/* bit 15 */
#    define MA_memreset_disable 	0x0 		
#    define MA_memreset_enable 		0x8000 		
#    define MA_fogen_MASK 		0xfbffffff 	/* bit 26 */
#    define MA_fogen_disable 		0x0 		
#    define MA_fogen_enable 		0x4000000 	
#    define MA_tlutload_MASK 		0xdfffffff 	/* bit 29 */
#    define MA_tlutload_disable 	0x0 		
#    define MA_tlutload_enable 		0x20000000 	
#    define MA_nodither_MASK 		0xbfffffff 	/* bit 30 */
#    define MA_nodither_disable 	0x0 		
#    define MA_nodither_enable 		0x40000000 	
#    define MA_dit555_MASK 		0x7fffffff 	/* bit 31 */
#    define MA_dit555_disable 		0x0 		
#    define MA_dit555_enable 		0x80000000 	

#    define MCWS_casltncy_MASK 		0xfffffff8 	/* bits 0-2 */
#    define MCWS_casltncy_SHIFT 	0 		
#    define MCWS_rrddelay_MASK 		0xffffffcf 	/* bits 4-5 */
#    define MCWS_rcddelay_MASK 		0xfffffe7f 	/* bits 7-8 */
#    define MCWS_rasmin_MASK 		0xffffe3ff 	/* bits 10-12 */
#    define MCWS_rasmin_SHIFT 		10 		
#    define MCWS_rpdelay_MASK 		0xffff3fff 	/* bits 14-15 */
#    define MCWS_wrdelay_MASK 		0xfff3ffff 	/* bits 18-19 */
#    define MCWS_rddelay_MASK 		0xffdfffff 	/* bit 21 */
#    define MCWS_rddelay_disable 	0x0 		
#    define MCWS_rddelay_enable 	0x200000 	
#    define MCWS_smrdelay_MASK 		0xfe7fffff 	/* bits 23-24 */
#    define MCWS_bwcdelay_MASK 		0xf3ffffff 	/* bits 26-27 */
#    define MCWS_bpldelay_MASK 		0x1fffffff 	/* bits 29-31 */
#    define MCWS_bpldelay_SHIFT 	29 		

#    define MRB_mclkbrd0_MASK 		0xfffffff0 	/* bits 0-3 */
#    define MRB_mclkbrd0_SHIFT 		0 		
#    define MRB_mclkbrd1_MASK 		0xfffffe1f 	/* bits 5-8 */
#    define MRB_mclkbrd1_SHIFT 		5 		
#    define MRB_strmfctl_MASK 		0xff3fffff 	/* bits 22-23 */
#    define MRB_mrsopcod_MASK 		0xe1ffffff 	/* bits 25-28 */
#    define MRB_mrsopcod_SHIFT 		25 		

#    define OM_dmamod_MASK 		0xfffffff3 	/* bits 2-3 */
#    define OM_dmamod_general 		0x0 		/* val 0, shift 2 */
#    define OM_dmamod_blit 		0x4 		/* val 1, shift 2 */
#    define OM_dmamod_vector 		0x8 		/* val 2, shift 2 */
#    define OM_dmamod_vertex 		0xc 		/* val 3, shift 2 */
#    define OM_dmadatasiz_MASK 		0xfffffcff 	/* bits 8-9 */
#    define OM_dmadatasiz_8 		0x0 		/* val 0, shift 8 */
#    define OM_dmadatasiz_16 		0x100 		/* val 1, shift 8 */
#    define OM_dmadatasiz_32 		0x200 		/* val 2, shift 8 */
#    define OM_dirdatasiz_MASK 		0xfffcffff 	/* bits 16-17 */
#    define OM_dirdatasiz_8 		0x0 		/* val 0, shift 16 */
#    define OM_dirdatasiz_16 		0x10000 	/* val 1, shift 16 */
#    define OM_dirdatasiz_32 		0x20000 	/* val 2, shift 16 */

#    define P_iy_MASK 			0xffffe000 	/* bits 0-12 */
#    define P_iy_SHIFT 			0 		
#    define P_ylin_MASK 		0xffff7fff 	/* bit 15 */
#    define P_ylin_disable 		0x0 		
#    define P_ylin_enable 		0x8000 		

#    define PDCA_primod_MASK 		0xfffffffc 	/* bits 0-1 */
#    define PDCA_primod_general 	0x0 		/* val 0, shift 0 */
#    define PDCA_primod_blit 		0x1 		/* val 1, shift 0 */
#    define PDCA_primod_vector 		0x2 		/* val 2, shift 0 */
#    define PDCA_primod_vertex 		0x3 		/* val 3, shift 0 */
#    define PDCA_primaddress_MASK 	0x3 		/* bits 2-31 */
#    define PDCA_primaddress_SHIFT 	2 		

#    define PDEA_primnostart_MASK 	0xfffffffe 	/* bit 0 */
#    define PDEA_primnostart_disable 	0x0 		
#    define PDEA_primnostart_enable 	0x1 		
#    define PDEA_pagpxfer_MASK 		0xfffffffd 	/* bit 1 */
#    define PDEA_pagpxfer_disable 	0x0 		
#    define PDEA_pagpxfer_enable 	0x2 		
#    define PDEA_primend_MASK 		0x3 		/* bits 2-31 */
#    define PDEA_primend_SHIFT 		2 		

#    define PLS_primptren0_MASK 	0xfffffffe 	/* bit 0 */
#    define PLS_primptren0_disable 	0x0 		
#    define PLS_primptren0_enable 	0x1 		
#    define PLS_primptren1_MASK 	0xfffffffd 	/* bit 1 */
#    define PLS_primptren1_disable 	0x0 		
#    define PLS_primptren1_enable 	0x2 		
#    define PLS_primptr_MASK 		0x7 		/* bits 3-31 */
#    define PLS_primptr_SHIFT 		3 		

#    define R_softreset_MASK 		0xfffffffe 	/* bit 0 */
#    define R_softreset_disable 	0x0 		
#    define R_softreset_enable 		0x1 		
#    define R_softextrst_MASK 		0xfffffffd 	/* bit 1 */
#    define R_softextrst_disable 	0x0 		
#    define R_softextrst_enable 	0x2 		

#    define SDCA_secmod_MASK 		0xfffffffc 	/* bits 0-1 */
#    define SDCA_secmod_general 	0x0 		/* val 0, shift 0 */
#    define SDCA_secmod_blit 		0x1 		/* val 1, shift 0 */
#    define SDCA_secmod_vector 		0x2 		/* val 2, shift 0 */
#    define SDCA_secmod_vertex 		0x3 		/* val 3, shift 0 */
#    define SDCA_secaddress_MASK 	0x3 		/* bits 2-31 */
#    define SDCA_secaddress_SHIFT 	2 		

#    define SDEA_sagpxfer_MASK 		0xfffffffd 	/* bit 1 */
#    define SDEA_sagpxfer_disable 	0x0 		
#    define SDEA_sagpxfer_enable 	0x2 		
#    define SDEA_secend_MASK 		0x3 		/* bits 2-31 */
#    define SDEA_secend_SHIFT 		2 		

#    define SETDCA_setupmod_MASK 	0xfffffffc 	/* bits 0-1 */
#    define SETDCA_setupmod_vertlist 	0x0 		/* val 0, shift 0 */
#    define SETDCA_setupaddress_MASK 	0x3 		/* bits 2-31 */
#    define SETDCA_setupaddress_SHIFT 	2 		

#    define SETDEA_setupagpxfer_MASK 	0xfffffffd 	/* bit 1 */
#    define SETDEA_setupagpxfer_disable 0x0 		
#    define SETDEA_setupagpxfer_enable 	0x2 		
#    define SETDEA_setupend_MASK 	0x3 		/* bits 2-31 */
#    define SETDEA_setupend_SHIFT 	2 		

#    define S_sdydxl_MASK 		0xfffffffe 	/* bit 0 */
#    define S_sdydxl_y 			0x0 		
#    define S_sdydxl_x 			0x1 		
#    define S_scanleft_MASK 		0xfffffffe 	/* bit 0 */
#    define S_scanleft_disable 		0x0 		
#    define S_scanleft_enable 		0x1 		
#    define S_sdxl_MASK 		0xfffffffd 	/* bit 1 */
#    define S_sdxl_pos 			0x0 		
#    define S_sdxl_neg 			0x2 		
#    define S_sdy_MASK 			0xfffffffb 	/* bit 2 */
#    define S_sdy_pos 			0x0 		
#    define S_sdy_neg 			0x4 		
#    define S_sdxr_MASK 		0xffffffdf 	/* bit 5 */
#    define S_sdxr_pos 			0x0 		
#    define S_sdxr_neg 			0x20 		
#    define S_brkleft_MASK 		0xfffffeff 	/* bit 8 */
#    define S_brkleft_disable 		0x0 		
#    define S_brkleft_enable 		0x100 		
#    define S_errorinit_MASK 		0x7fffffff 	/* bit 31 */
#    define S_errorinit_disable 	0x0 		
#    define S_errorinit_enable 		0x80000000 	

#    define FSC_x_off_MASK 		0xfffffff0 	/* bits 0-3 */
#    define FSC_x_off_SHIFT 		0 		
#    define FSC_funcnt_MASK 		0xffffff80 	/* bits 0-6 */
#    define FSC_funcnt_SHIFT 		0 		
#    define FSC_y_off_MASK 		0xffffff8f 	/* bits 4-6 */
#    define FSC_y_off_SHIFT 		4 		
#    define FSC_funoff_MASK 		0xffc0ffff 	/* bits 16-21 */
#    define FSC_funoff_SHIFT 		16 		
#    define FSC_stylelen_MASK 		0xffc0ffff 	/* bits 16-21 */
#    define FSC_stylelen_SHIFT 		16 		


#    define STH_softraphand_MASK 	0x3 		/* bits 2-31 */
#    define STH_softraphand_SHIFT 	2 		

#    define SO_srcmap_MASK 		0xfffffffe 	/* bit 0 */
#    define SO_srcmap_fb 		0x0 		
#    define SO_srcmap_sys 		0x1 		
#    define SO_srcacc_MASK 		0xfffffffd 	/* bit 1 */
#    define SO_srcacc_pci 		0x0 		
#    define SO_srcacc_agp 		0x2 		
#    define SO_srcorg_MASK 		0x7 		/* bits 3-31 */
#    define SO_srcorg_SHIFT 		3 		

#    define STAT_softrapen_MASK 	0xfffffffe 	/* bit 0 */
#    define STAT_softrapen_disable 	0x0 		
#    define STAT_softrapen_enable 	0x1 		
#    define STAT_pickpen_MASK 		0xfffffffb 	/* bit 2 */
#    define STAT_pickpen_disable 	0x0 		
#    define STAT_pickpen_enable 	0x4 		
#    define STAT_vsyncsts_MASK 		0xfffffff7 	/* bit 3 */
#    define STAT_vsyncsts_disable 	0x0 		
#    define STAT_vsyncsts_enable 	0x8 		
#    define STAT_vsyncpen_MASK 		0xffffffef 	/* bit 4 */
#    define STAT_vsyncpen_disable 	0x0 		
#    define STAT_vsyncpen_enable 	0x10 		
#    define STAT_vlinepen_MASK 		0xffffffdf 	/* bit 5 */
#    define STAT_vlinepen_disable 	0x0 		
#    define STAT_vlinepen_enable 	0x20 		
#    define STAT_extpen_MASK 		0xffffffbf 	/* bit 6 */
#    define STAT_extpen_disable 	0x0 		
#    define STAT_extpen_enable 		0x40 		
#    define STAT_wpen_MASK 		0xffffff7f 	/* bit 7 */
#    define STAT_wpen_disable 		0x0 		
#    define STAT_wpen_enable 		0x80 		
#    define STAT_wcpen_MASK 		0xfffffeff 	/* bit 8 */
#    define STAT_wcpen_disable 		0x0 		
#    define STAT_wcpen_enable 		0x100 		
#    define STAT_dwgengsts_MASK 	0xfffeffff 	/* bit 16 */
#    define STAT_dwgengsts_disable 	0x0 		
#    define STAT_dwgengsts_enable 	0x10000 	
#    define STAT_endprdmasts_MASK 	0xfffdffff 	/* bit 17 */
#    define STAT_endprdmasts_disable 	0x0 		
#    define STAT_endprdmasts_enable 	0x20000 	
#    define STAT_wbusy_MASK 		0xfffbffff 	/* bit 18 */
#    define STAT_wbusy_disable 		0x0 		
#    define STAT_wbusy_enable 		0x40000 	
#    define STAT_swflag_MASK 		0xfffffff 	/* bits 28-31 */
#    define STAT_swflag_SHIFT 		28 		

#    define S_sref_MASK 		0xffffff00 	/* bits 0-7 */
#    define S_sref_SHIFT 		0 		
#    define S_smsk_MASK 		0xffff00ff 	/* bits 8-15 */
#    define S_smsk_SHIFT 		8 		
#    define S_swtmsk_MASK 		0xff00ffff 	/* bits 16-23 */
#    define S_swtmsk_SHIFT 		16 		

#    define SC_smode_MASK 		0xfffffff8 	/* bits 0-2 */
#    define SC_smode_salways 		0x0 		/* val 0, shift 0 */
#    define SC_smode_snever 		0x1 		/* val 1, shift 0 */
#    define SC_smode_se 		0x2 		/* val 2, shift 0 */
#    define SC_smode_sne 		0x3 		/* val 3, shift 0 */
#    define SC_smode_slt 		0x4 		/* val 4, shift 0 */
#    define SC_smode_slte 		0x5 		/* val 5, shift 0 */
#    define SC_smode_sgt 		0x6 		/* val 6, shift 0 */
#    define SC_smode_sgte 		0x7 		/* val 7, shift 0 */
#    define SC_sfailop_MASK 		0xffffffc7 	/* bits 3-5 */
#    define SC_sfailop_keep 		0x0 		/* val 0, shift 3 */
#    define SC_sfailop_zero 		0x8 		/* val 1, shift 3 */
#    define SC_sfailop_replace 		0x10 		/* val 2, shift 3 */
#    define SC_sfailop_incrsat 		0x18 		/* val 3, shift 3 */
#    define SC_sfailop_decrsat 		0x20 		/* val 4, shift 3 */
#    define SC_sfailop_invert 		0x28 		/* val 5, shift 3 */
#    define SC_sfailop_incr 		0x30 		/* val 6, shift 3 */
#    define SC_sfailop_decr 		0x38 		/* val 7, shift 3 */
#    define SC_szfailop_MASK 		0xfffffe3f 	/* bits 6-8 */
#    define SC_szfailop_keep 		0x0 		/* val 0, shift 6 */
#    define SC_szfailop_zero 		0x40 		/* val 1, shift 6 */
#    define SC_szfailop_replace 	0x80 		/* val 2, shift 6 */
#    define SC_szfailop_incrsat 	0xc0 		/* val 3, shift 6 */
#    define SC_szfailop_decrsat 	0x100 		/* val 4, shift 6 */
#    define SC_szfailop_invert 		0x140 		/* val 5, shift 6 */
#    define SC_szfailop_incr 		0x180 		/* val 6, shift 6 */
#    define SC_szfailop_decr 		0x1c0 		/* val 7, shift 6 */
#    define SC_szpassop_MASK 		0xfffff1ff 	/* bits 9-11 */
#    define SC_szpassop_keep 		0x0 		/* val 0, shift 9 */
#    define SC_szpassop_zero 		0x200 		/* val 1, shift 9 */
#    define SC_szpassop_replace 	0x400 		/* val 2, shift 9 */
#    define SC_szpassop_incrsat 	0x600 		/* val 3, shift 9 */
#    define SC_szpassop_decrsat 	0x800 		/* val 4, shift 9 */
#    define SC_szpassop_invert 		0xa00 		/* val 5, shift 9 */
#    define SC_szpassop_incr 		0xc00 		/* val 6, shift 9 */
#    define SC_szpassop_decr 		0xe00 		/* val 7, shift 9 */

#    define TD1_color1arg2selMASK 	0xfffffffc 	/* bits 0-1 */
#    define TD1_color1alphaselMASK 	0xffffffe3 	/* bits 2-4 */
#    define TD1_color1alphaselSHIFT 	2 		
#    define TD1_color1arg1alphaMASK 	0xffffffdf 	/* bit 5 */
#    define TD1_color1arg1alphadisable 	0x0 		
#    define TD1_color1arg1alphaenable 	0x20 		
#    define TD1_color1arg1invMASK 	0xffffffbf 	/* bit 6 */
#    define TD1_color1arg1invdisable 	0x0 		
#    define TD1_color1arg1invenable 	0x40 		
#    define TD1_color1arg2alphaMASK 	0xffffff7f 	/* bit 7 */
#    define TD1_color1arg2alphadisable 	0x0 		
#    define TD1_color1arg2alphaenable 	0x80 		
#    define TD1_color1arg2invMASK 	0xfffffeff 	/* bit 8 */
#    define TD1_color1arg2invdisable 	0x0 		
#    define TD1_color1arg2invenable 	0x100 		
#    define TD1_color1alpha1invMASK 	0xfffffdff 	/* bit 9 */
#    define TD1_color1alpha1invdisable 	0x0 		
#    define TD1_color1alpha1invenable 	0x200 		
#    define TD1_color1alpha2invMASK 	0xfffffbff 	/* bit 10 */
#    define TD1_color1alpha2invdisable 	0x0 		
#    define TD1_color1alpha2invenable 	0x400 		
#    define TD1_color1selMASK 		0xff9fffff 	/* bits 21-22 */
#    define TD1_color1selarg1 		0x0 		/* val 0, shift 21 */
#    define TD1_color1selarg2 		0x200000 	/* val 1, shift 21 */
#    define TD1_color1seladd 		0x400000 	/* val 2, shift 21 */
#    define TD1_color1selmul 		0x600000 	/* val 3, shift 21 */
#    define TD1_alpha1selMASK 		0x3fffffff 	/* bits 30-31 */
#    define TD1_alpha1selarg1 		0x0 		/* val 0, shift 30 */
#    define TD1_alpha1selarg2 		0x40000000 	/* val 1, shift 30 */
#    define TD1_alpha1seladd 		0x80000000 	/* val 2, shift 30 */
#    define TD1_alpha1selmul 		0xc0000000 	/* val 3, shift 30 */

#    define TST_ramtsten_MASK 		0xfffffffe 	/* bit 0 */
#    define TST_ramtsten_disable 	0x0 		
#    define TST_ramtsten_enable 	0x1 		
#    define TST_ramtstdone_MASK 	0xfffffffd 	/* bit 1 */
#    define TST_ramtstdone_disable 	0x0 		
#    define TST_ramtstdone_enable 	0x2 		
#    define TST_wramtstpass_MASK 	0xfffffffb 	/* bit 2 */
#    define TST_wramtstpass_disable 	0x0 		
#    define TST_wramtstpass_enable 	0x4 		
#    define TST_tcachetstpass_MASK 	0xfffffff7 	/* bit 3 */
#    define TST_tcachetstpass_disable 	0x0 		
#    define TST_tcachetstpass_enable 	0x8 		
#    define TST_tluttstpass_MASK 	0xffffffef 	/* bit 4 */
#    define TST_tluttstpass_disable 	0x0 		
#    define TST_tluttstpass_enable 	0x10 		
#    define TST_luttstpass_MASK 	0xffffffdf 	/* bit 5 */
#    define TST_luttstpass_disable 	0x0 		
#    define TST_luttstpass_enable 	0x20 		
#    define TST_besramtstpass_MASK 	0xffffffbf 	/* bit 6 */
#    define TST_besramtstpass_disable 	0x0 		
#    define TST_besramtstpass_enable 	0x40 		
#    define TST_ringen_MASK 		0xfffffeff 	/* bit 8 */
#    define TST_ringen_disable 		0x0 		
#    define TST_ringen_enable 		0x100 		
#    define TST_apllbyp_MASK 		0xfffffdff 	/* bit 9 */
#    define TST_apllbyp_disable 	0x0 		
#    define TST_apllbyp_enable 		0x200 		
#    define TST_hiten_MASK 		0xfffffbff 	/* bit 10 */
#    define TST_hiten_disable 		0x0 		
#    define TST_hiten_enable 		0x400 		
#    define TST_tmode_MASK 		0xffffc7ff 	/* bits 11-13 */
#    define TST_tmode_SHIFT 		11 		
#    define TST_tclksel_MASK 		0xfffe3fff 	/* bits 14-16 */
#    define TST_tclksel_SHIFT 		14 		
#    define TST_ringcnten_MASK 		0xfffdffff 	/* bit 17 */
#    define TST_ringcnten_disable 	0x0 		
#    define TST_ringcnten_enable 	0x20000 	
#    define TST_ringcnt_MASK 		0xc003ffff 	/* bits 18-29 */
#    define TST_ringcnt_SHIFT 		18 		
#    define TST_ringcntclksl_MASK 	0xbfffffff 	/* bit 30 */
#    define TST_ringcntclksl_disable 	0x0 		
#    define TST_ringcntclksl_enable 	0x40000000 	
#    define TST_biosboot_MASK 		0x7fffffff 	/* bit 31 */
#    define TST_biosboot_disable 	0x0 		
#    define TST_biosboot_enable 	0x80000000 	

#    define TMC_tformat_MASK 		0xfffffff0 	/* bits 0-3 */
#    define TMC_tformat_tw4 		0x0 		/* val 0, shift 0 */
#    define TMC_tformat_tw8 		0x1 		/* val 1, shift 0 */
#    define TMC_tformat_tw15 		0x2 		/* val 2, shift 0 */
#    define TMC_tformat_tw16 		0x3 		/* val 3, shift 0 */
#    define TMC_tformat_tw12 		0x4 		/* val 4, shift 0 */
#    define TMC_tformat_tw32 		0x6 		/* val 6, shift 0 */
#    define TMC_tformat_tw422 		0xa 		/* val 10, shift 0 */
#    define TMC_tpitchlin_MASK 		0xfffffeff 	/* bit 8 */
#    define TMC_tpitchlin_disable 	0x0 		
#    define TMC_tpitchlin_enable 	0x100 		
#    define TMC_tpitchext_MASK 		0xfff001ff 	/* bits 9-19 */
#    define TMC_tpitchext_SHIFT 	9 		
#    define TMC_tpitch_MASK 		0xfff8ffff 	/* bits 16-18 */
#    define TMC_tpitch_SHIFT 		16 		
#    define TMC_owalpha_MASK 		0xffbfffff 	/* bit 22 */
#    define TMC_owalpha_disable 	0x0 		
#    define TMC_owalpha_enable 		0x400000 	
#    define TMC_azeroextend_MASK 	0xff7fffff 	/* bit 23 */
#    define TMC_azeroextend_disable 	0x0 		
#    define TMC_azeroextend_enable 	0x800000 	
#    define TMC_decalckey_MASK 		0xfeffffff 	/* bit 24 */
#    define TMC_decalckey_disable 	0x0 		
#    define TMC_decalckey_enable 	0x1000000 	
#    define TMC_takey_MASK 		0xfdffffff 	/* bit 25 */
#    define TMC_takey_0 		0x0 		
#    define TMC_takey_1 		0x2000000 	
#    define TMC_tamask_MASK 		0xfbffffff 	/* bit 26 */
#    define TMC_tamask_0 		0x0 		
#    define TMC_tamask_1 		0x4000000 	
#    define TMC_clampv_MASK 		0xf7ffffff 	/* bit 27 */
#    define TMC_clampv_disable 		0x0 		
#    define TMC_clampv_enable 		0x8000000 	
#    define TMC_clampu_MASK 		0xefffffff 	/* bit 28 */
#    define TMC_clampu_disable 		0x0 		
#    define TMC_clampu_enable 		0x10000000 	
#    define TMC_tmodulate_MASK 		0xdfffffff 	/* bit 29 */
#    define TMC_tmodulate_disable 	0x0 		
#    define TMC_tmodulate_enable 	0x20000000 	
#    define TMC_strans_MASK 		0xbfffffff 	/* bit 30 */
#    define TMC_strans_disable 		0x0 		
#    define TMC_strans_enable 		0x40000000 	
#    define TMC_itrans_MASK 		0x7fffffff 	/* bit 31 */
#    define TMC_itrans_disable 		0x0 		
#    define TMC_itrans_enable 		0x80000000 	

#    define TMC_decalblend_MASK 	0xfffffffe 	/* bit 0 */
#    define TMC_decalblend_disable 	0x0 		
#    define TMC_decalblend_enable 	0x1 		
#    define TMC_idecal_MASK 		0xfffffffd 	/* bit 1 */
#    define TMC_idecal_disable 		0x0 		
#    define TMC_idecal_enable 		0x2 		
#    define TMC_decaldis_MASK 		0xfffffffb 	/* bit 2 */
#    define TMC_decaldis_disable 	0x0 		
#    define TMC_decaldis_enable 	0x4 		
#    define TMC_ckstransdis_MASK 	0xffffffef 	/* bit 4 */
#    define TMC_ckstransdis_disable 	0x0 		
#    define TMC_ckstransdis_enable 	0x10 		
#    define TMC_borderen_MASK 		0xffffffdf 	/* bit 5 */
#    define TMC_borderen_disable 	0x0 		
#    define TMC_borderen_enable 	0x20 		
#    define TMC_specen_MASK 		0xffffffbf 	/* bit 6 */
#    define TMC_specen_disable 		0x0 		
#    define TMC_specen_enable 		0x40 		

#    define TF_minfilter_MASK 		0xfffffff0 	/* bits 0-3 */
#    define TF_minfilter_nrst 		0x0 		/* val 0, shift 0 */
#    define TF_minfilter_bilin 		0x2 		/* val 2, shift 0 */
#    define TF_minfilter_cnst 		0x3 		/* val 3, shift 0 */
#    define TF_minfilter_mm1s 		0x8 		/* val 8, shift 0 */
#    define TF_minfilter_mm2s 		0x9 		/* val 9, shift 0 */
#    define TF_minfilter_mm4s 		0xa 		/* val 10, shift 0 */
#    define TF_minfilter_mm8s 		0xc 		/* val 12, shift 0 */
#    define TF_magfilter_MASK 		0xffffff0f 	/* bits 4-7 */
#    define TF_magfilter_nrst 		0x0 		/* val 0, shift 4 */
#    define TF_magfilter_bilin 		0x20 		/* val 2, shift 4 */
#    define TF_magfilter_cnst 		0x30 		/* val 3, shift 4 */
#    define TF_avgstride_MASK 		0xfff7ffff 	/* bit 19 */
#    define TF_avgstride_disable 	0x0 		
#    define TF_avgstride_enable 	0x80000 	
#    define TF_filteralpha_MASK 	0xffefffff 	/* bit 20 */
#    define TF_filteralpha_disable 	0x0 		
#    define TF_filteralpha_enable 	0x100000 	
#    define TF_fthres_MASK 		0xe01fffff 	/* bits 21-28 */
#    define TF_fthres_SHIFT 		21 		
#    define TF_mapnb_MASK 		0x1fffffff 	/* bits 29-31 */
#    define TF_mapnb_SHIFT 		29 		

#    define TH_th_MASK 			0xffffffc0 	/* bits 0-5 */
#    define TH_th_SHIFT 		0 		
#    define TH_rfh_MASK 		0xffff81ff 	/* bits 9-14 */
#    define TH_rfh_SHIFT 		9 		
#    define TH_thmask_MASK 		0xe003ffff 	/* bits 18-28 */
#    define TH_thmask_SHIFT 		18 		

#    define TO_texorgmap_MASK 		0xfffffffe 	/* bit 0 */
#    define TO_texorgmap_fb 		0x0 		
#    define TO_texorgmap_sys 		0x1 		
#    define TO_texorgacc_MASK 		0xfffffffd 	/* bit 1 */
#    define TO_texorgacc_pci 		0x0 		
#    define TO_texorgacc_agp 		0x2 		
#    define TO_texorg_MASK 		0x1f 		/* bits 5-31 */
#    define TO_texorg_SHIFT 		5 		

#    define TT_tckey_MASK 		0xffff0000 	/* bits 0-15 */
#    define TT_tckey_SHIFT 		0 		
#    define TT_tkmask_MASK 		0xffff 		/* bits 16-31 */
#    define TT_tkmask_SHIFT 		16 		

#    define TT_tckeyh_MASK 		0xffff0000 	/* bits 0-15 */
#    define TT_tckeyh_SHIFT 		0 		
#    define TT_tkmaskh_MASK 		0xffff 		/* bits 16-31 */
#    define TT_tkmaskh_SHIFT 		16 		

#    define TW_tw_MASK 			0xffffffc0 	/* bits 0-5 */
#    define TW_tw_SHIFT 		0 		
#    define TW_rfw_MASK 		0xffff81ff 	/* bits 9-14 */
#    define TW_rfw_SHIFT 		9 		
#    define TW_twmask_MASK 		0xe003ffff 	/* bits 18-28 */
#    define TW_twmask_SHIFT 		18 		

#    define WAS_seqdst0_MASK 		0xffffffc0 	/* bits 0-5 */
#    define WAS_seqdst0_SHIFT 		0 		
#    define WAS_seqdst1_MASK 		0xfffff03f 	/* bits 6-11 */
#    define WAS_seqdst1_SHIFT 		6 		
#    define WAS_seqdst2_MASK 		0xfffc0fff 	/* bits 12-17 */
#    define WAS_seqdst2_SHIFT 		12 		
#    define WAS_seqdst3_MASK 		0xff03ffff 	/* bits 18-23 */
#    define WAS_seqdst3_SHIFT 		18 		
#    define WAS_seqlen_MASK 		0xfcffffff 	/* bits 24-25 */
#    define WAS_wfirsttag_MASK 		0xfbffffff 	/* bit 26 */
#    define WAS_wfirsttag_disable 	0x0 		
#    define WAS_wfirsttag_enable 	0x4000000 	
#    define WAS_wsametag_MASK 		0xf7ffffff 	/* bit 27 */
#    define WAS_wsametag_disable 	0x0 		
#    define WAS_wsametag_enable 	0x8000000 	
#    define WAS_seqoff_MASK 		0xefffffff 	/* bit 28 */
#    define WAS_seqoff_disable 		0x0 		
#    define WAS_seqoff_enable 		0x10000000 	

#    define WMA_wcodeaddr_MASK 		0xff 		/* bits 8-31 */
#    define WMA_wcodeaddr_SHIFT 	8 		

#    define WF_walustsflag_MASK 	0xffffff00 	/* bits 0-7 */
#    define WF_walustsflag_SHIFT 	0 		
#    define WF_walucfgflag_MASK 	0xffff00ff 	/* bits 8-15 */
#    define WF_walucfgflag_SHIFT 	8 		
#    define WF_wprgflag_MASK 		0xffff 		/* bits 16-31 */
#    define WF_wprgflag_SHIFT 		16 		

#    define WF1_walustsflag1_MASK 	0xffffff00 	/* bits 0-7 */
#    define WF1_walustsflag1_SHIFT 	0 		
#    define WF1_walucfgflag1_MASK 	0xffff00ff 	/* bits 8-15 */
#    define WF1_walucfgflag1_SHIFT 	8 		
#    define WF1_wprgflag1_MASK 		0xffff 		/* bits 16-31 */
#    define WF1_wprgflag1_SHIFT 	16 		

#    define WGV_wgetmsbmin_MASK 	0xffffffe0 	/* bits 0-4 */
#    define WGV_wgetmsbmin_SHIFT 	0 		
#    define WGV_wgetmsbmax_MASK 	0xffffe0ff 	/* bits 8-12 */
#    define WGV_wgetmsbmax_SHIFT 	8 		
#    define WGV_wbrklefttop_MASK 	0xfffeffff 	/* bit 16 */
#    define WGV_wbrklefttop_disable 	0x0 		
#    define WGV_wbrklefttop_enable 	0x10000 	
#    define WGV_wfastcrop_MASK 		0xfffdffff 	/* bit 17 */
#    define WGV_wfastcrop_disable 	0x0 		
#    define WGV_wfastcrop_enable 	0x20000 	
#    define WGV_wcentersnap_MASK 	0xfffbffff 	/* bit 18 */
#    define WGV_wcentersnap_disable 	0x0 		
#    define WGV_wcentersnap_enable 	0x40000 	
#    define WGV_wbrkrighttop_MASK 	0xfff7ffff 	/* bit 19 */
#    define WGV_wbrkrighttop_disable 	0x0 		
#    define WGV_wbrkrighttop_enable 	0x80000 	

#    define WIA_wmode_MASK 		0xfffffffc 	/* bits 0-1 */
#    define WIA_wmode_suspend 		0x0 		/* val 0, shift 0 */
#    define WIA_wmode_resume 		0x1 		/* val 1, shift 0 */
#    define WIA_wmode_jump 		0x2 		/* val 2, shift 0 */
#    define WIA_wmode_start 		0x3 		/* val 3, shift 0 */
#    define WIA_wagp_MASK 		0xfffffffb 	/* bit 2 */
#    define WIA_wagp_pci 		0x0 		
#    define WIA_wagp_agp 		0x4 		
#    define WIA_wiaddr_MASK 		0x7 		/* bits 3-31 */
#    define WIA_wiaddr_SHIFT 		3 		

#    define WIA2_wmode_MASK 		0xfffffffc 	/* bits 0-1 */
#    define WIA2_wmode_suspend 		0x0 		/* val 0, shift 0 */
#    define WIA2_wmode_resume 		0x1 		/* val 1, shift 0 */
#    define WIA2_wmode_jump 		0x2 		/* val 2, shift 0 */
#    define WIA2_wmode_start 		0x3 		/* val 3, shift 0 */
#    define WIA2_wagp_MASK 		0xfffffffb 	/* bit 2 */
#    define WIA2_wagp_pci 		0x0 		
#    define WIA2_wagp_agp 		0x4 		
#    define WIA2_wiaddr_MASK 		0x7 		/* bits 3-31 */
#    define WIA2_wiaddr_SHIFT 		3 		

#    define WIMA_wimemaddr_MASK 	0xffffff00 	/* bits 0-7 */
#    define WIMA_wimemaddr_SHIFT 	0 		

#    define WM_wucodecache_MASK 	0xfffffffe 	/* bit 0 */
#    define WM_wucodecache_disable 	0x0 		
#    define WM_wucodecache_enable 	0x1 		
#    define WM_wmaster_MASK 		0xfffffffd 	/* bit 1 */
#    define WM_wmaster_disable 		0x0 		
#    define WM_wmaster_enable 		0x2 		
#    define WM_wcacheflush_MASK 	0xfffffff7 	/* bit 3 */
#    define WM_wcacheflush_disable 	0x0 		
#    define WM_wcacheflush_enable 	0x8 		

#    define WVS_wvrtxsz_MASK 		0xffffffc0 	/* bits 0-5 */
#    define WVS_wvrtxsz_SHIFT 		0 		
#    define WVS_primsz_MASK 		0xffffc0ff 	/* bits 8-13 */
#    define WVS_primsz_SHIFT 		8 		

#    define XYEA_x_end_MASK 		0xffff0000 	/* bits 0-15 */
#    define XYEA_x_end_SHIFT 		0 		
#    define XYEA_y_end_MASK 		0xffff 		/* bits 16-31 */
#    define XYEA_y_end_SHIFT 		16 		

#    define XYSA_x_start_MASK 		0xffff0000 	/* bits 0-15 */
#    define XYSA_x_start_SHIFT 		0 		
#    define XYSA_y_start_MASK 		0xffff 		/* bits 16-31 */
#    define XYSA_y_start_SHIFT 		16 		

#    define YA_ydst_MASK 		0xff800000 	/* bits 0-22 */
#    define YA_ydst_SHIFT 		0 		
#    define YA_sellin_MASK 		0x1fffffff 	/* bits 29-31 */
#    define YA_sellin_SHIFT 		29 		

#    define YDL_length_MASK 		0xffff0000 	/* bits 0-15 */
#    define YDL_length_SHIFT 		0 		
#    define YDL_yval_MASK 		0xffff 		/* bits 16-31 */
#    define YDL_yval_SHIFT 		16 		

#    define ZO_zorgmap_MASK 		0xfffffffe 	/* bit 0 */
#    define ZO_zorgmap_fb 		0x0 		
#    define ZO_zorgmap_sys 		0x1 		
#    define ZO_zorgacc_MASK 		0xfffffffd 	/* bit 1 */
#    define ZO_zorgacc_pci 		0x0 		
#    define ZO_zorgacc_agp 		0x2 		
#    define ZO_zorg_MASK 		0x3 		/* bits 2-31 */
#    define ZO_zorg_SHIFT 		2 		




/**************** (END) AUTOMATICALLY GENERATED REGISTER FILE ****************/

#endif 	/* _MGAREGS_H_ */

