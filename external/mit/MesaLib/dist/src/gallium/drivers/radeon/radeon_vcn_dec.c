/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "radeon_vcn_dec.h"

#include "pipe/p_video_codec.h"
#include "radeon_video.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_mpeg12_decoder.h"
#include "vl/vl_probs_table.h"
#include "pspdecryptionparam.h"

#include <assert.h>
#include <stdio.h>

#include "radeon_vcn_av1_default.h"

#define FB_BUFFER_OFFSET             0x1000
#define FB_BUFFER_SIZE               2048
#define IT_SCALING_TABLE_SIZE        992
#define VP9_PROBS_TABLE_SIZE         (RDECODE_VP9_PROBS_DATA_SIZE + 256)
#define RDECODE_SESSION_CONTEXT_SIZE (128 * 1024)

#define RDECODE_VCN1_GPCOM_VCPU_CMD   0x2070c
#define RDECODE_VCN1_GPCOM_VCPU_DATA0 0x20710
#define RDECODE_VCN1_GPCOM_VCPU_DATA1 0x20714
#define RDECODE_VCN1_ENGINE_CNTL      0x20718

#define RDECODE_VCN2_GPCOM_VCPU_CMD   (0x503 << 2)
#define RDECODE_VCN2_GPCOM_VCPU_DATA0 (0x504 << 2)
#define RDECODE_VCN2_GPCOM_VCPU_DATA1 (0x505 << 2)
#define RDECODE_VCN2_ENGINE_CNTL      (0x506 << 2)

#define RDECODE_VCN2_5_GPCOM_VCPU_CMD   0x3c
#define RDECODE_VCN2_5_GPCOM_VCPU_DATA0 0x40
#define RDECODE_VCN2_5_GPCOM_VCPU_DATA1 0x44
#define RDECODE_VCN2_5_ENGINE_CNTL      0x9b4

#define NUM_MPEG2_REFS 6
#define NUM_H264_REFS  17
#define NUM_VC1_REFS   5
#define NUM_VP9_REFS   8
#define NUM_AV1_REFS   8
#define NUM_AV1_REFS_PER_FRAME 7

static unsigned calc_dpb_size(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h264_perf(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h265_main(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h265_main10(struct radeon_decoder *dec,
                                          struct pipe_h265_picture_desc *pic);

static rvcn_dec_message_avc_t get_h264_msg(struct radeon_decoder *dec,
                                           struct pipe_h264_picture_desc *pic)
{
   rvcn_dec_message_avc_t result;

   memset(&result, 0, sizeof(result));
   switch (pic->base.profile) {
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
      result.profile = RDECODE_H264_PROFILE_BASELINE;
      break;

   case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
      result.profile = RDECODE_H264_PROFILE_MAIN;
      break;

   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
      result.profile = RDECODE_H264_PROFILE_HIGH;
      break;

   default:
      assert(0);
      break;
   }

   result.level = dec->base.level;

   result.sps_info_flags = 0;
   result.sps_info_flags |= pic->pps->sps->direct_8x8_inference_flag << 0;
   result.sps_info_flags |= pic->pps->sps->mb_adaptive_frame_field_flag << 1;
   result.sps_info_flags |= pic->pps->sps->frame_mbs_only_flag << 2;
   result.sps_info_flags |= pic->pps->sps->delta_pic_order_always_zero_flag << 3;
   result.sps_info_flags |= 1 << RDECODE_SPS_INFO_H264_EXTENSION_SUPPORT_FLAG_SHIFT;

   result.bit_depth_luma_minus8 = pic->pps->sps->bit_depth_luma_minus8;
   result.bit_depth_chroma_minus8 = pic->pps->sps->bit_depth_chroma_minus8;
   result.log2_max_frame_num_minus4 = pic->pps->sps->log2_max_frame_num_minus4;
   result.pic_order_cnt_type = pic->pps->sps->pic_order_cnt_type;
   result.log2_max_pic_order_cnt_lsb_minus4 = pic->pps->sps->log2_max_pic_order_cnt_lsb_minus4;

   switch (dec->base.chroma_format) {
   case PIPE_VIDEO_CHROMA_FORMAT_NONE:
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_400:
      result.chroma_format = 0;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_420:
      result.chroma_format = 1;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_422:
      result.chroma_format = 2;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_444:
      result.chroma_format = 3;
      break;
   }

   result.pps_info_flags = 0;
   result.pps_info_flags |= pic->pps->transform_8x8_mode_flag << 0;
   result.pps_info_flags |= pic->pps->redundant_pic_cnt_present_flag << 1;
   result.pps_info_flags |= pic->pps->constrained_intra_pred_flag << 2;
   result.pps_info_flags |= pic->pps->deblocking_filter_control_present_flag << 3;
   result.pps_info_flags |= pic->pps->weighted_bipred_idc << 4;
   result.pps_info_flags |= pic->pps->weighted_pred_flag << 6;
   result.pps_info_flags |= pic->pps->bottom_field_pic_order_in_frame_present_flag << 7;
   result.pps_info_flags |= pic->pps->entropy_coding_mode_flag << 8;

   result.num_slice_groups_minus1 = pic->pps->num_slice_groups_minus1;
   result.slice_group_map_type = pic->pps->slice_group_map_type;
   result.slice_group_change_rate_minus1 = pic->pps->slice_group_change_rate_minus1;
   result.pic_init_qp_minus26 = pic->pps->pic_init_qp_minus26;
   result.chroma_qp_index_offset = pic->pps->chroma_qp_index_offset;
   result.second_chroma_qp_index_offset = pic->pps->second_chroma_qp_index_offset;

   memcpy(result.scaling_list_4x4, pic->pps->ScalingList4x4, 6 * 16);
   memcpy(result.scaling_list_8x8, pic->pps->ScalingList8x8, 2 * 64);

   memcpy(dec->it, result.scaling_list_4x4, 6 * 16);
   memcpy((dec->it + 96), result.scaling_list_8x8, 2 * 64);

   result.num_ref_frames = pic->num_ref_frames;

   result.num_ref_idx_l0_active_minus1 = pic->num_ref_idx_l0_active_minus1;
   result.num_ref_idx_l1_active_minus1 = pic->num_ref_idx_l1_active_minus1;

   result.frame_num = pic->frame_num;
   memcpy(result.frame_num_list, pic->frame_num_list, 4 * 16);
   result.curr_field_order_cnt_list[0] = pic->field_order_cnt[0];
   result.curr_field_order_cnt_list[1] = pic->field_order_cnt[1];
   memcpy(result.field_order_cnt_list, pic->field_order_cnt_list, 4 * 16 * 2);

   result.decoded_pic_idx = pic->frame_num;

   return result;
}

static void radeon_dec_destroy_associated_data(void *data)
{
   /* NOOP, since we only use an intptr */
}

static rvcn_dec_message_hevc_t get_h265_msg(struct radeon_decoder *dec,
                                            struct pipe_video_buffer *target,
                                            struct pipe_h265_picture_desc *pic)
{
   rvcn_dec_message_hevc_t result;
   unsigned i, j;

   memset(&result, 0, sizeof(result));
   result.sps_info_flags = 0;
   result.sps_info_flags |= pic->pps->sps->scaling_list_enabled_flag << 0;
   result.sps_info_flags |= pic->pps->sps->amp_enabled_flag << 1;
   result.sps_info_flags |= pic->pps->sps->sample_adaptive_offset_enabled_flag << 2;
   result.sps_info_flags |= pic->pps->sps->pcm_enabled_flag << 3;
   result.sps_info_flags |= pic->pps->sps->pcm_loop_filter_disabled_flag << 4;
   result.sps_info_flags |= pic->pps->sps->long_term_ref_pics_present_flag << 5;
   result.sps_info_flags |= pic->pps->sps->sps_temporal_mvp_enabled_flag << 6;
   result.sps_info_flags |= pic->pps->sps->strong_intra_smoothing_enabled_flag << 7;
   result.sps_info_flags |= pic->pps->sps->separate_colour_plane_flag << 8;
   if (((struct si_screen *)dec->screen)->info.family == CHIP_CARRIZO)
      result.sps_info_flags |= 1 << 9;
   if (pic->UseRefPicList == true)
      result.sps_info_flags |= 1 << 10;
   if (pic->UseStRpsBits == true && pic->pps->st_rps_bits != 0) {
      result.sps_info_flags |= 1 << 11;
      result.st_rps_bits = pic->pps->st_rps_bits;
  }

   result.chroma_format = pic->pps->sps->chroma_format_idc;
   result.bit_depth_luma_minus8 = pic->pps->sps->bit_depth_luma_minus8;
   result.bit_depth_chroma_minus8 = pic->pps->sps->bit_depth_chroma_minus8;
   result.log2_max_pic_order_cnt_lsb_minus4 = pic->pps->sps->log2_max_pic_order_cnt_lsb_minus4;
   result.sps_max_dec_pic_buffering_minus1 = pic->pps->sps->sps_max_dec_pic_buffering_minus1;
   result.log2_min_luma_coding_block_size_minus3 =
      pic->pps->sps->log2_min_luma_coding_block_size_minus3;
   result.log2_diff_max_min_luma_coding_block_size =
      pic->pps->sps->log2_diff_max_min_luma_coding_block_size;
   result.log2_min_transform_block_size_minus2 =
      pic->pps->sps->log2_min_transform_block_size_minus2;
   result.log2_diff_max_min_transform_block_size =
      pic->pps->sps->log2_diff_max_min_transform_block_size;
   result.max_transform_hierarchy_depth_inter = pic->pps->sps->max_transform_hierarchy_depth_inter;
   result.max_transform_hierarchy_depth_intra = pic->pps->sps->max_transform_hierarchy_depth_intra;
   result.pcm_sample_bit_depth_luma_minus1 = pic->pps->sps->pcm_sample_bit_depth_luma_minus1;
   result.pcm_sample_bit_depth_chroma_minus1 = pic->pps->sps->pcm_sample_bit_depth_chroma_minus1;
   result.log2_min_pcm_luma_coding_block_size_minus3 =
      pic->pps->sps->log2_min_pcm_luma_coding_block_size_minus3;
   result.log2_diff_max_min_pcm_luma_coding_block_size =
      pic->pps->sps->log2_diff_max_min_pcm_luma_coding_block_size;
   result.num_short_term_ref_pic_sets = pic->pps->sps->num_short_term_ref_pic_sets;

   result.pps_info_flags = 0;
   result.pps_info_flags |= pic->pps->dependent_slice_segments_enabled_flag << 0;
   result.pps_info_flags |= pic->pps->output_flag_present_flag << 1;
   result.pps_info_flags |= pic->pps->sign_data_hiding_enabled_flag << 2;
   result.pps_info_flags |= pic->pps->cabac_init_present_flag << 3;
   result.pps_info_flags |= pic->pps->constrained_intra_pred_flag << 4;
   result.pps_info_flags |= pic->pps->transform_skip_enabled_flag << 5;
   result.pps_info_flags |= pic->pps->cu_qp_delta_enabled_flag << 6;
   result.pps_info_flags |= pic->pps->pps_slice_chroma_qp_offsets_present_flag << 7;
   result.pps_info_flags |= pic->pps->weighted_pred_flag << 8;
   result.pps_info_flags |= pic->pps->weighted_bipred_flag << 9;
   result.pps_info_flags |= pic->pps->transquant_bypass_enabled_flag << 10;
   result.pps_info_flags |= pic->pps->tiles_enabled_flag << 11;
   result.pps_info_flags |= pic->pps->entropy_coding_sync_enabled_flag << 12;
   result.pps_info_flags |= pic->pps->uniform_spacing_flag << 13;
   result.pps_info_flags |= pic->pps->loop_filter_across_tiles_enabled_flag << 14;
   result.pps_info_flags |= pic->pps->pps_loop_filter_across_slices_enabled_flag << 15;
   result.pps_info_flags |= pic->pps->deblocking_filter_override_enabled_flag << 16;
   result.pps_info_flags |= pic->pps->pps_deblocking_filter_disabled_flag << 17;
   result.pps_info_flags |= pic->pps->lists_modification_present_flag << 18;
   result.pps_info_flags |= pic->pps->slice_segment_header_extension_present_flag << 19;

   result.num_extra_slice_header_bits = pic->pps->num_extra_slice_header_bits;
   result.num_long_term_ref_pic_sps = pic->pps->sps->num_long_term_ref_pics_sps;
   result.num_ref_idx_l0_default_active_minus1 = pic->pps->num_ref_idx_l0_default_active_minus1;
   result.num_ref_idx_l1_default_active_minus1 = pic->pps->num_ref_idx_l1_default_active_minus1;
   result.pps_cb_qp_offset = pic->pps->pps_cb_qp_offset;
   result.pps_cr_qp_offset = pic->pps->pps_cr_qp_offset;
   result.pps_beta_offset_div2 = pic->pps->pps_beta_offset_div2;
   result.pps_tc_offset_div2 = pic->pps->pps_tc_offset_div2;
   result.diff_cu_qp_delta_depth = pic->pps->diff_cu_qp_delta_depth;
   result.num_tile_columns_minus1 = pic->pps->num_tile_columns_minus1;
   result.num_tile_rows_minus1 = pic->pps->num_tile_rows_minus1;
   result.log2_parallel_merge_level_minus2 = pic->pps->log2_parallel_merge_level_minus2;
   result.init_qp_minus26 = pic->pps->init_qp_minus26;

   for (i = 0; i < 19; ++i)
      result.column_width_minus1[i] = pic->pps->column_width_minus1[i];

   for (i = 0; i < 21; ++i)
      result.row_height_minus1[i] = pic->pps->row_height_minus1[i];

   result.num_delta_pocs_ref_rps_idx = pic->NumDeltaPocsOfRefRpsIdx;
   result.curr_poc = pic->CurrPicOrderCntVal;

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      for (j = 0; 
           (pic->ref[j] != NULL) && (j < ARRAY_SIZE(dec->render_pic_list));
           j++) {
         if (dec->render_pic_list[i] == pic->ref[j])
            break;
         if (j == ARRAY_SIZE(dec->render_pic_list) - 1)
            dec->render_pic_list[i] = NULL;
         else if (pic->ref[j + 1] == NULL)
            dec->render_pic_list[i] = NULL;
      }
   }
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      if (dec->render_pic_list[i] == NULL) {
         dec->render_pic_list[i] = target;
         result.curr_idx = i;
         break;
      }
   }

   vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)result.curr_idx,
                                       &radeon_dec_destroy_associated_data);

   for (i = 0; i < 16; ++i) {
      struct pipe_video_buffer *ref = pic->ref[i];
      uintptr_t ref_pic = 0;

      result.poc_list[i] = pic->PicOrderCntVal[i];

      if (ref)
         ref_pic = (uintptr_t)vl_video_buffer_get_associated_data(ref, &dec->base);
      else
         ref_pic = 0x7F;
      result.ref_pic_list[i] = ref_pic;
   }

   for (i = 0; i < 8; ++i) {
      result.ref_pic_set_st_curr_before[i] = 0xFF;
      result.ref_pic_set_st_curr_after[i] = 0xFF;
      result.ref_pic_set_lt_curr[i] = 0xFF;
   }

   for (i = 0; i < pic->NumPocStCurrBefore; ++i)
      result.ref_pic_set_st_curr_before[i] = pic->RefPicSetStCurrBefore[i];

   for (i = 0; i < pic->NumPocStCurrAfter; ++i)
      result.ref_pic_set_st_curr_after[i] = pic->RefPicSetStCurrAfter[i];

   for (i = 0; i < pic->NumPocLtCurr; ++i)
      result.ref_pic_set_lt_curr[i] = pic->RefPicSetLtCurr[i];

   for (i = 0; i < 6; ++i)
      result.ucScalingListDCCoefSizeID2[i] = pic->pps->sps->ScalingListDCCoeff16x16[i];

   for (i = 0; i < 2; ++i)
      result.ucScalingListDCCoefSizeID3[i] = pic->pps->sps->ScalingListDCCoeff32x32[i];

   memcpy(dec->it, pic->pps->sps->ScalingList4x4, 6 * 16);
   memcpy(dec->it + 96, pic->pps->sps->ScalingList8x8, 6 * 64);
   memcpy(dec->it + 480, pic->pps->sps->ScalingList16x16, 6 * 64);
   memcpy(dec->it + 864, pic->pps->sps->ScalingList32x32, 2 * 64);

   for (i = 0; i < 2; i++) {
      for (j = 0; j < 15; j++)
         result.direct_reflist[i][j] = pic->RefPicList[i][j];
   }

   if (pic->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.p010_mode = 0;
         result.luma_10to8 = 5;
         result.chroma_10to8 = 5;
         result.hevc_reserved[0] = 4; /* sclr_luma10to8 */
         result.hevc_reserved[1] = 4; /* sclr_chroma10to8 */
      }
   }

   return result;
}

static void fill_probs_table(void *ptr)
{
   rvcn_dec_vp9_probs_t *probs = (rvcn_dec_vp9_probs_t *)ptr;

   memcpy(&probs->coef_probs[0], default_coef_probs_4x4, sizeof(default_coef_probs_4x4));
   memcpy(&probs->coef_probs[1], default_coef_probs_8x8, sizeof(default_coef_probs_8x8));
   memcpy(&probs->coef_probs[2], default_coef_probs_16x16, sizeof(default_coef_probs_16x16));
   memcpy(&probs->coef_probs[3], default_coef_probs_32x32, sizeof(default_coef_probs_32x32));
   memcpy(probs->y_mode_prob, default_if_y_probs, sizeof(default_if_y_probs));
   memcpy(probs->uv_mode_prob, default_if_uv_probs, sizeof(default_if_uv_probs));
   memcpy(probs->single_ref_prob, default_single_ref_p, sizeof(default_single_ref_p));
   memcpy(probs->switchable_interp_prob, default_switchable_interp_prob,
          sizeof(default_switchable_interp_prob));
   memcpy(probs->partition_prob, default_partition_probs, sizeof(default_partition_probs));
   memcpy(probs->inter_mode_probs, default_inter_mode_probs, sizeof(default_inter_mode_probs));
   memcpy(probs->mbskip_probs, default_skip_probs, sizeof(default_skip_probs));
   memcpy(probs->intra_inter_prob, default_intra_inter_p, sizeof(default_intra_inter_p));
   memcpy(probs->comp_inter_prob, default_comp_inter_p, sizeof(default_comp_inter_p));
   memcpy(probs->comp_ref_prob, default_comp_ref_p, sizeof(default_comp_ref_p));
   memcpy(probs->tx_probs_32x32, default_tx_probs_32x32, sizeof(default_tx_probs_32x32));
   memcpy(probs->tx_probs_16x16, default_tx_probs_16x16, sizeof(default_tx_probs_16x16));
   memcpy(probs->tx_probs_8x8, default_tx_probs_8x8, sizeof(default_tx_probs_8x8));
   memcpy(probs->mv_joints, default_nmv_joints, sizeof(default_nmv_joints));
   memcpy(&probs->mv_comps[0], default_nmv_components, sizeof(default_nmv_components));
   memset(&probs->nmvc_mask, 0, sizeof(rvcn_dec_vp9_nmv_ctx_mask_t));
}

static rvcn_dec_message_vp9_t get_vp9_msg(struct radeon_decoder *dec,
                                          struct pipe_video_buffer *target,
                                          struct pipe_vp9_picture_desc *pic)
{
   rvcn_dec_message_vp9_t result;
   unsigned i ,j;

   memset(&result, 0, sizeof(result));

   /* segment table */
   rvcn_dec_vp9_probs_segment_t *prbs = (rvcn_dec_vp9_probs_segment_t *)(dec->probs);

   if (pic->picture_parameter.pic_fields.segmentation_enabled) {
      for (i = 0; i < 8; ++i) {
         prbs->seg.feature_data[i] =
            (pic->slice_parameter.seg_param[i].alt_quant & 0xffff) |
            ((pic->slice_parameter.seg_param[i].alt_lf & 0xff) << 16) |
            ((pic->slice_parameter.seg_param[i].segment_flags.segment_reference & 0xf) << 24);
         prbs->seg.feature_mask[i] =
            (pic->slice_parameter.seg_param[i].alt_quant_enabled << 0) |
            (pic->slice_parameter.seg_param[i].alt_lf_enabled << 1) |
            (pic->slice_parameter.seg_param[i].segment_flags.segment_reference_enabled << 2) |
            (pic->slice_parameter.seg_param[i].segment_flags.segment_reference_skipped << 3);
      }

      for (i = 0; i < 7; ++i)
         prbs->seg.tree_probs[i] = pic->picture_parameter.mb_segment_tree_probs[i];

      for (i = 0; i < 3; ++i)
         prbs->seg.pred_probs[i] = pic->picture_parameter.segment_pred_probs[i];

      prbs->seg.abs_delta = pic->picture_parameter.abs_delta;
   } else
      memset(&prbs->seg, 0, 256);

   result.frame_header_flags = (pic->picture_parameter.pic_fields.frame_type
                                << RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_SHIFT) &
                               RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.error_resilient_mode
                                 << RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.intra_only
                                 << RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.allow_high_precision_mv
                                 << RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.frame_parallel_decoding_mode
                                 << RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.refresh_frame_context
                                 << RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_enabled
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_update_map
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_temporal_update
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_ref_delta_enabled
                                 << RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_ref_delta_update
                                 << RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_MASK;

   result.frame_header_flags |=
      ((dec->show_frame && !pic->picture_parameter.pic_fields.error_resilient_mode &&
        dec->last_width == dec->base.width && dec->last_height == dec->base.height)
       << RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_SHIFT) &
      RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_MASK;
   dec->show_frame = pic->picture_parameter.pic_fields.show_frame;

   result.frame_header_flags |=  (1 << RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_MASK;

   result.interp_filter = pic->picture_parameter.pic_fields.mcomp_filter_type;

   result.frame_context_idx = pic->picture_parameter.pic_fields.frame_context_idx;
   result.reset_frame_context = pic->picture_parameter.pic_fields.reset_frame_context;

   result.filter_level = pic->picture_parameter.filter_level;
   result.sharpness_level = pic->picture_parameter.sharpness_level;

   for (i = 0; i < 8; ++i)
      memcpy(result.lf_adj_level[i], pic->slice_parameter.seg_param[i].filter_level, 4 * 2);

   if (pic->picture_parameter.pic_fields.lossless_flag) {
      result.base_qindex = 0;
      result.y_dc_delta_q = 0;
      result.uv_ac_delta_q = 0;
      result.uv_dc_delta_q = 0;
   } else {
      result.base_qindex = pic->picture_parameter.base_qindex;
      result.y_dc_delta_q = pic->picture_parameter.y_dc_delta_q;
      result.uv_ac_delta_q = pic->picture_parameter.uv_ac_delta_q;
      result.uv_dc_delta_q = pic->picture_parameter.uv_dc_delta_q;
   }

   result.log2_tile_cols = pic->picture_parameter.log2_tile_columns;
   result.log2_tile_rows = pic->picture_parameter.log2_tile_rows;
   result.chroma_format = 1;
   result.bit_depth_luma_minus8 = result.bit_depth_chroma_minus8 =
      (pic->picture_parameter.bit_depth - 8);

   result.vp9_frame_size = align(dec->bs_size, 128);
   result.uncompressed_header_size = pic->picture_parameter.frame_header_length_in_bytes;
   result.compressed_header_size = pic->picture_parameter.first_partition_size;

   assert(dec->base.max_references + 1 <= ARRAY_SIZE(dec->render_pic_list));

   //clear the dec->render list if it is not used as a reference
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      if (dec->render_pic_list[i]) {
          for (j=0;j<8;j++) {
            if (dec->render_pic_list[i] == pic->ref[j])
                 break;
	  }
	  if(j == 8)
             dec->render_pic_list[i] = NULL;
      }
   }

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
      if (dec->render_pic_list[i] && dec->render_pic_list[i] == target) {
	if (target->codec != NULL){
	    result.curr_pic_idx =(uintptr_t)vl_video_buffer_get_associated_data(target, &dec->base);
	} else {
	    result.curr_pic_idx = i;
	    vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
						&radeon_dec_destroy_associated_data);
	}
	break;
      } else if (!dec->render_pic_list[i]) {
         dec->render_pic_list[i] = target;
         result.curr_pic_idx = i;
         vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
                                             &radeon_dec_destroy_associated_data);
         break;
      }
   }

   for (i = 0; i < 8; i++) {
      result.ref_frame_map[i] =
         (pic->ref[i]) ? (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base)
                       : 0x7f;
   }

   result.frame_refs[0] = result.ref_frame_map[pic->picture_parameter.pic_fields.last_ref_frame];
   result.ref_frame_sign_bias[0] = pic->picture_parameter.pic_fields.last_ref_frame_sign_bias;
   result.frame_refs[1] = result.ref_frame_map[pic->picture_parameter.pic_fields.golden_ref_frame];
   result.ref_frame_sign_bias[1] = pic->picture_parameter.pic_fields.golden_ref_frame_sign_bias;
   result.frame_refs[2] = result.ref_frame_map[pic->picture_parameter.pic_fields.alt_ref_frame];
   result.ref_frame_sign_bias[2] = pic->picture_parameter.pic_fields.alt_ref_frame_sign_bias;

   if (pic->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.p010_mode = 0;
         result.luma_10to8 = 1;
         result.chroma_10to8 = 1;
      }
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = (pic->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) ?
         CODEC_10_BITS : CODEC_8_BITS;
      dec->ref_codec.index = result.curr_pic_idx;
      dec->ref_codec.ref_size = 8;
      memset(dec->ref_codec.ref_list, 0x7f, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_frame_map, sizeof(result.ref_frame_map));
   }

   dec->last_width = dec->base.width;
   dec->last_height = dec->base.height;

   return result;
}

static void set_drm_keys(rvcn_dec_message_drm_t *drm, DECRYPT_PARAMETERS *decrypted)
{
   int cbc = decrypted->u.s.cbc;
   int ctr = decrypted->u.s.ctr;
   int id = decrypted->u.s.drm_id;
   int ekc = 1;
   int data1 = 1;
   int data2 = 1;

   drm->drm_cmd = 0;
   drm->drm_cntl = 0;

   drm->drm_cntl = 1 << DRM_CNTL_BYPASS_SHIFT;

   if (cbc || ctr) {
      drm->drm_cntl = 0 << DRM_CNTL_BYPASS_SHIFT;
      drm->drm_cmd |= 0xff << DRM_CMD_BYTE_MASK_SHIFT;

      if (ctr)
         drm->drm_cmd |= 0x00 << DRM_CMD_ALGORITHM_SHIFT;
      else if (cbc)
         drm->drm_cmd |= 0x02 << DRM_CMD_ALGORITHM_SHIFT;

      drm->drm_cmd |= 1 << DRM_CMD_GEN_MASK_SHIFT;
      drm->drm_cmd |= ekc << DRM_CMD_UNWRAP_KEY_SHIFT;
      drm->drm_cmd |= 0 << DRM_CMD_OFFSET_SHIFT;
      drm->drm_cmd |= data2 << DRM_CMD_CNT_DATA_SHIFT;
      drm->drm_cmd |= data1 << DRM_CMD_CNT_KEY_SHIFT;
      drm->drm_cmd |= ekc << DRM_CMD_KEY_SHIFT;
      drm->drm_cmd |= id << DRM_CMD_SESSION_SEL_SHIFT;

      if (ekc)
         memcpy(drm->drm_wrapped_key, decrypted->encrypted_key, 16);
      if (data1)
         memcpy(drm->drm_key, decrypted->session_iv, 16);
      if (data2)
         memcpy(drm->drm_counter, decrypted->encrypted_iv, 16);
      drm->drm_offset = 0;
   }
}

static rvcn_dec_message_av1_t get_av1_msg(struct radeon_decoder *dec,
                                          struct pipe_video_buffer *target,
                                          struct pipe_av1_picture_desc *pic)
{
   rvcn_dec_message_av1_t result;
   unsigned i, j;

   memset(&result, 0, sizeof(result));

   result.frame_header_flags = (pic->picture_parameter.pic_info_fields.show_frame
                                << RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.disable_cdf_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.pic_info_fields.disable_frame_end_update_cdf)
                                 << RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_MASK;

   result.frame_header_flags |= ((pic->picture_parameter.pic_info_fields.frame_type ==
                                 2 /* INTRA_ONLY_FRAME */) << RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_intrabc
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_high_precision_mv
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.mono_chrome
                                 << RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.skip_mode_present
                                 << RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_MASK;

   result.frame_header_flags |= (((pic->picture_parameter.qmatrix_fields.qm_y == 0xf) ? 0 : 1)
                                 << RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_filter_intra
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_intra_edge_filter
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_interintra_compound
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_masked_compound
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_warped_motion
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_dual_filter
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_order_hint
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_jnt_comp
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.use_ref_frame_mvs
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_screen_content_tools
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.force_integer_mv
                                 << RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled
                                 << RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.loop_filter_info_fields.mode_ref_delta_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_q_present_flag
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_lf_present_flag
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.reduced_tx_set_used
                                 << RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.enabled
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.update_map
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.temporal_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_lf_multi
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.is_motion_mode_switchable
                                 << RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.refresh_frame_flags)
                                 << RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.seq_info_fields.ref_frame_mvs)
                                 << RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_MASK;

   result.current_frame_id = pic->picture_parameter.current_frame_id;
   result.frame_offset = pic->picture_parameter.order_hint;

   result.profile = pic->picture_parameter.profile;
   result.is_annexb = 0;
   result.frame_type = pic->picture_parameter.pic_info_fields.frame_type;
   result.primary_ref_frame = pic->picture_parameter.primary_ref_frame;
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
      if (dec->render_pic_list[i] && dec->render_pic_list[i] == target) {
         result.curr_pic_idx = (uintptr_t)vl_video_buffer_get_associated_data(target, &dec->base);
         break;
      } else if (!dec->render_pic_list[i]) {
         dec->render_pic_list[i] = target;
         result.curr_pic_idx = dec->ref_idx;
         vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)dec->ref_idx++,
                                             &radeon_dec_destroy_associated_data);
         break;
      }
   }

   result.sb_size = pic->picture_parameter.seq_info_fields.use_128x128_superblock;
   result.interp_filter = pic->picture_parameter.interp_filter;
   for (i = 0; i < 2; ++i)
      result.filter_level[i] = pic->picture_parameter.filter_level[i];
   result.filter_level_u = pic->picture_parameter.filter_level_u;
   result.filter_level_v = pic->picture_parameter.filter_level_v;
   result.sharpness_level = pic->picture_parameter.loop_filter_info_fields.sharpness_level;
   for (i = 0; i < 8; ++i)
      result.ref_deltas[i] = pic->picture_parameter.ref_deltas[i];
   for (i = 0; i < 2; ++i)
      result.mode_deltas[i] = pic->picture_parameter.mode_deltas[i];
   result.base_qindex = pic->picture_parameter.base_qindex;
   result.y_dc_delta_q = pic->picture_parameter.y_dc_delta_q;
   result.u_dc_delta_q = pic->picture_parameter.u_dc_delta_q;
   result.v_dc_delta_q = pic->picture_parameter.v_dc_delta_q;
   result.u_ac_delta_q = pic->picture_parameter.u_ac_delta_q;
   result.v_ac_delta_q = pic->picture_parameter.v_ac_delta_q;
   result.qm_y = pic->picture_parameter.qmatrix_fields.qm_y | 0xf0;
   result.qm_u = pic->picture_parameter.qmatrix_fields.qm_u | 0xf0;
   result.qm_v = pic->picture_parameter.qmatrix_fields.qm_v | 0xf0;
   result.delta_q_res = 1 << pic->picture_parameter.mode_control_fields.log2_delta_q_res;
   result.delta_lf_res = 1 << pic->picture_parameter.mode_control_fields.log2_delta_lf_res;

   result.tile_cols = pic->picture_parameter.tile_cols;
   result.tile_rows = pic->picture_parameter.tile_rows;
   result.tx_mode = pic->picture_parameter.mode_control_fields.tx_mode;
   result.reference_mode = (pic->picture_parameter.mode_control_fields.reference_select == 1) ? 2 : 0;
   result.chroma_format = pic->picture_parameter.seq_info_fields.mono_chrome ? 0 : 1;
   result.tile_size_bytes = 0xff;
   result.context_update_tile_id = pic->picture_parameter.context_update_tile_id;
   for (i = 0; i < 65; ++i) {
      result.tile_col_start_sb[i] = pic->picture_parameter.tile_col_start_sb[i];
      result.tile_row_start_sb[i] = pic->picture_parameter.tile_row_start_sb[i];
   }
   result.max_width = pic->picture_parameter.max_width;
   result.max_height = pic->picture_parameter.max_height;
   if (pic->picture_parameter.pic_info_fields.use_superres) {
      result.width = (pic->picture_parameter.frame_width * 8 + pic->picture_parameter.superres_scale_denominator / 2) /
         pic->picture_parameter.superres_scale_denominator;
      result.superres_scale_denominator = pic->picture_parameter.superres_scale_denominator;
   } else {
      result.width = pic->picture_parameter.frame_width;
      result.superres_scale_denominator = pic->picture_parameter.superres_scale_denominator;
   }
   result.height = pic->picture_parameter.frame_height;
   result.superres_upscaled_width = pic->picture_parameter.frame_width;
   result.order_hint_bits = pic->picture_parameter.order_hint_bits_minus_1 + 1;

   for (i = 0; i < NUM_AV1_REFS; ++i) {
      result.ref_frame_map[i] =
         (pic->ref[i]) ? (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base)
                       : 0x7f;
   }
   for (i = 0; i < NUM_AV1_REFS_PER_FRAME; ++i)
       result.frame_refs[i] = result.ref_frame_map[pic->picture_parameter.ref_frame_idx[i]];

   result.bit_depth_luma_minus8 = result.bit_depth_chroma_minus8 = pic->picture_parameter.bit_depth_idx << 1;

   for (i = 0; i < 8; ++i) {
      for (j = 0; j < 8; ++j)
         result.feature_data[i][j] = pic->picture_parameter.seg_info.feature_data[i][j];
      result.feature_mask[i] = pic->picture_parameter.seg_info.feature_mask[i];
   }
   memcpy(dec->probs, &pic->picture_parameter.seg_info.feature_data, 128);
   memcpy((dec->probs + 128), &pic->picture_parameter.seg_info.feature_mask, 8);

   result.cdef_damping = pic->picture_parameter.cdef_damping_minus_3 + 3;
   result.cdef_bits = pic->picture_parameter.cdef_bits;
   for (i = 0; i < 8; ++i) {
      result.cdef_strengths[i] = pic->picture_parameter.cdef_y_strengths[i];
      result.cdef_uv_strengths[i] = pic->picture_parameter.cdef_uv_strengths[i];
   }
   result.frame_restoration_type[0] = pic->picture_parameter.loop_restoration_fields.yframe_restoration_type;
   result.frame_restoration_type[1] = pic->picture_parameter.loop_restoration_fields.cbframe_restoration_type;
   result.frame_restoration_type[2] = pic->picture_parameter.loop_restoration_fields.crframe_restoration_type;
   for (i = 0; i < 3; ++i) {
      int log2_num = 0;
      int unit_size = pic->picture_parameter.lr_unit_size[i];
      if (unit_size) {
         while (unit_size >>= 1)
            log2_num++;
         result.log2_restoration_unit_size_minus5[i] = log2_num - 5;
      } else {
         result.log2_restoration_unit_size_minus5[i] = 0;
      }
   }

   if (pic->picture_parameter.bit_depth_idx) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.luma_10to8 = 1;
         result.chroma_10to8 = 1;
      }
   }

   result.preskip_segid = 0;
   result.last_active_segid = 0;
   for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++) {
         if (pic->picture_parameter.seg_info.feature_mask[i] & (1 << j)) {
            result.last_active_segid = i;
            if (j >= 5)
               result.preskip_segid = 1;
         }
      }
   }

   result.seg_lossless_flag = 0;
   for (i = 0; i < 8; ++i) {
      int av1_get_qindex, qindex;
      int segfeature_active = pic->picture_parameter.seg_info.feature_mask[i] & (1 << 0);
      if (segfeature_active) {
         int seg_qindex = pic->picture_parameter.base_qindex +
                          pic->picture_parameter.seg_info.feature_data[i][0];
         av1_get_qindex = seg_qindex < 0 ? 0 : (seg_qindex > 255 ? 255 : seg_qindex);
      } else {
         av1_get_qindex = pic->picture_parameter.base_qindex;
      }
      qindex = pic->picture_parameter.seg_info.segment_info_fields.enabled ?
               av1_get_qindex :
               pic->picture_parameter.base_qindex;
      result.seg_lossless_flag |= (((qindex == 0) && result.y_dc_delta_q == 0 &&
                                    result.u_dc_delta_q == 0 && result.v_dc_delta_q == 0 &&
                                    result.u_ac_delta_q == 0 && result.v_ac_delta_q == 0) << i);
   }

   rvcn_dec_film_grain_params_t* fg_params = &result.film_grain;
   fg_params->apply_grain = pic->picture_parameter.film_grain_info.film_grain_info_fields.apply_grain;
   if (fg_params->apply_grain) {
      fg_params->random_seed = pic->picture_parameter.film_grain_info.grain_seed;
      fg_params->grain_scale_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.grain_scale_shift;
      fg_params->scaling_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.grain_scaling_minus_8 + 8;
      fg_params->chroma_scaling_from_luma =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.chroma_scaling_from_luma;
      fg_params->num_y_points = pic->picture_parameter.film_grain_info.num_y_points;
      fg_params->num_cb_points = pic->picture_parameter.film_grain_info.num_cb_points;
      fg_params->num_cr_points = pic->picture_parameter.film_grain_info.num_cr_points;
      fg_params->cb_mult = pic->picture_parameter.film_grain_info.cb_mult;
      fg_params->cb_luma_mult = pic->picture_parameter.film_grain_info.cb_luma_mult;
      fg_params->cb_offset = pic->picture_parameter.film_grain_info.cb_offset;
      fg_params->cr_mult = pic->picture_parameter.film_grain_info.cr_mult;
      fg_params->cr_luma_mult = pic->picture_parameter.film_grain_info.cr_luma_mult;
      fg_params->cr_offset = pic->picture_parameter.film_grain_info.cr_offset;
      fg_params->bit_depth_minus_8 = pic->picture_parameter.bit_depth_idx << 1;

      for (i = 0; i < fg_params->num_y_points; ++i) {
         fg_params->scaling_points_y[i][0] = pic->picture_parameter.film_grain_info.point_y_value[i];
         fg_params->scaling_points_y[i][1] = pic->picture_parameter.film_grain_info.point_y_scaling[i];
      }
      for (i = 0; i < fg_params->num_cb_points; ++i) {
         fg_params->scaling_points_cb[i][0] = pic->picture_parameter.film_grain_info.point_cb_value[i];
         fg_params->scaling_points_cb[i][1] = pic->picture_parameter.film_grain_info.point_cb_scaling[i];
      }
      for (i = 0; i < fg_params->num_cr_points; ++i) {
         fg_params->scaling_points_cr[i][0] = pic->picture_parameter.film_grain_info.point_cr_value[i];
         fg_params->scaling_points_cr[i][1] = pic->picture_parameter.film_grain_info.point_cr_scaling[i];
      }

      fg_params->ar_coeff_lag = pic->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_lag;
      fg_params->ar_coeff_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_shift_minus_6 + 6;

      for (i = 0; i < 24; ++i)
         fg_params->ar_coeffs_y[i] = pic->picture_parameter.film_grain_info.ar_coeffs_y[i];

      for (i = 0; i < 25; ++i) {
         fg_params->ar_coeffs_cb[i] = pic->picture_parameter.film_grain_info.ar_coeffs_cb[i];
         fg_params->ar_coeffs_cr[i] = pic->picture_parameter.film_grain_info.ar_coeffs_cr[i];
      }

      fg_params->overlap_flag = pic->picture_parameter.film_grain_info.film_grain_info_fields.overlap_flag;
      fg_params->clip_to_restricted_range =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.clip_to_restricted_range;
   }

   result.uncompressed_header_size = 0;
   for (i = 0; i < 7; ++i) {
      result.global_motion[i + 1].wmtype = (rvcn_dec_transformation_type_e)pic->picture_parameter.wm[i].wmtype;
      for (j = 0; j < 6; ++j)
         result.global_motion[i + 1].wmmat[j] = pic->picture_parameter.wm[i].wmmat[j];
   }
   for (i = 0; i < 256; ++i) {
      result.tile_info[i].offset = pic->slice_parameter.slice_data_offset[i];
      result.tile_info[i].size = pic->slice_parameter.slice_data_size[i];
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = pic->picture_parameter.bit_depth_idx ? CODEC_10_BITS : CODEC_8_BITS;
      dec->ref_codec.index = result.curr_pic_idx;
      dec->ref_codec.ref_size = 8;
      memset(dec->ref_codec.ref_list, 0x7f, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_frame_map, sizeof(result.ref_frame_map));
   }

   return result;
}

static void rvcn_init_mode_probs(void *prob)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;
   int i;

   memcpy(fc->palette_y_size_cdf, default_palette_y_size_cdf, sizeof(default_palette_y_size_cdf));
   memcpy(fc->palette_uv_size_cdf, default_palette_uv_size_cdf, sizeof(default_palette_uv_size_cdf));
   memcpy(fc->palette_y_color_index_cdf, default_palette_y_color_index_cdf, sizeof(default_palette_y_color_index_cdf));
   memcpy(fc->palette_uv_color_index_cdf, default_palette_uv_color_index_cdf, sizeof(default_palette_uv_color_index_cdf));
   memcpy(fc->kf_y_cdf, default_kf_y_mode_cdf, sizeof(default_kf_y_mode_cdf));
   memcpy(fc->angle_delta_cdf, default_angle_delta_cdf, sizeof(default_angle_delta_cdf));
   memcpy(fc->comp_inter_cdf, default_comp_inter_cdf, sizeof(default_comp_inter_cdf));
   memcpy(fc->comp_ref_type_cdf, default_comp_ref_type_cdf,sizeof(default_comp_ref_type_cdf));
   memcpy(fc->uni_comp_ref_cdf, default_uni_comp_ref_cdf, sizeof(default_uni_comp_ref_cdf));
   memcpy(fc->palette_y_mode_cdf, default_palette_y_mode_cdf, sizeof(default_palette_y_mode_cdf));
   memcpy(fc->palette_uv_mode_cdf, default_palette_uv_mode_cdf, sizeof(default_palette_uv_mode_cdf));
   memcpy(fc->comp_ref_cdf, default_comp_ref_cdf, sizeof(default_comp_ref_cdf));
   memcpy(fc->comp_bwdref_cdf, default_comp_bwdref_cdf, sizeof(default_comp_bwdref_cdf));
   memcpy(fc->single_ref_cdf, default_single_ref_cdf, sizeof(default_single_ref_cdf));
   memcpy(fc->txfm_partition_cdf, default_txfm_partition_cdf, sizeof(default_txfm_partition_cdf));
   memcpy(fc->compound_index_cdf, default_compound_idx_cdfs, sizeof(default_compound_idx_cdfs));
   memcpy(fc->comp_group_idx_cdf, default_comp_group_idx_cdfs, sizeof(default_comp_group_idx_cdfs));
   memcpy(fc->newmv_cdf, default_newmv_cdf, sizeof(default_newmv_cdf));
   memcpy(fc->zeromv_cdf, default_zeromv_cdf, sizeof(default_zeromv_cdf));
   memcpy(fc->refmv_cdf, default_refmv_cdf, sizeof(default_refmv_cdf));
   memcpy(fc->drl_cdf, default_drl_cdf, sizeof(default_drl_cdf));
   memcpy(fc->motion_mode_cdf, default_motion_mode_cdf, sizeof(default_motion_mode_cdf));
   memcpy(fc->obmc_cdf, default_obmc_cdf, sizeof(default_obmc_cdf));
   memcpy(fc->inter_compound_mode_cdf, default_inter_compound_mode_cdf, sizeof(default_inter_compound_mode_cdf));
   memcpy(fc->compound_type_cdf, default_compound_type_cdf, sizeof(default_compound_type_cdf));
   memcpy(fc->wedge_idx_cdf, default_wedge_idx_cdf, sizeof(default_wedge_idx_cdf));
   memcpy(fc->interintra_cdf, default_interintra_cdf, sizeof(default_interintra_cdf));
   memcpy(fc->wedge_interintra_cdf, default_wedge_interintra_cdf, sizeof(default_wedge_interintra_cdf));
   memcpy(fc->interintra_mode_cdf, default_interintra_mode_cdf, sizeof(default_interintra_mode_cdf));
   memcpy(fc->pred_cdf, default_segment_pred_cdf, sizeof(default_segment_pred_cdf));
   memcpy(fc->switchable_restore_cdf, default_switchable_restore_cdf, sizeof(default_switchable_restore_cdf));
   memcpy(fc->wiener_restore_cdf, default_wiener_restore_cdf, sizeof(default_wiener_restore_cdf));
   memcpy(fc->sgrproj_restore_cdf, default_sgrproj_restore_cdf, sizeof(default_sgrproj_restore_cdf));
   memcpy(fc->y_mode_cdf, default_if_y_mode_cdf, sizeof(default_if_y_mode_cdf));
   memcpy(fc->uv_mode_cdf, default_uv_mode_cdf, sizeof(default_uv_mode_cdf));
   memcpy(fc->switchable_interp_cdf, default_switchable_interp_cdf, sizeof(default_switchable_interp_cdf));
   memcpy(fc->partition_cdf, default_partition_cdf, sizeof(default_partition_cdf));
   memcpy(fc->intra_ext_tx_cdf, default_intra_ext_tx_cdf, sizeof(default_intra_ext_tx_cdf));
   memcpy(fc->inter_ext_tx_cdf, default_inter_ext_tx_cdf, sizeof(default_inter_ext_tx_cdf));
   memcpy(fc->skip_cdfs, default_skip_cdfs, sizeof(default_skip_cdfs));
   memcpy(fc->intra_inter_cdf, default_intra_inter_cdf, sizeof(default_intra_inter_cdf));
   memcpy(fc->tree_cdf, default_seg_tree_cdf, sizeof(default_seg_tree_cdf));
   for (i = 0; i < SPATIAL_PREDICTION_PROBS; ++i)
      memcpy(fc->spatial_pred_seg_cdf[i], default_spatial_pred_seg_tree_cdf[i], sizeof(default_spatial_pred_seg_tree_cdf[i]));
   memcpy(fc->tx_size_cdf, default_tx_size_cdf, sizeof(default_tx_size_cdf));
   memcpy(fc->delta_q_cdf, default_delta_q_cdf, sizeof(default_delta_q_cdf));
   memcpy(fc->skip_mode_cdfs, default_skip_mode_cdfs, sizeof(default_skip_mode_cdfs));
   memcpy(fc->delta_lf_cdf, default_delta_lf_cdf, sizeof(default_delta_lf_cdf));
   memcpy(fc->delta_lf_multi_cdf, default_delta_lf_multi_cdf, sizeof(default_delta_lf_multi_cdf));
   memcpy(fc->cfl_sign_cdf, default_cfl_sign_cdf, sizeof(default_cfl_sign_cdf));
   memcpy(fc->cfl_alpha_cdf, default_cfl_alpha_cdf, sizeof(default_cfl_alpha_cdf));
   memcpy(fc->filter_intra_cdfs, default_filter_intra_cdfs, sizeof(default_filter_intra_cdfs));
   memcpy(fc->filter_intra_mode_cdf, default_filter_intra_mode_cdf, sizeof(default_filter_intra_mode_cdf));
   memcpy(fc->intrabc_cdf, default_intrabc_cdf, sizeof(default_intrabc_cdf));
}

static void rvcn_av1_init_mv_probs(void *prob)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;

   memcpy(fc->nmvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->nmvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->nmvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->nmvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->nmvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->nmvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->nmvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->nmvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->nmvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->nmvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->nmvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->nmvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->nmvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->nmvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->nmvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->nmvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->nmvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
   memcpy(fc->ndvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->ndvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->ndvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->ndvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->ndvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->ndvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->ndvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->ndvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->ndvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->ndvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->ndvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->ndvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->ndvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->ndvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->ndvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->ndvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->ndvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
}

static void rvcn_av1_default_coef_probs(void *prob, int index)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;

   memcpy(fc->txb_skip_cdf, av1_default_txb_skip_cdfs[index], sizeof(av1_default_txb_skip_cdfs[index]));
   memcpy(fc->eob_extra_cdf, av1_default_eob_extra_cdfs[index], sizeof(av1_default_eob_extra_cdfs[index]));
   memcpy(fc->dc_sign_cdf, av1_default_dc_sign_cdfs[index], sizeof(av1_default_dc_sign_cdfs[index]));
   memcpy(fc->coeff_br_cdf, av1_default_coeff_lps_multi_cdfs[index], sizeof(av1_default_coeff_lps_multi_cdfs[index]));
   memcpy(fc->coeff_base_cdf, av1_default_coeff_base_multi_cdfs[index], sizeof(av1_default_coeff_base_multi_cdfs[index]));
   memcpy(fc->coeff_base_eob_cdf, av1_default_coeff_base_eob_multi_cdfs[index], sizeof(av1_default_coeff_base_eob_multi_cdfs[index]));
   memcpy(fc->eob_flag_cdf16, av1_default_eob_multi16_cdfs[index], sizeof(av1_default_eob_multi16_cdfs[index]));
   memcpy(fc->eob_flag_cdf32, av1_default_eob_multi32_cdfs[index], sizeof(av1_default_eob_multi32_cdfs[index]));
   memcpy(fc->eob_flag_cdf64, av1_default_eob_multi64_cdfs[index], sizeof(av1_default_eob_multi64_cdfs[index]));
   memcpy(fc->eob_flag_cdf128, av1_default_eob_multi128_cdfs[index], sizeof(av1_default_eob_multi128_cdfs[index]));
   memcpy(fc->eob_flag_cdf256, av1_default_eob_multi256_cdfs[index], sizeof(av1_default_eob_multi256_cdfs[index]));
   memcpy(fc->eob_flag_cdf512, av1_default_eob_multi512_cdfs[index], sizeof(av1_default_eob_multi512_cdfs[index]));
   memcpy(fc->eob_flag_cdf1024, av1_default_eob_multi1024_cdfs[index], sizeof(av1_default_eob_multi1024_cdfs[index]));
}

static unsigned calc_ctx_size_h265_main(struct radeon_decoder *dec)
{
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   unsigned max_references = dec->base.max_references + 1;

   if (dec->base.width * dec->base.height >= 4096 * 2000)
      max_references = MAX2(max_references, 8);
   else
      max_references = MAX2(max_references, 17);

   width = align(width, 16);
   height = align(height, 16);
   return ((width + 255) / 16) * ((height + 255) / 16) * 16 * max_references + 52 * 1024;
}

static unsigned calc_ctx_size_h265_main10(struct radeon_decoder *dec,
                                          struct pipe_h265_picture_desc *pic)
{
   unsigned log2_ctb_size, width_in_ctb, height_in_ctb, num_16x16_block_per_ctb;
   unsigned context_buffer_size_per_ctb_row, cm_buffer_size, max_mb_address, db_left_tile_pxl_size;
   unsigned db_left_tile_ctx_size = 4096 / 16 * (32 + 16 * 4);

   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);
   unsigned coeff_10bit =
      (pic->pps->sps->bit_depth_luma_minus8 || pic->pps->sps->bit_depth_chroma_minus8) ? 2 : 1;

   unsigned max_references = dec->base.max_references + 1;

   if (dec->base.width * dec->base.height >= 4096 * 2000)
      max_references = MAX2(max_references, 8);
   else
      max_references = MAX2(max_references, 17);

   log2_ctb_size = pic->pps->sps->log2_min_luma_coding_block_size_minus3 + 3 +
                   pic->pps->sps->log2_diff_max_min_luma_coding_block_size;

   width_in_ctb = (width + ((1 << log2_ctb_size) - 1)) >> log2_ctb_size;
   height_in_ctb = (height + ((1 << log2_ctb_size) - 1)) >> log2_ctb_size;

   num_16x16_block_per_ctb = ((1 << log2_ctb_size) >> 4) * ((1 << log2_ctb_size) >> 4);
   context_buffer_size_per_ctb_row = align(width_in_ctb * num_16x16_block_per_ctb * 16, 256);
   max_mb_address = (unsigned)ceil(height * 8 / 2048.0);

   cm_buffer_size = max_references * context_buffer_size_per_ctb_row * height_in_ctb;
   db_left_tile_pxl_size = coeff_10bit * (max_mb_address * 2 * 2048 + 1024);

   return cm_buffer_size + db_left_tile_ctx_size + db_left_tile_pxl_size;
}

static rvcn_dec_message_vc1_t get_vc1_msg(struct pipe_vc1_picture_desc *pic)
{
   rvcn_dec_message_vc1_t result;

   memset(&result, 0, sizeof(result));
   switch (pic->base.profile) {
   case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
      result.profile = RDECODE_VC1_PROFILE_SIMPLE;
      result.level = 1;
      break;

   case PIPE_VIDEO_PROFILE_VC1_MAIN:
      result.profile = RDECODE_VC1_PROFILE_MAIN;
      result.level = 2;
      break;

   case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
      result.profile = RDECODE_VC1_PROFILE_ADVANCED;
      result.level = 4;
      break;

   default:
      assert(0);
   }

   result.sps_info_flags |= pic->postprocflag << 7;
   result.sps_info_flags |= pic->pulldown << 6;
   result.sps_info_flags |= pic->interlace << 5;
   result.sps_info_flags |= pic->tfcntrflag << 4;
   result.sps_info_flags |= pic->finterpflag << 3;
   result.sps_info_flags |= pic->psf << 1;

   result.pps_info_flags |= pic->range_mapy_flag << 31;
   result.pps_info_flags |= pic->range_mapy << 28;
   result.pps_info_flags |= pic->range_mapuv_flag << 27;
   result.pps_info_flags |= pic->range_mapuv << 24;
   result.pps_info_flags |= pic->multires << 21;
   result.pps_info_flags |= pic->maxbframes << 16;
   result.pps_info_flags |= pic->overlap << 11;
   result.pps_info_flags |= pic->quantizer << 9;
   result.pps_info_flags |= pic->panscan_flag << 7;
   result.pps_info_flags |= pic->refdist_flag << 6;
   result.pps_info_flags |= pic->vstransform << 0;

   if (pic->base.profile != PIPE_VIDEO_PROFILE_VC1_SIMPLE) {
      result.pps_info_flags |= pic->syncmarker << 20;
      result.pps_info_flags |= pic->rangered << 19;
      result.pps_info_flags |= pic->loopfilter << 5;
      result.pps_info_flags |= pic->fastuvmc << 4;
      result.pps_info_flags |= pic->extended_mv << 3;
      result.pps_info_flags |= pic->extended_dmv << 8;
      result.pps_info_flags |= pic->dquant << 1;
   }

   result.chroma_format = 1;

   return result;
}

static uint32_t get_ref_pic_idx(struct radeon_decoder *dec, struct pipe_video_buffer *ref)
{
   uint32_t min = MAX2(dec->frame_number, NUM_MPEG2_REFS) - NUM_MPEG2_REFS;
   uint32_t max = MAX2(dec->frame_number, 1) - 1;
   uintptr_t frame;

   /* seems to be the most sane fallback */
   if (!ref)
      return max;

   /* get the frame number from the associated data */
   frame = (uintptr_t)vl_video_buffer_get_associated_data(ref, &dec->base);

   /* limit the frame number to a valid range */
   return MAX2(MIN2(frame, max), min);
}

static rvcn_dec_message_mpeg2_vld_t get_mpeg2_msg(struct radeon_decoder *dec,
                                                  struct pipe_mpeg12_picture_desc *pic)
{
   const int *zscan = pic->alternate_scan ? vl_zscan_alternate : vl_zscan_normal;
   rvcn_dec_message_mpeg2_vld_t result;
   unsigned i;

   memset(&result, 0, sizeof(result));
   result.decoded_pic_idx = dec->frame_number;

   result.forward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[0]);
   result.backward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[1]);

   if (pic->intra_matrix) {
      result.load_intra_quantiser_matrix = 1;
      for (i = 0; i < 64; ++i) {
         result.intra_quantiser_matrix[i] = pic->intra_matrix[zscan[i]];
      }
   }
   if (pic->non_intra_matrix) {
      result.load_nonintra_quantiser_matrix = 1;
      for (i = 0; i < 64; ++i) {
         result.nonintra_quantiser_matrix[i] = pic->non_intra_matrix[zscan[i]];
      }
   }

   result.profile_and_level_indication = 0;
   result.chroma_format = 0x1;

   result.picture_coding_type = pic->picture_coding_type;
   result.f_code[0][0] = pic->f_code[0][0] + 1;
   result.f_code[0][1] = pic->f_code[0][1] + 1;
   result.f_code[1][0] = pic->f_code[1][0] + 1;
   result.f_code[1][1] = pic->f_code[1][1] + 1;
   result.intra_dc_precision = pic->intra_dc_precision;
   result.pic_structure = pic->picture_structure;
   result.top_field_first = pic->top_field_first;
   result.frame_pred_frame_dct = pic->frame_pred_frame_dct;
   result.concealment_motion_vectors = pic->concealment_motion_vectors;
   result.q_scale_type = pic->q_scale_type;
   result.intra_vlc_format = pic->intra_vlc_format;
   result.alternate_scan = pic->alternate_scan;

   return result;
}

static rvcn_dec_message_mpeg4_asp_vld_t get_mpeg4_msg(struct radeon_decoder *dec,
                                                      struct pipe_mpeg4_picture_desc *pic)
{
   rvcn_dec_message_mpeg4_asp_vld_t result;
   unsigned i;

   memset(&result, 0, sizeof(result));
   result.decoded_pic_idx = dec->frame_number;

   result.forward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[0]);
   result.backward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[1]);

   result.variant_type = 0;
   result.profile_and_level_indication = 0xF0;

   result.video_object_layer_verid = 0x5;
   result.video_object_layer_shape = 0x0;

   result.video_object_layer_width = dec->base.width;
   result.video_object_layer_height = dec->base.height;

   result.vop_time_increment_resolution = pic->vop_time_increment_resolution;

   result.short_video_header = pic->short_video_header;
   result.interlaced = pic->interlaced;
   result.load_intra_quant_mat = 1;
   result.load_nonintra_quant_mat = 1;
   result.quarter_sample = pic->quarter_sample;
   result.complexity_estimation_disable = 1;
   result.resync_marker_disable = pic->resync_marker_disable;
   result.newpred_enable = 0;
   result.reduced_resolution_vop_enable = 0;

   result.quant_type = pic->quant_type;

   for (i = 0; i < 64; ++i) {
      result.intra_quant_mat[i] = pic->intra_matrix[vl_zscan_normal[i]];
      result.nonintra_quant_mat[i] = pic->non_intra_matrix[vl_zscan_normal[i]];
   }

   return result;
}

static void rvcn_dec_message_create(struct radeon_decoder *dec)
{
   rvcn_dec_message_header_t *header = dec->msg;
   rvcn_dec_message_create_t *create = dec->msg + sizeof(rvcn_dec_message_header_t);
   unsigned sizes = sizeof(rvcn_dec_message_header_t) + sizeof(rvcn_dec_message_create_t);

   memset(dec->msg, 0, sizes);
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizes;
   header->num_buffers = 1;
   header->msg_type = RDECODE_MSG_CREATE;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = 0;

   header->index[0].message_id = RDECODE_MESSAGE_CREATE;
   header->index[0].offset = sizeof(rvcn_dec_message_header_t);
   header->index[0].size = sizeof(rvcn_dec_message_create_t);
   header->index[0].filled = 0;

   create->stream_type = dec->stream_type;
   create->session_flags = 0;
   create->width_in_samples = dec->base.width;
   create->height_in_samples = dec->base.height;
}

static unsigned rvcn_dec_dynamic_dpb_t2_message(struct radeon_decoder *dec, rvcn_dec_message_decode_t *decode,
      rvcn_dec_message_dynamic_dpb_t2_t *dynamic_dpb_t2)
{
   struct rvcn_dec_dynamic_dpb_t2 *dpb = NULL, *dummy = NULL;
   unsigned width, height, size;
   uint64_t addr;
   int i;

   width = align(decode->width_in_samples, dec->db_alignment);
   height = align(decode->height_in_samples, dec->db_alignment);
   size = align((width * height * 3) / 2, 256);
   if (dec->ref_codec.bts == CODEC_10_BITS)
      size = size * 3 / 2;

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
      for (i = 0; i < dec->ref_codec.ref_size; ++i) {
         if ((dec->ref_codec.ref_list[i] != 0x7f) && (d->index == (dec->ref_codec.ref_list[i] & 0x7f))) {
            if (!dummy)
               dummy = d;

            addr = dec->ws->buffer_get_virtual_address(d->dpb.res->buf);
            if (!addr && dummy) {
               RVID_ERR("Ref list from application is incorrect, using dummy buffer instead.\n");
               addr = dec->ws->buffer_get_virtual_address(dummy->dpb.res->buf);
            }
            dynamic_dpb_t2->dpbAddrLo[i] = addr;
            dynamic_dpb_t2->dpbAddrHi[i] = addr >> 32;
            ++dynamic_dpb_t2->dpbArraySize;
            break;
         }
      }
      if (i == dec->ref_codec.ref_size) {
         if (d->dpb.res->b.b.width0 * d->dpb.res->b.b.height0 != size) {
            list_del(&d->list);
            list_addtail(&d->list, &dec->dpb_unref_list);
         } else {
            d->index = 0x7f;
         }
      }
   }

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
      if (d->dpb.res->b.b.width0 * d->dpb.res->b.b.height0 == size && d->index == dec->ref_codec.index) {
         dpb = d;
         break;
      }
   }

   if (!dpb) {
      list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
         if (d->index == 0x7f) {
            d->index = dec->ref_codec.index;
            dpb = d;
            break;
         }
      }
   }

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_unref_list, list) {
      list_del(&d->list);
      si_vid_destroy_buffer(&d->dpb);
      FREE(d);
   }

   if (!dpb) {
      dpb = CALLOC_STRUCT(rvcn_dec_dynamic_dpb_t2);
      if (!dpb)
         return 1;
      dpb->index = dec->ref_codec.index;
      if (!si_vid_create_buffer(dec->screen, &dpb->dpb, size, PIPE_USAGE_DEFAULT)) {
         RVID_ERR("Can't allocated dpb buffer.\n");
         FREE(dpb);
         return 1;
      }
      list_addtail(&dpb->list, &dec->dpb_ref_list);
   }

   dec->ws->cs_add_buffer(&dec->cs, dpb->dpb.res->buf,
      RADEON_USAGE_READWRITE | RADEON_USAGE_SYNCHRONIZED, RADEON_DOMAIN_VRAM, 0);
   addr = dec->ws->buffer_get_virtual_address(dpb->dpb.res->buf);
   dynamic_dpb_t2->dpbCurrLo = addr;
   dynamic_dpb_t2->dpbCurrHi = addr >> 32;

   decode->decode_flags = 1;
   dynamic_dpb_t2->dpbConfigFlags = 0;
   dynamic_dpb_t2->dpbLumaPitch = align(decode->width_in_samples, dec->db_alignment);
   dynamic_dpb_t2->dpbLumaAlignedHeight = align(decode->height_in_samples, dec->db_alignment);
   dynamic_dpb_t2->dpbLumaAlignedSize = dynamic_dpb_t2->dpbLumaPitch *
      dynamic_dpb_t2->dpbLumaAlignedHeight;
   dynamic_dpb_t2->dpbChromaPitch = dynamic_dpb_t2->dpbLumaPitch >> 1;
   dynamic_dpb_t2->dpbChromaAlignedHeight = dynamic_dpb_t2->dpbLumaAlignedHeight >> 1;
   dynamic_dpb_t2->dpbChromaAlignedSize = dynamic_dpb_t2->dpbChromaPitch *
      dynamic_dpb_t2->dpbChromaAlignedHeight * 2;

   if (dec->ref_codec.bts == CODEC_10_BITS) {
      dynamic_dpb_t2->dpbLumaAlignedSize = dynamic_dpb_t2->dpbLumaAlignedSize * 3 / 2;
      dynamic_dpb_t2->dpbChromaAlignedSize = dynamic_dpb_t2->dpbChromaAlignedSize * 3 / 2;
   }

   return 0;
}

static struct pb_buffer *rvcn_dec_message_decode(struct radeon_decoder *dec,
                                                 struct pipe_video_buffer *target,
                                                 struct pipe_picture_desc *picture)
{
   DECRYPT_PARAMETERS *decrypt = (DECRYPT_PARAMETERS *)picture->decrypt_key;
   bool encrypted = (DECRYPT_PARAMETERS *)picture->protected_playback;
   struct si_texture *luma = (struct si_texture *)((struct vl_video_buffer *)target)->resources[0];
   struct si_texture *chroma =
      (struct si_texture *)((struct vl_video_buffer *)target)->resources[1];
   ASSERTED struct si_screen *sscreen = (struct si_screen *)dec->screen;
   rvcn_dec_message_header_t *header;
   rvcn_dec_message_index_t *index_codec;
   rvcn_dec_message_index_t *index_drm = NULL;
   rvcn_dec_message_index_t *index_dynamic_dpb = NULL;
   rvcn_dec_message_decode_t *decode;
   unsigned sizes = 0, offset_decode, offset_codec;
   unsigned offset_drm = 0, offset_dynamic_dpb = 0;
   void *codec;
   rvcn_dec_message_drm_t *drm = NULL;
   rvcn_dec_message_dynamic_dpb_t *dynamic_dpb = NULL;
   rvcn_dec_message_dynamic_dpb_t2_t *dynamic_dpb_t2 = NULL;

   header = dec->msg;
   sizes += sizeof(rvcn_dec_message_header_t);

   index_codec = (void*)header + sizes;
   sizes += sizeof(rvcn_dec_message_index_t);

   if (encrypted) {
      index_drm = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_index_t);
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      index_dynamic_dpb = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_index_t);
   }

   offset_decode = sizes;
   decode = (void*)header + sizes;
   sizes += sizeof(rvcn_dec_message_decode_t);

   if (encrypted) {
      offset_drm = sizes;
      drm = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_drm_t);
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      offset_dynamic_dpb = sizes;
      if (dec->dpb_type == DPB_DYNAMIC_TIER_1) {
         dynamic_dpb = (void*)header + sizes;
         sizes += sizeof(rvcn_dec_message_dynamic_dpb_t);
      }
      else if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
         dynamic_dpb_t2 = (void*)header + sizes;
         sizes += sizeof(rvcn_dec_message_dynamic_dpb_t2_t);
      }
   }

   offset_codec = sizes;
   codec = (void*)header + sizes;

   memset(dec->msg, 0, sizes);
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizes;
   header->msg_type = RDECODE_MSG_DECODE;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = dec->frame_number;

   header->index[0].message_id = RDECODE_MESSAGE_DECODE;
   header->index[0].offset = offset_decode;
   header->index[0].size = sizeof(rvcn_dec_message_decode_t);
   header->index[0].filled = 0;
   header->num_buffers = 1;

   index_codec->offset = offset_codec;
   index_codec->size = sizeof(rvcn_dec_message_avc_t);
   index_codec->filled = 0;
   ++header->num_buffers;

   if (encrypted) {
      index_drm->message_id = RDECODE_MESSAGE_DRM;
      index_drm->offset = offset_drm;
      index_drm->size = sizeof(rvcn_dec_message_drm_t);
      index_drm->filled = 0;
      ++header->num_buffers;
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      index_dynamic_dpb->message_id = RDECODE_MESSAGE_DYNAMIC_DPB;
      index_dynamic_dpb->offset = offset_dynamic_dpb;
      index_dynamic_dpb->filled = 0;
      ++header->num_buffers;
      if (dec->dpb_type == DPB_DYNAMIC_TIER_1)
         index_dynamic_dpb->size = sizeof(rvcn_dec_message_dynamic_dpb_t);
      else if (dec->dpb_type == DPB_DYNAMIC_TIER_2)
         index_dynamic_dpb->size = sizeof(rvcn_dec_message_dynamic_dpb_t2_t);
   }

   decode->stream_type = dec->stream_type;
   decode->decode_flags = 0;
   decode->width_in_samples = dec->base.width;
   decode->height_in_samples = dec->base.height;

   decode->bsd_size = align(dec->bs_size, 128);

   if (!dec->dpb.res && dec->dpb_type != DPB_DYNAMIC_TIER_2) {
      bool r;
      if (dec->dpb_size) {
         if (encrypted) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->dpb, dec->dpb_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->dpb, dec->dpb_size, PIPE_USAGE_DEFAULT);
         }
         assert(encrypted == (bool)(dec->dpb.res->flags & RADEON_FLAG_ENCRYPTED));
         if (!r) {
            RVID_ERR("Can't allocated dpb.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->dpb);
      }
   }

   if (!dec->ctx.res) {
      enum pipe_video_format fmt = u_reduce_video_profile(picture->profile);
      if (dec->stream_type == RDECODE_CODEC_H264_PERF) {
         unsigned ctx_size = calc_ctx_size_h264_perf(dec);
         bool r;
         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         assert((encrypted && dec->tmz_ctx) == (bool)(dec->ctx.res->flags & RADEON_FLAG_ENCRYPTED));

         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);
      } else if (fmt == PIPE_VIDEO_FORMAT_VP9) {
         unsigned ctx_size;
         uint8_t *ptr;
         bool r;

         /* default probability + probability data */
         ctx_size = 2304 * 5;

         if (((struct si_screen *)dec->screen)->info.family >= CHIP_RENOIR) {
            /* SRE collocated context data */
            ctx_size += 32 * 2 * 128 * 68;
            /* SMP collocated context data */
            ctx_size += 9 * 64 * 2 * 128 * 68;
            /* SDB left tile pixel */
            ctx_size += 8 * 2 * 2 * 8192;
         } else {
            ctx_size += 32 * 2 * 64 * 64;
            ctx_size += 9 * 64 * 2 * 64 * 64;
            ctx_size += 8 * 2 * 4096;
         }

         if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
            ctx_size += 8 * 2 * 4096;

         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);

         /* ctx needs probs table */
         ptr = dec->ws->buffer_map(dec->ws, dec->ctx.res->buf, &dec->cs,
                                   PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
         fill_probs_table(ptr);
         dec->ws->buffer_unmap(dec->ws, dec->ctx.res->buf);
         dec->bs_ptr = NULL;
      } else if (fmt == PIPE_VIDEO_FORMAT_HEVC) {
         unsigned ctx_size;
         bool r;
         if (dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
            ctx_size = calc_ctx_size_h265_main10(dec, (struct pipe_h265_picture_desc *)picture);
         else
            ctx_size = calc_ctx_size_h265_main(dec);

         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);
      }
   }
   if (encrypted != dec->ws->cs_is_secure(&dec->cs)) {
      dec->ws->cs_flush(&dec->cs, RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION, NULL);
   }

   decode->dpb_size = (dec->dpb_type != DPB_DYNAMIC_TIER_2) ? dec->dpb.res->buf->size : 0;
   decode->dt_size = si_resource(((struct vl_video_buffer *)target)->resources[0])->buf->size +
                     si_resource(((struct vl_video_buffer *)target)->resources[1])->buf->size;

   decode->sct_size = 0;
   decode->sc_coeff_size = 0;

   decode->sw_ctxt_size = RDECODE_SESSION_CONTEXT_SIZE;
   decode->db_pitch = align(dec->base.width, dec->db_alignment);

   if (((struct si_screen*)dec->screen)->info.family >= CHIP_SIENNA_CICHLID &&
       (dec->stream_type == RDECODE_CODEC_VP9 || dec->stream_type == RDECODE_CODEC_AV1 ||
        dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10))
      decode->db_aligned_height = align(dec->base.height, 64);

   decode->db_surf_tile_config = 0;

   decode->dt_pitch = luma->surface.u.gfx9.surf_pitch * luma->surface.blk_w;
   decode->dt_uv_pitch = chroma->surface.u.gfx9.surf_pitch * chroma->surface.blk_w;

   if (luma->surface.meta_offset) {
      RVID_ERR("DCC surfaces not supported.\n");
      return NULL;
   }

   decode->dt_tiling_mode = 0;
   decode->dt_swizzle_mode = luma->surface.u.gfx9.swizzle_mode;
   decode->dt_array_mode = RDECODE_ARRAY_MODE_LINEAR;
   decode->dt_field_mode = ((struct vl_video_buffer *)target)->base.interlaced;
   decode->dt_surf_tile_config = 0;
   decode->dt_uv_surf_tile_config = 0;

   decode->dt_luma_top_offset = luma->surface.u.gfx9.surf_offset;
   decode->dt_chroma_top_offset = chroma->surface.u.gfx9.surf_offset;
   if (decode->dt_field_mode) {
      decode->dt_luma_bottom_offset =
         luma->surface.u.gfx9.surf_offset + luma->surface.u.gfx9.surf_slice_size;
      decode->dt_chroma_bottom_offset =
         chroma->surface.u.gfx9.surf_offset + chroma->surface.u.gfx9.surf_slice_size;
   } else {
      decode->dt_luma_bottom_offset = decode->dt_luma_top_offset;
      decode->dt_chroma_bottom_offset = decode->dt_chroma_top_offset;
   }
   if (dec->stream_type == RDECODE_CODEC_AV1)
      decode->db_pitch_uv = chroma->surface.u.gfx9.surf_pitch * chroma->surface.blk_w;

   if (encrypted) {
      assert(sscreen->info.has_tmz_support);
      set_drm_keys(drm, decrypt);
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_1) {
      decode->decode_flags = 1;
      dynamic_dpb->dpbArraySize = NUM_VP9_REFS + 1;
      dynamic_dpb->dpbLumaPitch = align(decode->width_in_samples, dec->db_alignment);
      dynamic_dpb->dpbLumaAlignedHeight = align(decode->height_in_samples, dec->db_alignment);
      dynamic_dpb->dpbLumaAlignedSize =
         dynamic_dpb->dpbLumaPitch * dynamic_dpb->dpbLumaAlignedHeight;
      dynamic_dpb->dpbChromaPitch = dynamic_dpb->dpbLumaPitch >> 1;
      dynamic_dpb->dpbChromaAlignedHeight = dynamic_dpb->dpbLumaAlignedHeight >> 1;
      dynamic_dpb->dpbChromaAlignedSize =
         dynamic_dpb->dpbChromaPitch * dynamic_dpb->dpbChromaAlignedHeight * 2;
      dynamic_dpb->dpbReserved0[0] = dec->db_alignment;

      if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) {
         dynamic_dpb->dpbLumaAlignedSize = dynamic_dpb->dpbLumaAlignedSize * 3 / 2;
         dynamic_dpb->dpbChromaAlignedSize = dynamic_dpb->dpbChromaAlignedSize * 3 / 2;
      }
   }

   switch (u_reduce_video_profile(picture->profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      rvcn_dec_message_avc_t avc = get_h264_msg(dec, (struct pipe_h264_picture_desc *)picture);
      memcpy(codec, (void *)&avc, sizeof(rvcn_dec_message_avc_t));
      index_codec->message_id = RDECODE_MESSAGE_AVC;
      break;
   }
   case PIPE_VIDEO_FORMAT_HEVC: {
      rvcn_dec_message_hevc_t hevc =
         get_h265_msg(dec, target, (struct pipe_h265_picture_desc *)picture);

      memcpy(codec, (void *)&hevc, sizeof(rvcn_dec_message_hevc_t));
      index_codec->message_id = RDECODE_MESSAGE_HEVC;
      break;
   }
   case PIPE_VIDEO_FORMAT_VC1: {
      rvcn_dec_message_vc1_t vc1 = get_vc1_msg((struct pipe_vc1_picture_desc *)picture);

      memcpy(codec, (void *)&vc1, sizeof(rvcn_dec_message_vc1_t));
      if ((picture->profile == PIPE_VIDEO_PROFILE_VC1_SIMPLE) ||
          (picture->profile == PIPE_VIDEO_PROFILE_VC1_MAIN)) {
         decode->width_in_samples = align(decode->width_in_samples, 16) / 16;
         decode->height_in_samples = align(decode->height_in_samples, 16) / 16;
      }
      index_codec->message_id = RDECODE_MESSAGE_VC1;
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG12: {
      rvcn_dec_message_mpeg2_vld_t mpeg2 =
         get_mpeg2_msg(dec, (struct pipe_mpeg12_picture_desc *)picture);

      memcpy(codec, (void *)&mpeg2, sizeof(rvcn_dec_message_mpeg2_vld_t));
      index_codec->message_id = RDECODE_MESSAGE_MPEG2_VLD;
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG4: {
      rvcn_dec_message_mpeg4_asp_vld_t mpeg4 =
         get_mpeg4_msg(dec, (struct pipe_mpeg4_picture_desc *)picture);

      memcpy(codec, (void *)&mpeg4, sizeof(rvcn_dec_message_mpeg4_asp_vld_t));
      index_codec->message_id = RDECODE_MESSAGE_MPEG4_ASP_VLD;
      break;
   }
   case PIPE_VIDEO_FORMAT_VP9: {
      rvcn_dec_message_vp9_t vp9 =
         get_vp9_msg(dec, target, (struct pipe_vp9_picture_desc *)picture);

      memcpy(codec, (void *)&vp9, sizeof(rvcn_dec_message_vp9_t));
      index_codec->message_id = RDECODE_MESSAGE_VP9;
      break;
   }
   case PIPE_VIDEO_FORMAT_AV1: {
      rvcn_dec_message_av1_t av1 =
         get_av1_msg(dec, target, (struct pipe_av1_picture_desc *)picture);

      memcpy(codec, (void *)&av1, sizeof(rvcn_dec_message_av1_t));
      index_codec->message_id = RDECODE_MESSAGE_AV1;

      if (dec->ctx.res == NULL) {
         unsigned ctx_size = (9 + 4) * align(sizeof(rvcn_av1_hw_frame_context_t), 2048) +
                             9 * 64 * 34 * 512 + 9 * 64 * 34 * 256 * 5;
         int num_64x64_CTB_8k = 68;
         int num_128x128_CTB_8k = 34;
         int sdb_pitch_64x64 = align(32 * num_64x64_CTB_8k, 256);
         int sdb_pitch_128x128 = align(32 * num_128x128_CTB_8k, 256);
         int sdb_lf_size_ctb_64x64 = sdb_pitch_64x64 * (1728 / 32);
         int sdb_lf_size_ctb_128x128 = sdb_pitch_128x128 * (3008 / 32);
         int sdb_superres_size_ctb_64x64 = sdb_pitch_64x64 * (3232 / 32);
         int sdb_superres_size_ctb_128x128 = sdb_pitch_128x128 * (6208 / 32);
         int sdb_output_size_ctb_64x64 = sdb_pitch_64x64 * (1312 / 32);
         int sdb_output_size_ctb_128x128 = sdb_pitch_128x128 * (2336 / 32);
         int sdb_fg_avg_luma_size_ctb_64x64 = sdb_pitch_64x64 * (384 / 32);
         int sdb_fg_avg_luma_size_ctb_128x128 = sdb_pitch_128x128 * (640 / 32);
         uint8_t *ptr;
         int i;

         ctx_size += (MAX2(sdb_lf_size_ctb_64x64, sdb_lf_size_ctb_128x128) +
                      MAX2(sdb_superres_size_ctb_64x64, sdb_superres_size_ctb_128x128) +
                      MAX2(sdb_output_size_ctb_64x64, sdb_output_size_ctb_128x128) +
                      MAX2(sdb_fg_avg_luma_size_ctb_64x64, sdb_fg_avg_luma_size_ctb_128x128)) * 2  + 68 * 512;

         if (!si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT))
            RVID_ERR("Can't allocated context buffer.\n");
         si_vid_clear_buffer(dec->base.context, &dec->ctx);

         ptr = dec->ws->buffer_map(dec->ws, dec->ctx.res->buf, &dec->cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);

         for (i = 0; i < 4; ++i) {
            rvcn_init_mode_probs((void*)(ptr + i * align(sizeof(rvcn_av1_frame_context_t), 2048)));
            rvcn_av1_init_mv_probs((void*)(ptr + i * align(sizeof(rvcn_av1_frame_context_t), 2048)));
            rvcn_av1_default_coef_probs((void*)(ptr + i * align(sizeof(rvcn_av1_frame_context_t), 2048)), i);
         }
         dec->ws->buffer_unmap(dec->ws, dec->ctx.res->buf);
      }

      break;
   }
   default:
      assert(0);
      return NULL;
   }

   if (dec->ctx.res)
      decode->hw_ctxt_size = dec->ctx.res->buf->size;

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2)
      if (rvcn_dec_dynamic_dpb_t2_message(dec, decode, dynamic_dpb_t2))
         return NULL;

   return luma->buffer.buf;
}

static void rvcn_dec_message_destroy(struct radeon_decoder *dec)
{
   rvcn_dec_message_header_t *header = dec->msg;

   memset(dec->msg, 0, sizeof(rvcn_dec_message_header_t));
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizeof(rvcn_dec_message_header_t) - sizeof(rvcn_dec_message_index_t);
   header->num_buffers = 0;
   header->msg_type = RDECODE_MSG_DESTROY;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = 0;
}

static void rvcn_dec_message_feedback(struct radeon_decoder *dec)
{
   rvcn_dec_feedback_header_t *header = (void *)dec->fb;

   header->header_size = sizeof(rvcn_dec_feedback_header_t);
   header->total_size = sizeof(rvcn_dec_feedback_header_t);
   header->num_buffers = 0;
}

/* flush IB to the hardware */
static int flush(struct radeon_decoder *dec, unsigned flags)
{
   return dec->ws->cs_flush(&dec->cs, flags, NULL);
}

/* add a new set register command to the IB */
static void set_reg(struct radeon_decoder *dec, unsigned reg, uint32_t val)
{
   radeon_emit(&dec->cs, RDECODE_PKT0(reg >> 2, 0));
   radeon_emit(&dec->cs, val);
}

/* send a command to the VCPU through the GPCOM registers */
static void send_cmd(struct radeon_decoder *dec, unsigned cmd, struct pb_buffer *buf, uint32_t off,
                     enum radeon_bo_usage usage, enum radeon_bo_domain domain)
{
   uint64_t addr;

   dec->ws->cs_add_buffer(&dec->cs, buf, usage | RADEON_USAGE_SYNCHRONIZED, domain, 0);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   set_reg(dec, dec->reg.data0, addr);
   set_reg(dec, dec->reg.data1, addr >> 32);
   set_reg(dec, dec->reg.cmd, cmd << 1);
}

/* do the codec needs an IT buffer ?*/
static bool have_it(struct radeon_decoder *dec)
{
   return dec->stream_type == RDECODE_CODEC_H264_PERF || dec->stream_type == RDECODE_CODEC_H265;
}

/* do the codec needs an probs buffer? */
static bool have_probs(struct radeon_decoder *dec)
{
   return (dec->stream_type == RDECODE_CODEC_VP9 || dec->stream_type == RDECODE_CODEC_AV1);
}

/* map the next available message/feedback/itscaling buffer */
static void map_msg_fb_it_probs_buf(struct radeon_decoder *dec)
{
   struct rvid_buffer *buf;
   uint8_t *ptr;

   /* grab the current message/feedback buffer */
   buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];

   /* and map it for CPU access */
   ptr =
      dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);

   /* calc buffer offsets */
   dec->msg = ptr;

   dec->fb = (uint32_t *)(ptr + FB_BUFFER_OFFSET);
   if (have_it(dec))
      dec->it = (uint8_t *)(ptr + FB_BUFFER_OFFSET + FB_BUFFER_SIZE);
   else if (have_probs(dec))
      dec->probs = (uint8_t *)(ptr + FB_BUFFER_OFFSET + FB_BUFFER_SIZE);
}

/* unmap and send a message command to the VCPU */
static void send_msg_buf(struct radeon_decoder *dec)
{
   struct rvid_buffer *buf;

   /* ignore the request if message/feedback buffer isn't mapped */
   if (!dec->msg || !dec->fb)
      return;

   /* grab the current message buffer */
   buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];

   /* unmap the buffer */
   dec->ws->buffer_unmap(dec->ws, buf->res->buf);
   dec->bs_ptr = NULL;
   dec->msg = NULL;
   dec->fb = NULL;
   dec->it = NULL;
   dec->probs = NULL;

   if (dec->sessionctx.res)
      send_cmd(dec, RDECODE_CMD_SESSION_CONTEXT_BUFFER, dec->sessionctx.res->buf, 0,
               RADEON_USAGE_READWRITE, RADEON_DOMAIN_VRAM);

   /* and send it to the hardware */
   send_cmd(dec, RDECODE_CMD_MSG_BUFFER, buf->res->buf, 0, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
}

/* cycle to the next set of buffers */
static void next_buffer(struct radeon_decoder *dec)
{
   ++dec->cur_buffer;
   dec->cur_buffer %= NUM_BUFFERS;
}

static unsigned calc_ctx_size_h264_perf(struct radeon_decoder *dec)
{
   unsigned width_in_mb, height_in_mb, ctx_size;
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   unsigned max_references = dec->base.max_references + 1;

   // picture width & height in 16 pixel units
   width_in_mb = width / VL_MACROBLOCK_WIDTH;
   height_in_mb = align(height / VL_MACROBLOCK_HEIGHT, 2);

   unsigned fs_in_mb = width_in_mb * height_in_mb;
   unsigned num_dpb_buffer;
   switch (dec->base.level) {
   case 30:
      num_dpb_buffer = 8100 / fs_in_mb;
      break;
   case 31:
      num_dpb_buffer = 18000 / fs_in_mb;
      break;
   case 32:
      num_dpb_buffer = 20480 / fs_in_mb;
      break;
   case 41:
      num_dpb_buffer = 32768 / fs_in_mb;
      break;
   case 42:
      num_dpb_buffer = 34816 / fs_in_mb;
      break;
   case 50:
      num_dpb_buffer = 110400 / fs_in_mb;
      break;
   case 51:
      num_dpb_buffer = 184320 / fs_in_mb;
      break;
   default:
      num_dpb_buffer = 184320 / fs_in_mb;
      break;
   }
   num_dpb_buffer++;
   max_references = MAX2(MIN2(NUM_H264_REFS, num_dpb_buffer), max_references);
   ctx_size = max_references * align(width_in_mb * height_in_mb * 192, 256);

   return ctx_size;
}

/* calculate size of reference picture buffer */
static unsigned calc_dpb_size(struct radeon_decoder *dec)
{
   unsigned width_in_mb, height_in_mb, image_size, dpb_size;

   // always align them to MB size for dpb calculation
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   // always one more for currently decoded picture
   unsigned max_references = dec->base.max_references + 1;

   // aligned size of a single frame
   image_size = align(width, 32) * height;
   image_size += image_size / 2;
   image_size = align(image_size, 1024);

   // picture width & height in 16 pixel units
   width_in_mb = width / VL_MACROBLOCK_WIDTH;
   height_in_mb = align(height / VL_MACROBLOCK_HEIGHT, 2);

   switch (u_reduce_video_profile(dec->base.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      unsigned fs_in_mb = width_in_mb * height_in_mb;
      unsigned num_dpb_buffer;

      switch (dec->base.level) {
      case 30:
         num_dpb_buffer = 8100 / fs_in_mb;
         break;
      case 31:
         num_dpb_buffer = 18000 / fs_in_mb;
         break;
      case 32:
         num_dpb_buffer = 20480 / fs_in_mb;
         break;
      case 41:
         num_dpb_buffer = 32768 / fs_in_mb;
         break;
      case 42:
         num_dpb_buffer = 34816 / fs_in_mb;
         break;
      case 50:
         num_dpb_buffer = 110400 / fs_in_mb;
         break;
      case 51:
         num_dpb_buffer = 184320 / fs_in_mb;
         break;
      default:
         num_dpb_buffer = 184320 / fs_in_mb;
         break;
      }
      num_dpb_buffer++;
      max_references = MAX2(MIN2(NUM_H264_REFS, num_dpb_buffer), max_references);
      dpb_size = image_size * max_references;
      break;
   }

   case PIPE_VIDEO_FORMAT_HEVC:
      if (dec->base.width * dec->base.height >= 4096 * 2000)
         max_references = MAX2(max_references, 8);
      else
         max_references = MAX2(max_references, 17);

      width = align(width, 16);
      height = align(height, 16);
      if (dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
         dpb_size = align((align(width, 64) * align(height, 64) * 9) / 4, 256) * max_references;
      else
         dpb_size = align((align(width, 32) * height * 3) / 2, 256) * max_references;
      break;

   case PIPE_VIDEO_FORMAT_VC1:
      // the firmware seems to allways assume a minimum of ref frames
      max_references = MAX2(NUM_VC1_REFS, max_references);

      // reference picture buffer
      dpb_size = image_size * max_references;

      // CONTEXT_BUFFER
      dpb_size += width_in_mb * height_in_mb * 128;

      // IT surface buffer
      dpb_size += width_in_mb * 64;

      // DB surface buffer
      dpb_size += width_in_mb * 128;

      // BP
      dpb_size += align(MAX2(width_in_mb, height_in_mb) * 7 * 16, 64);
      break;

   case PIPE_VIDEO_FORMAT_MPEG12:
      // reference picture buffer, must be big enough for all frames
      dpb_size = image_size * NUM_MPEG2_REFS;
      break;

   case PIPE_VIDEO_FORMAT_MPEG4:
      // reference picture buffer
      dpb_size = image_size * max_references;

      // CM
      dpb_size += width_in_mb * height_in_mb * 64;

      // IT surface buffer
      dpb_size += align(width_in_mb * height_in_mb * 32, 64);

      dpb_size = MAX2(dpb_size, 30 * 1024 * 1024);
      break;

   case PIPE_VIDEO_FORMAT_VP9:
      max_references = MAX2(max_references, 9);

      if (dec->dpb_type == DPB_MAX_RES)
         dpb_size = (((struct si_screen *)dec->screen)->info.family >= CHIP_RENOIR)
            ? (8192 * 4320 * 3 / 2) * max_references
            : (4096 * 3000 * 3 / 2) * max_references;
      else
         dpb_size = (align(dec->base.width, dec->db_alignment) *
            align(dec->base.height, dec->db_alignment) * 3 / 2) * max_references;

      if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
         dpb_size = dpb_size * 3 / 2;
      break;

   case PIPE_VIDEO_FORMAT_AV1:
      max_references = MAX2(max_references, 9);
      dpb_size = 8192 * 4320 * 3 / 2 * max_references * 3 / 2;
      break;

   case PIPE_VIDEO_FORMAT_JPEG:
      dpb_size = 0;
      break;

   default:
      // something is missing here
      assert(0);

      // at least use a sane default value
      dpb_size = 32 * 1024 * 1024;
      break;
   }
   return dpb_size;
}

/**
 * destroy this video decoder
 */
static void radeon_dec_destroy(struct pipe_video_codec *decoder)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   unsigned i;

   assert(decoder);

   map_msg_fb_it_probs_buf(dec);
   rvcn_dec_message_destroy(dec);
   send_msg_buf(dec);

   flush(dec, 0);

   dec->ws->cs_destroy(&dec->cs);

   for (i = 0; i < NUM_BUFFERS; ++i) {
      si_vid_destroy_buffer(&dec->msg_fb_it_probs_buffers[i]);
      si_vid_destroy_buffer(&dec->bs_buffers[i]);
   }

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2) {
      si_vid_destroy_buffer(&dec->dpb);
   } else {
      list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
         list_del(&d->list);
         si_vid_destroy_buffer(&d->dpb);
         FREE(d);
      }
   }
   si_vid_destroy_buffer(&dec->ctx);
   si_vid_destroy_buffer(&dec->sessionctx);

   FREE(dec);
}

/**
 * start decoding of a new frame
 */
static void radeon_dec_begin_frame(struct pipe_video_codec *decoder,
                                   struct pipe_video_buffer *target,
                                   struct pipe_picture_desc *picture)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   uintptr_t frame;

   assert(decoder);

   frame = ++dec->frame_number;
   if (dec->stream_type != RDECODE_CODEC_VP9 && dec->stream_type != RDECODE_CODEC_AV1)
      vl_video_buffer_set_associated_data(target, decoder, (void *)frame,
                                          &radeon_dec_destroy_associated_data);

   dec->bs_size = 0;
   dec->bs_ptr = dec->ws->buffer_map(dec->ws, dec->bs_buffers[dec->cur_buffer].res->buf, &dec->cs,
                                     PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
}

/**
 * decode a macroblock
 */
static void radeon_dec_decode_macroblock(struct pipe_video_codec *decoder,
                                         struct pipe_video_buffer *target,
                                         struct pipe_picture_desc *picture,
                                         const struct pipe_macroblock *macroblocks,
                                         unsigned num_macroblocks)
{
   /* not supported (yet) */
   assert(0);
}

/**
 * decode a bitstream
 */
static void radeon_dec_decode_bitstream(struct pipe_video_codec *decoder,
                                        struct pipe_video_buffer *target,
                                        struct pipe_picture_desc *picture, unsigned num_buffers,
                                        const void *const *buffers, const unsigned *sizes)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   unsigned i;

   assert(decoder);

   if (!dec->bs_ptr)
      return;

   for (i = 0; i < num_buffers; ++i) {
      struct rvid_buffer *buf = &dec->bs_buffers[dec->cur_buffer];
      unsigned new_size = dec->bs_size + sizes[i];

      if (new_size > buf->res->buf->size) {
         dec->ws->buffer_unmap(dec->ws, buf->res->buf);
         dec->bs_ptr = NULL;
         if (!si_vid_resize_buffer(dec->screen, &dec->cs, buf, new_size)) {
            RVID_ERR("Can't resize bitstream buffer!");
            return;
         }

         dec->bs_ptr = dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs,
                                           PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
         if (!dec->bs_ptr)
            return;

         dec->bs_ptr += dec->bs_size;
      }

      memcpy(dec->bs_ptr, buffers[i], sizes[i]);
      dec->bs_size += sizes[i];
      dec->bs_ptr += sizes[i];
   }
}

/**
 * send cmd for vcn dec
 */
void send_cmd_dec(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                  struct pipe_picture_desc *picture)
{
   struct pb_buffer *dt;
   struct rvid_buffer *msg_fb_it_probs_buf, *bs_buf;

   msg_fb_it_probs_buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];
   bs_buf = &dec->bs_buffers[dec->cur_buffer];

   memset(dec->bs_ptr, 0, align(dec->bs_size, 128) - dec->bs_size);
   dec->ws->buffer_unmap(dec->ws, bs_buf->res->buf);
   dec->bs_ptr = NULL;

   map_msg_fb_it_probs_buf(dec);
   dt = rvcn_dec_message_decode(dec, target, picture);
   rvcn_dec_message_feedback(dec);
   send_msg_buf(dec);

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2)
      send_cmd(dec, RDECODE_CMD_DPB_BUFFER, dec->dpb.res->buf, 0, RADEON_USAGE_READWRITE,
            RADEON_DOMAIN_VRAM);
   if (dec->ctx.res)
      send_cmd(dec, RDECODE_CMD_CONTEXT_BUFFER, dec->ctx.res->buf, 0, RADEON_USAGE_READWRITE,
               RADEON_DOMAIN_VRAM);
   send_cmd(dec, RDECODE_CMD_BITSTREAM_BUFFER, bs_buf->res->buf, 0, RADEON_USAGE_READ,
            RADEON_DOMAIN_GTT);
   send_cmd(dec, RDECODE_CMD_DECODING_TARGET_BUFFER, dt, 0, RADEON_USAGE_WRITE, RADEON_DOMAIN_VRAM);
   send_cmd(dec, RDECODE_CMD_FEEDBACK_BUFFER, msg_fb_it_probs_buf->res->buf, FB_BUFFER_OFFSET,
            RADEON_USAGE_WRITE, RADEON_DOMAIN_GTT);
   if (have_it(dec))
      send_cmd(dec, RDECODE_CMD_IT_SCALING_TABLE_BUFFER, msg_fb_it_probs_buf->res->buf,
               FB_BUFFER_OFFSET + FB_BUFFER_SIZE, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
   else if (have_probs(dec))
      send_cmd(dec, RDECODE_CMD_PROB_TBL_BUFFER, msg_fb_it_probs_buf->res->buf,
               FB_BUFFER_OFFSET + FB_BUFFER_SIZE, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
   set_reg(dec, dec->reg.cntl, 1);
}

/**
 * end decoding of the current frame
 */
static void radeon_dec_end_frame(struct pipe_video_codec *decoder, struct pipe_video_buffer *target,
                                 struct pipe_picture_desc *picture)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;

   assert(decoder);

   if (!dec->bs_ptr)
      return;

   dec->send_cmd(dec, target, picture);
   flush(dec, PIPE_FLUSH_ASYNC);
   next_buffer(dec);
}

/**
 * flush any outstanding command buffers to the hardware
 */
static void radeon_dec_flush(struct pipe_video_codec *decoder)
{
}

/**
 * create and HW decoder
 */
struct pipe_video_codec *radeon_create_decoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templ)
{
   struct si_context *sctx = (struct si_context *)context;
   struct radeon_winsys *ws = sctx->ws;
   unsigned width = templ->width, height = templ->height;
   unsigned bs_buf_size, stream_type = 0, ring = RING_VCN_DEC;
   struct radeon_decoder *dec;
   int r, i;

   switch (u_reduce_video_profile(templ->profile)) {
   case PIPE_VIDEO_FORMAT_MPEG12:
      if (templ->entrypoint > PIPE_VIDEO_ENTRYPOINT_BITSTREAM)
         return vl_create_mpeg12_decoder(context, templ);
      stream_type = RDECODE_CODEC_MPEG2_VLD;
      break;
   case PIPE_VIDEO_FORMAT_MPEG4:
      width = align(width, VL_MACROBLOCK_WIDTH);
      height = align(height, VL_MACROBLOCK_HEIGHT);
      stream_type = RDECODE_CODEC_MPEG4;
      break;
   case PIPE_VIDEO_FORMAT_VC1:
      stream_type = RDECODE_CODEC_VC1;
      break;
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      width = align(width, VL_MACROBLOCK_WIDTH);
      height = align(height, VL_MACROBLOCK_HEIGHT);
      stream_type = RDECODE_CODEC_H264_PERF;
      break;
   case PIPE_VIDEO_FORMAT_HEVC:
      stream_type = RDECODE_CODEC_H265;
      break;
   case PIPE_VIDEO_FORMAT_VP9:
      stream_type = RDECODE_CODEC_VP9;
      break;
   case PIPE_VIDEO_FORMAT_AV1:
      stream_type = RDECODE_CODEC_AV1;
      break;
   case PIPE_VIDEO_FORMAT_JPEG:
      stream_type = RDECODE_CODEC_JPEG;
      ring = RING_VCN_JPEG;
      break;
   default:
      assert(0);
      break;
   }

   dec = CALLOC_STRUCT(radeon_decoder);

   if (!dec)
      return NULL;

   dec->base = *templ;
   dec->base.context = context;
   dec->base.width = width;
   dec->base.height = height;

   dec->base.destroy = radeon_dec_destroy;
   dec->base.begin_frame = radeon_dec_begin_frame;
   dec->base.decode_macroblock = radeon_dec_decode_macroblock;
   dec->base.decode_bitstream = radeon_dec_decode_bitstream;
   dec->base.end_frame = radeon_dec_end_frame;
   dec->base.flush = radeon_dec_flush;

   dec->stream_type = stream_type;
   dec->stream_handle = si_vid_alloc_stream_handle();
   dec->screen = context->screen;
   dec->ws = ws;

   if (!ws->cs_create(&dec->cs, sctx->ctx, ring, NULL, NULL, false)) {
      RVID_ERR("Can't get command submission context.\n");
      goto error;
   }

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++)
      dec->render_pic_list[i] = NULL;
   bs_buf_size = width * height * (512 / (16 * 16));
   for (i = 0; i < NUM_BUFFERS; ++i) {
      unsigned msg_fb_it_probs_size = FB_BUFFER_OFFSET + FB_BUFFER_SIZE;
      if (have_it(dec))
         msg_fb_it_probs_size += IT_SCALING_TABLE_SIZE;
      else if (have_probs(dec))
         msg_fb_it_probs_size += (dec->stream_type == RDECODE_CODEC_VP9) ?
                                 VP9_PROBS_TABLE_SIZE :
                                 sizeof(rvcn_dec_av1_segment_fg_t);
      /* use vram to improve performance, workaround an unknown bug */
      if (!si_vid_create_buffer(dec->screen, &dec->msg_fb_it_probs_buffers[i], msg_fb_it_probs_size,
                                PIPE_USAGE_DEFAULT)) {
         RVID_ERR("Can't allocated message buffers.\n");
         goto error;
      }

      if (!si_vid_create_buffer(dec->screen, &dec->bs_buffers[i], bs_buf_size,
                                PIPE_USAGE_STAGING)) {
         RVID_ERR("Can't allocated bitstream buffers.\n");
         goto error;
      }

      si_vid_clear_buffer(context, &dec->msg_fb_it_probs_buffers[i]);
      si_vid_clear_buffer(context, &dec->bs_buffers[i]);

      if (have_probs(dec) && dec->stream_type == RDECODE_CODEC_VP9) {
         struct rvid_buffer *buf;
         void *ptr;

         buf = &dec->msg_fb_it_probs_buffers[i];
         ptr = dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs,
                                   PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
         ptr += FB_BUFFER_OFFSET + FB_BUFFER_SIZE;
         fill_probs_table(ptr);
         dec->ws->buffer_unmap(dec->ws, buf->res->buf);
         dec->bs_ptr = NULL;
      }
   }

   if (sctx->family >= CHIP_SIENNA_CICHLID &&
       (stream_type == RDECODE_CODEC_VP9 || stream_type == RDECODE_CODEC_AV1))
      dec->dpb_type = DPB_DYNAMIC_TIER_2;
   else if (sctx->family <= CHIP_NAVI14 && stream_type == RDECODE_CODEC_VP9)
      dec->dpb_type = DPB_DYNAMIC_TIER_1;
   else
      dec->dpb_type = DPB_MAX_RES;

   dec->db_alignment = (((struct si_screen *)dec->screen)->info.family >= CHIP_RENOIR &&
                   dec->base.width > 32 && (dec->stream_type == RDECODE_CODEC_VP9 ||
                   dec->stream_type == RDECODE_CODEC_AV1 ||
                   dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)) ? 64 : 32;

   dec->dpb_size = calc_dpb_size(dec);

   if (!si_vid_create_buffer(dec->screen, &dec->sessionctx, RDECODE_SESSION_CONTEXT_SIZE,
                             PIPE_USAGE_DEFAULT)) {
      RVID_ERR("Can't allocated session ctx.\n");
      goto error;
   }
   si_vid_clear_buffer(context, &dec->sessionctx);

   switch (sctx->family) {
   case CHIP_RAVEN:
   case CHIP_RAVEN2:
      dec->reg.data0 = RDECODE_VCN1_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN1_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN1_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN1_ENGINE_CNTL;
      dec->jpg.direct_reg = false;
      break;
   case CHIP_NAVI10:
   case CHIP_NAVI12:
   case CHIP_NAVI14:
   case CHIP_RENOIR:
      dec->reg.data0 = RDECODE_VCN2_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN2_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN2_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN2_ENGINE_CNTL;
      dec->jpg.direct_reg = true;
      break;
   case CHIP_ARCTURUS:
   case CHIP_ALDEBARAN:
   case CHIP_SIENNA_CICHLID:
   case CHIP_NAVY_FLOUNDER:
   case CHIP_DIMGREY_CAVEFISH:
   case CHIP_BEIGE_GOBY:
   case CHIP_VANGOGH:
   case CHIP_YELLOW_CARP:
      dec->reg.data0 = RDECODE_VCN2_5_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN2_5_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN2_5_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN2_5_ENGINE_CNTL;
      dec->jpg.direct_reg = true;
      break;
   default:
      RVID_ERR("VCN is not supported.\n");
      goto error;
   }

   map_msg_fb_it_probs_buf(dec);
   rvcn_dec_message_create(dec);
   send_msg_buf(dec);
   r = flush(dec, 0);
   if (r)
      goto error;

   next_buffer(dec);

   if (stream_type == RDECODE_CODEC_JPEG)
      dec->send_cmd = send_cmd_jpeg;
   else
      dec->send_cmd = send_cmd_dec;


   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      list_inithead(&dec->dpb_ref_list);
      list_inithead(&dec->dpb_unref_list);
   }

   dec->tmz_ctx = sctx->family < CHIP_RENOIR;

   return &dec->base;

error:
   dec->ws->cs_destroy(&dec->cs);

   for (i = 0; i < NUM_BUFFERS; ++i) {
      si_vid_destroy_buffer(&dec->msg_fb_it_probs_buffers[i]);
      si_vid_destroy_buffer(&dec->bs_buffers[i]);
   }

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2)
      si_vid_destroy_buffer(&dec->dpb);
   si_vid_destroy_buffer(&dec->ctx);
   si_vid_destroy_buffer(&dec->sessionctx);

   FREE(dec);

   return NULL;
}
