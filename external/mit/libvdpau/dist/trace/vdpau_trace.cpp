/*
 * Copyright (c) 2008-2015 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdpau/vdpau_x11.h>

#include "../src/util.h"

#define _VDP_TRACE_ARSIZE(_x_) ((sizeof (_x_)) / (sizeof ((_x_)[0])))

#if DEBUG

static void _vdp_trace_error_breakpoint(char const * file, int line, char const * function)
{
    fprintf(stderr, "VDPAU trace: Error detected at %s:%d %s()\n", file, line, function);
}

#define _VDP_TRACE_ERROR_BREAKPOINT() _vdp_trace_error_breakpoint(__FILE__, __LINE__, __FUNCTION__)

#else

#define _VDP_TRACE_ERROR_BREAKPOINT()

#endif

#define LEVEL_PARAMS 1
#define LEVEL_DATA   2

struct _VdpCapData {
    void * dll;

    int    level;
    FILE * fp;

    VdpDevice           vdp_device;
    VdpGetProcAddress * vdp_get_proc_address;

    VdpGetErrorString * vdp_get_error_string;
    VdpGetApiVersion * vdp_get_api_version;
    VdpGetInformationString * vdp_get_information_string;
    VdpDeviceDestroy * vdp_device_destroy;
    VdpGenerateCSCMatrix * vdp_generate_csc_matrix;
    VdpVideoSurfaceQueryCapabilities * vdp_video_surface_query_capabilities;
    VdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities * vdp_video_surface_query_get_put_bits_y_cb_cr_capabilities;
    VdpVideoSurfaceCreate * vdp_video_surface_create;
    VdpVideoSurfaceDestroy * vdp_video_surface_destroy;
    VdpVideoSurfaceGetParameters * vdp_video_surface_get_parameters;
    VdpVideoSurfaceGetBitsYCbCr * vdp_video_surface_get_bits_y_cb_cr;
    VdpVideoSurfacePutBitsYCbCr * vdp_video_surface_put_bits_y_cb_cr;
    VdpOutputSurfaceQueryCapabilities * vdp_output_surface_query_capabilities;
    VdpOutputSurfaceQueryGetPutBitsNativeCapabilities * vdp_output_surface_query_get_put_bits_native_capabilities;
    VdpOutputSurfaceQueryPutBitsIndexedCapabilities * vdp_output_surface_query_put_bits_indexed_capabilities;
    VdpOutputSurfaceQueryPutBitsYCbCrCapabilities * vdp_output_surface_query_put_bits_y_cb_cr_capabilities;
    VdpOutputSurfaceCreate * vdp_output_surface_create;
    VdpOutputSurfaceDestroy * vdp_output_surface_destroy;
    VdpOutputSurfaceGetParameters * vdp_output_surface_get_parameters;
    VdpOutputSurfaceGetBitsNative * vdp_output_surface_get_bits_native;
    VdpOutputSurfacePutBitsNative * vdp_output_surface_put_bits_native;
    VdpOutputSurfacePutBitsIndexed * vdp_output_surface_put_bits_indexed;
    VdpOutputSurfacePutBitsYCbCr * vdp_output_surface_put_bits_y_cb_cr;
    VdpBitmapSurfaceQueryCapabilities * vdp_bitmap_surface_query_capabilities;
    VdpBitmapSurfaceCreate * vdp_bitmap_surface_create;
    VdpBitmapSurfaceDestroy * vdp_bitmap_surface_destroy;
    VdpBitmapSurfaceGetParameters * vdp_bitmap_surface_get_parameters;
    VdpBitmapSurfacePutBitsNative * vdp_bitmap_surface_put_bits_native;
    VdpOutputSurfaceRenderOutputSurface * vdp_output_surface_render_output_surface;
    VdpOutputSurfaceRenderBitmapSurface * vdp_output_surface_render_bitmap_surface;
    VdpDecoderQueryCapabilities * vdp_decoder_query_capabilities;
    VdpDecoderCreate * vdp_decoder_create;
    VdpDecoderDestroy * vdp_decoder_destroy;
    VdpDecoderGetParameters * vdp_decoder_get_parameters;
    VdpDecoderRender * vdp_decoder_render;
    VdpVideoMixerQueryFeatureSupport * vdp_video_mixer_query_feature_support;
    VdpVideoMixerQueryParameterSupport * vdp_video_mixer_query_parameter_support;
    VdpVideoMixerQueryAttributeSupport * vdp_video_mixer_query_attribute_support;
    VdpVideoMixerQueryParameterValueRange * vdp_video_mixer_query_parameter_value_range;
    VdpVideoMixerQueryAttributeValueRange * vdp_video_mixer_query_attribute_value_range;
    VdpVideoMixerCreate * vdp_video_mixer_create;
    VdpVideoMixerSetFeatureEnables * vdp_video_mixer_set_feature_enables;
    VdpVideoMixerSetAttributeValues * vdp_video_mixer_set_attribute_values;
    VdpVideoMixerGetFeatureSupport * vdp_video_mixer_get_feature_support;
    VdpVideoMixerGetFeatureEnables * vdp_video_mixer_get_feature_enables;
    VdpVideoMixerGetParameterValues * vdp_video_mixer_get_parameter_values;
    VdpVideoMixerGetAttributeValues * vdp_video_mixer_get_attribute_values;
    VdpVideoMixerDestroy * vdp_video_mixer_destroy;
    VdpVideoMixerRender * vdp_video_mixer_render;
    VdpPresentationQueueTargetDestroy * vdp_presentation_queue_target_destroy;
    VdpPresentationQueueCreate * vdp_presentation_queue_create;
    VdpPresentationQueueDestroy * vdp_presentation_queue_destroy;
    VdpPresentationQueueSetBackgroundColor * vdp_presentation_queue_set_background_color;
    VdpPresentationQueueGetBackgroundColor * vdp_presentation_queue_get_background_color;
    VdpPresentationQueueGetTime * vdp_presentation_queue_get_time;
    VdpPresentationQueueDisplay * vdp_presentation_queue_display;
    VdpPresentationQueueBlockUntilSurfaceIdle * vdp_presentation_queue_block_until_surface_idle;
    VdpPresentationQueueQuerySurfaceStatus * vdp_presentation_queue_query_surface_status;
    VdpPreemptionCallbackRegister * vdp_preemption_callback_register;
    VdpPresentationQueueTargetCreateX11 * vdp_presentation_queue_target_create_x11;
};

static _VdpCapData _vdp_cap_data;

template<class T> static inline T const delta(T const & a, T const & b)
{
    if (a < b) {
        return b - a;
    }
    else {
        return a - b;
    }
}

static void _vdp_cap_dump_procamp(VdpProcamp * procamp)
{
    if (!procamp) {
        fprintf(_vdp_cap_data.fp, "NULL");
        return;
    }

    fprintf(
        _vdp_cap_data.fp,
        "{(ver=%d)%s %f, %f, %f, %f}",
        procamp->struct_version,
        (procamp->struct_version > 0)
            ? "(unsupported; cannot dump all fields)"
            : "",
        procamp->brightness,
        procamp->contrast,
        procamp->saturation,
        procamp->hue
    );
}

static void _vdp_cap_dump_color(
    VdpColor const * color
)
{
    if (!color) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fprintf(
        _vdp_cap_data.fp,
        "{%f, %f, %f, %f}",
        color->red,
        color->green,
        color->blue,
        color->alpha
    );
}

static void _vdp_cap_dump_rect(
    VdpRect const * rect
)
{
    if (!rect) {
        fprintf(_vdp_cap_data.fp, "NULL");
        return;
    }

    fprintf(
        _vdp_cap_data.fp,
        "{%u, %u, %u, %u}",
        rect->x0,
        rect->y0,
        rect->x1,
        rect->y1
    );
}

static void _vdp_cap_dump_csc_matrix(
    VdpCSCMatrix const * matrix
)
{
    if (!matrix) {
        fprintf(_vdp_cap_data.fp, "NULL");
        return;
    }

    fprintf(
        _vdp_cap_data.fp,
        "{{%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f}}",
        (*matrix)[0][0],
        (*matrix)[0][1],
        (*matrix)[0][2],
        (*matrix)[0][3],
        (*matrix)[1][0],
        (*matrix)[1][1],
        (*matrix)[1][2],
        (*matrix)[1][3],
        (*matrix)[2][0],
        (*matrix)[2][1],
        (*matrix)[2][2],
        (*matrix)[2][3]
    );
}

static void _vdp_cap_dump_blend_state(
    VdpOutputSurfaceRenderBlendState const * blend_state
)
{
    if (!blend_state) {
        fprintf(_vdp_cap_data.fp, "NULL");
        return;
    }

    fprintf(
        _vdp_cap_data.fp,
        "{(ver=%d)%s %u, %u, %u, %u, %u, %u, ",
        blend_state->struct_version,
        (blend_state->struct_version > 0)
            ? "(unsupported; cannot dump all fields)"
            : "",
        blend_state->blend_factor_source_color,
        blend_state->blend_factor_destination_color,
        blend_state->blend_factor_source_alpha,
        blend_state->blend_factor_destination_alpha,
        blend_state->blend_equation_color,
        blend_state->blend_equation_alpha
    );
    _vdp_cap_dump_color(&(blend_state->blend_constant));
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_picture_info(
    VdpDecoderProfile      profile,
    VdpPictureInfo const * picture_info
)
{
    switch (profile) {
    case VDP_DECODER_PROFILE_MPEG1:
    case VDP_DECODER_PROFILE_MPEG2_SIMPLE:
    case VDP_DECODER_PROFILE_MPEG2_MAIN:
        {
            VdpPictureInfoMPEG1Or2 const * picture_info_mpeg1or2 =
                (VdpPictureInfoMPEG1Or2 const *)picture_info;

            fprintf(
                _vdp_cap_data.fp,
                "{%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, {{%u, %u}, {%u, %u}}, {",
                picture_info_mpeg1or2->forward_reference,
                picture_info_mpeg1or2->backward_reference,
                picture_info_mpeg1or2->slice_count,
                (uint32_t)picture_info_mpeg1or2->picture_structure,
                (uint32_t)picture_info_mpeg1or2->picture_coding_type,
                (uint32_t)picture_info_mpeg1or2->intra_dc_precision,
                (uint32_t)picture_info_mpeg1or2->frame_pred_frame_dct,
                (uint32_t)picture_info_mpeg1or2->concealment_motion_vectors,
                (uint32_t)picture_info_mpeg1or2->intra_vlc_format,
                (uint32_t)picture_info_mpeg1or2->alternate_scan,
                (uint32_t)picture_info_mpeg1or2->q_scale_type,
                (uint32_t)picture_info_mpeg1or2->top_field_first,
                (uint32_t)picture_info_mpeg1or2->full_pel_forward_vector,
                (uint32_t)picture_info_mpeg1or2->full_pel_backward_vector,
                (uint32_t)picture_info_mpeg1or2->f_code[0][0],
                (uint32_t)picture_info_mpeg1or2->f_code[0][1],
                (uint32_t)picture_info_mpeg1or2->f_code[1][0],
                (uint32_t)picture_info_mpeg1or2->f_code[1][1]
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_mpeg1or2->intra_quantizer_matrix); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_mpeg1or2->intra_quantizer_matrix[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_mpeg1or2->non_intra_quantizer_matrix); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_mpeg1or2->non_intra_quantizer_matrix[i]
                );
            }
            fputs("}}", _vdp_cap_data.fp);
        }
        break;
    case VDP_DECODER_PROFILE_H264_BASELINE:
    case VDP_DECODER_PROFILE_H264_MAIN:
    case VDP_DECODER_PROFILE_H264_HIGH:
        {
            VdpPictureInfoH264 const * picture_info_h264 =
                (VdpPictureInfoH264 const *)picture_info;

            fprintf(
                _vdp_cap_data.fp,
                "{%u, {%d, %d}, %d, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %d, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, {",
                picture_info_h264->slice_count,
                picture_info_h264->field_order_cnt[0],
                picture_info_h264->field_order_cnt[1],
                (int32_t)picture_info_h264->is_reference,
                (uint32_t)picture_info_h264->frame_num,
                (uint32_t)picture_info_h264->field_pic_flag,
                (uint32_t)picture_info_h264->bottom_field_flag,
                (uint32_t)picture_info_h264->num_ref_frames,
                (uint32_t)picture_info_h264->mb_adaptive_frame_field_flag,
                (uint32_t)picture_info_h264->constrained_intra_pred_flag,
                (uint32_t)picture_info_h264->weighted_pred_flag,
                (uint32_t)picture_info_h264->weighted_bipred_idc,
                (uint32_t)picture_info_h264->frame_mbs_only_flag,
                (uint32_t)picture_info_h264->transform_8x8_mode_flag,
                (int32_t)picture_info_h264->chroma_qp_index_offset,
                (int32_t)picture_info_h264->second_chroma_qp_index_offset,
                (int32_t)picture_info_h264->pic_init_qp_minus26,
                (uint32_t)picture_info_h264->num_ref_idx_l0_active_minus1,
                (uint32_t)picture_info_h264->num_ref_idx_l1_active_minus1,
                (uint32_t)picture_info_h264->log2_max_frame_num_minus4,
                (uint32_t)picture_info_h264->pic_order_cnt_type,
                (uint32_t)picture_info_h264->log2_max_pic_order_cnt_lsb_minus4,
                (uint32_t)picture_info_h264->delta_pic_order_always_zero_flag,
                (uint32_t)picture_info_h264->direct_8x8_inference_flag,
                (uint32_t)picture_info_h264->entropy_coding_mode_flag,
                (uint32_t)picture_info_h264->pic_order_present_flag,
                (uint32_t)picture_info_h264->deblocking_filter_control_present_flag,
                (uint32_t)picture_info_h264->redundant_pic_cnt_present_flag
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_h264->scaling_lists_4x4); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_h264->scaling_lists_4x4[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_h264->scaling_lists_4x4[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_h264->scaling_lists_8x8); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_h264->scaling_lists_8x8[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_h264->scaling_lists_8x8[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_h264->referenceFrames); ++i) {
                VdpReferenceFrameH264 const * rf = &(picture_info_h264->referenceFrames[i]);
                fprintf(
                    _vdp_cap_data.fp,
                    "%s{%u, %d, %d, %d, {%d, %d}, %u}",
                    (i == 0) ? "" : ", ",
                    rf->surface,
                    (int32_t)rf->is_long_term,
                    (int32_t)rf->top_is_reference,
                    (int32_t)rf->bottom_is_reference,
                    (int32_t)rf->field_order_cnt[0],
                    (int32_t)rf->field_order_cnt[1],
                    (uint32_t)rf->frame_idx
                );
            }
            fputs("}}", _vdp_cap_data.fp);
        }
        break;
    case VDP_DECODER_PROFILE_VC1_SIMPLE:
    case VDP_DECODER_PROFILE_VC1_MAIN:
    case VDP_DECODER_PROFILE_VC1_ADVANCED:
        {
            VdpPictureInfoVC1 const * picture_info_vc1 =
                (VdpPictureInfoVC1 const *)picture_info;

            fprintf(
                _vdp_cap_data.fp,
                "{%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u}",
                picture_info_vc1->forward_reference,
                picture_info_vc1->backward_reference,
                picture_info_vc1->slice_count,
                (uint32_t)picture_info_vc1->picture_type,
                (uint32_t)picture_info_vc1->frame_coding_mode,
                (uint32_t)picture_info_vc1->postprocflag,
                (uint32_t)picture_info_vc1->pulldown,
                (uint32_t)picture_info_vc1->interlace,
                (uint32_t)picture_info_vc1->tfcntrflag,
                (uint32_t)picture_info_vc1->finterpflag,
                (uint32_t)picture_info_vc1->psf,
                (uint32_t)picture_info_vc1->dquant,
                (uint32_t)picture_info_vc1->panscan_flag,
                (uint32_t)picture_info_vc1->refdist_flag,
                (uint32_t)picture_info_vc1->quantizer,
                (uint32_t)picture_info_vc1->extended_mv,
                (uint32_t)picture_info_vc1->extended_dmv,
                (uint32_t)picture_info_vc1->overlap,
                (uint32_t)picture_info_vc1->vstransform,
                (uint32_t)picture_info_vc1->loopfilter,
                (uint32_t)picture_info_vc1->fastuvmc,
                (uint32_t)picture_info_vc1->range_mapy_flag,
                (uint32_t)picture_info_vc1->range_mapy,
                (uint32_t)picture_info_vc1->range_mapuv_flag,
                (uint32_t)picture_info_vc1->range_mapuv,
                (uint32_t)picture_info_vc1->multires,
                (uint32_t)picture_info_vc1->syncmarker,
                (uint32_t)picture_info_vc1->rangered,
                (uint32_t)picture_info_vc1->maxbframes,
                (uint32_t)picture_info_vc1->deblockEnable,
                (uint32_t)picture_info_vc1->pquant
            );
        }
        break;
    case VDP_DECODER_PROFILE_MPEG4_PART2_SP:
    case VDP_DECODER_PROFILE_MPEG4_PART2_ASP:
    case VDP_DECODER_PROFILE_DIVX4_QMOBILE:
    case VDP_DECODER_PROFILE_DIVX4_MOBILE:
    case VDP_DECODER_PROFILE_DIVX4_HOME_THEATER:
    case VDP_DECODER_PROFILE_DIVX4_HD_1080P:
    case VDP_DECODER_PROFILE_DIVX5_QMOBILE:
    case VDP_DECODER_PROFILE_DIVX5_MOBILE:
    case VDP_DECODER_PROFILE_DIVX5_HOME_THEATER:
    case VDP_DECODER_PROFILE_DIVX5_HD_1080P:
        {
            VdpPictureInfoMPEG4Part2 const * picture_info_mpeg4 =
                (VdpPictureInfoMPEG4Part2 const *)picture_info;

            fprintf(
                _vdp_cap_data.fp,
                "{%u, %u, {%d, %d}, {%d, %d}, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, {",
                picture_info_mpeg4->forward_reference,
                picture_info_mpeg4->backward_reference,
                (int32_t)picture_info_mpeg4->trd[0],
                (int32_t)picture_info_mpeg4->trd[1],
                (int32_t)picture_info_mpeg4->trb[0],
                (int32_t)picture_info_mpeg4->trb[1],
                (uint32_t)picture_info_mpeg4->vop_time_increment_resolution,
                (uint32_t)picture_info_mpeg4->vop_coding_type,
                (uint32_t)picture_info_mpeg4->vop_fcode_forward,
                (uint32_t)picture_info_mpeg4->vop_fcode_backward,
                (uint32_t)picture_info_mpeg4->resync_marker_disable,
                (uint32_t)picture_info_mpeg4->interlaced,
                (uint32_t)picture_info_mpeg4->quant_type,
                (uint32_t)picture_info_mpeg4->quarter_sample,
                (uint32_t)picture_info_mpeg4->short_video_header,
                (uint32_t)picture_info_mpeg4->rounding_control,
                (uint32_t)picture_info_mpeg4->alternate_vertical_scan_flag,
                (uint32_t)picture_info_mpeg4->top_field_first
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_mpeg4->intra_quantizer_matrix); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_mpeg4->intra_quantizer_matrix[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_mpeg4->non_intra_quantizer_matrix); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_mpeg4->non_intra_quantizer_matrix[i]
                );
            }
            fputs("}}", _vdp_cap_data.fp);
        }
        break;
    case VDP_DECODER_PROFILE_HEVC_MAIN:
    case VDP_DECODER_PROFILE_HEVC_MAIN_10:
    case VDP_DECODER_PROFILE_HEVC_MAIN_STILL:
    case VDP_DECODER_PROFILE_HEVC_MAIN_12:
    case VDP_DECODER_PROFILE_HEVC_MAIN_444:
        {
            VdpPictureInfoHEVC const * picture_info_hevc =
                (VdpPictureInfoHEVC const *)picture_info;

            fprintf(
                _vdp_cap_data.fp,
                "{%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, {",
                (uint32_t)picture_info_hevc->chroma_format_idc,
                (uint32_t)picture_info_hevc->separate_colour_plane_flag,
                (uint32_t)picture_info_hevc->pic_width_in_luma_samples,
                (uint32_t)picture_info_hevc->pic_height_in_luma_samples,
                (uint32_t)picture_info_hevc->bit_depth_luma_minus8,
                (uint32_t)picture_info_hevc->bit_depth_chroma_minus8,
                (uint32_t)picture_info_hevc->log2_max_pic_order_cnt_lsb_minus4,
                (uint32_t)picture_info_hevc->sps_max_dec_pic_buffering_minus1,
                (uint32_t)picture_info_hevc->log2_min_luma_coding_block_size_minus3,
                (uint32_t)picture_info_hevc->log2_diff_max_min_luma_coding_block_size,
                (uint32_t)picture_info_hevc->log2_min_transform_block_size_minus2,
                (uint32_t)picture_info_hevc->log2_diff_max_min_transform_block_size,
                (uint32_t)picture_info_hevc->max_transform_hierarchy_depth_inter,
                (uint32_t)picture_info_hevc->max_transform_hierarchy_depth_intra,
                (uint32_t)picture_info_hevc->scaling_list_enabled_flag
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList4x4); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList4x4[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_hevc->ScalingList4x4[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList8x8); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList8x8[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_hevc->ScalingList8x8[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList16x16); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList16x16[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_hevc->ScalingList16x16[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList32x32); ++i) {
                fputs((i == 0) ? "{" : "}, {", _vdp_cap_data.fp);
                for (uint32_t j = 0; j < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingList32x32[0]); ++j) {
                    fprintf(
                        _vdp_cap_data.fp,
                        "%s%u",
                        (j == 0) ? "" : ", ",
                        (uint32_t)picture_info_hevc->ScalingList32x32[i][j]
                    );
                }
            }
            fputs("}}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingListDCCoeff16x16); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->ScalingListDCCoeff16x16[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->ScalingListDCCoeff32x32); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->ScalingListDCCoeff32x32[i]
                );
            }
            fputs("}, ", _vdp_cap_data.fp);
            fprintf(
                _vdp_cap_data.fp,
                "%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, %u, %u, %u, %u, %d, %d, %u, %u, %u, %u, %u, %u, %u, %u, %u, {",
                (uint32_t)picture_info_hevc->amp_enabled_flag,
                (uint32_t)picture_info_hevc->sample_adaptive_offset_enabled_flag,
                (uint32_t)picture_info_hevc->pcm_enabled_flag,
                (uint32_t)picture_info_hevc->pcm_sample_bit_depth_luma_minus1,
                (uint32_t)picture_info_hevc->pcm_sample_bit_depth_chroma_minus1,
                (uint32_t)picture_info_hevc->log2_min_pcm_luma_coding_block_size_minus3,
                (uint32_t)picture_info_hevc->log2_diff_max_min_pcm_luma_coding_block_size,
                (uint32_t)picture_info_hevc->pcm_loop_filter_disabled_flag,
                (uint32_t)picture_info_hevc->num_short_term_ref_pic_sets,
                (uint32_t)picture_info_hevc->long_term_ref_pics_present_flag,
                (uint32_t)picture_info_hevc->num_long_term_ref_pics_sps,
                (uint32_t)picture_info_hevc->sps_temporal_mvp_enabled_flag,
                (uint32_t)picture_info_hevc->strong_intra_smoothing_enabled_flag,
                (uint32_t)picture_info_hevc->dependent_slice_segments_enabled_flag,
                (uint32_t)picture_info_hevc->output_flag_present_flag,
                (uint32_t)picture_info_hevc->num_extra_slice_header_bits,
                (uint32_t)picture_info_hevc->sign_data_hiding_enabled_flag,
                (uint32_t)picture_info_hevc->cabac_init_present_flag,
                (uint32_t)picture_info_hevc->num_ref_idx_l0_default_active_minus1,
                (uint32_t)picture_info_hevc->num_ref_idx_l1_default_active_minus1,
                (int32_t)picture_info_hevc->init_qp_minus26,
                (uint32_t)picture_info_hevc->constrained_intra_pred_flag,
                (uint32_t)picture_info_hevc->transform_skip_enabled_flag,
                (uint32_t)picture_info_hevc->cu_qp_delta_enabled_flag,
                (uint32_t)picture_info_hevc->diff_cu_qp_delta_depth,
                (int32_t)picture_info_hevc->pps_cb_qp_offset,
                (int32_t)picture_info_hevc->pps_cr_qp_offset,
                (uint32_t)picture_info_hevc->pps_slice_chroma_qp_offsets_present_flag,
                (uint32_t)picture_info_hevc->weighted_pred_flag,
                (uint32_t)picture_info_hevc->weighted_bipred_flag,
                (uint32_t)picture_info_hevc->transquant_bypass_enabled_flag,
                (uint32_t)picture_info_hevc->tiles_enabled_flag,
                (uint32_t)picture_info_hevc->entropy_coding_sync_enabled_flag,
                (uint32_t)picture_info_hevc->num_tile_columns_minus1,
                (uint32_t)picture_info_hevc->num_tile_rows_minus1,
                (uint32_t)picture_info_hevc->uniform_spacing_flag
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->column_width_minus1); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->column_width_minus1[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->row_height_minus1); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->row_height_minus1[i]
                );
            }
            fputs("}, ", _vdp_cap_data.fp);
            fprintf(
                _vdp_cap_data.fp,
                "%u, %u, %u, %u, %u, %d, %d, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, {",
                (uint32_t)picture_info_hevc->loop_filter_across_tiles_enabled_flag,
                (uint32_t)picture_info_hevc->pps_loop_filter_across_slices_enabled_flag,
                (uint32_t)picture_info_hevc->deblocking_filter_control_present_flag,
                (uint32_t)picture_info_hevc->deblocking_filter_override_enabled_flag,
                (uint32_t)picture_info_hevc->pps_deblocking_filter_disabled_flag,
                (int32_t)picture_info_hevc->pps_beta_offset_div2,
                (int32_t)picture_info_hevc->pps_tc_offset_div2,
                (uint32_t)picture_info_hevc->lists_modification_present_flag,
                (uint32_t)picture_info_hevc->log2_parallel_merge_level_minus2,
                (uint32_t)picture_info_hevc->slice_segment_header_extension_present_flag,
                (uint32_t)picture_info_hevc->IDRPicFlag,
                (uint32_t)picture_info_hevc->RAPPicFlag,
                (uint32_t)picture_info_hevc->CurrRpsIdx,
                (uint32_t)picture_info_hevc->NumPocTotalCurr,
                (uint32_t)picture_info_hevc->NumDeltaPocsOfRefRpsIdx,
                (uint32_t)picture_info_hevc->NumShortTermPictureSliceHeaderBits,
                (uint32_t)picture_info_hevc->NumLongTermPictureSliceHeaderBits,
                (int32_t)picture_info_hevc->CurrPicOrderCntVal
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->RefPics); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->RefPics[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->PicOrderCntVal); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%d",
                    (i == 0) ? "" : ", ",
                    (int32_t)picture_info_hevc->PicOrderCntVal[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->IsLongTerm); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->IsLongTerm[i]
                );
            }
            fputs("}, ", _vdp_cap_data.fp);
            fprintf(
                _vdp_cap_data.fp,
                "%u, %u, %u, {",
                (uint32_t)picture_info_hevc->NumPocStCurrBefore,
                (uint32_t)picture_info_hevc->NumPocStCurrAfter,
                (uint32_t)picture_info_hevc->NumPocLtCurr
            );
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->RefPicSetStCurrBefore); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->RefPicSetStCurrBefore[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->RefPicSetStCurrAfter); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->RefPicSetStCurrAfter[i]
                );
            }
            fputs("}, {", _vdp_cap_data.fp);
            for (uint32_t i = 0; i < _VDP_TRACE_ARSIZE(picture_info_hevc->RefPicSetLtCurr); ++i) {
                fprintf(
                    _vdp_cap_data.fp,
                    "%s%u",
                    (i == 0) ? "" : ", ",
                    (uint32_t)picture_info_hevc->RefPicSetLtCurr[i]
                );
            }
            fputs("}}", _vdp_cap_data.fp);
        }
        break;
    default:
        fputs("{...}", _vdp_cap_data.fp);
        break;
    }
}

static void _vdp_cap_dump_video_mixer_parameter_value(
    VdpVideoMixerParameter parameter,
    void const *           value
)
{
    switch (parameter) {
    case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH:
    case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT:
    case VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE:
    case VDP_VIDEO_MIXER_PARAMETER_LAYERS:
        if (!value) {
            fputs("???", _vdp_cap_data.fp);
        }
        else {
            fprintf(_vdp_cap_data.fp, "%u", *(uint32_t const *)value);
        }
        break;
    default:
        fputs("???", _vdp_cap_data.fp);
        break;
    }
}

static void _vdp_cap_dump_video_mixer_attribute_value(
    VdpVideoMixerAttribute attribute,
    void const *           value,
    bool                   get_operation
)
{
    if (!value) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    switch (attribute) {
    case VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR:
        _vdp_cap_dump_color((VdpColor const *)value);
        break;
    case VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX:
        {
            VdpCSCMatrix const * ptr;

            // For some objects, the "default" value is a NULL pointer.
            // So, VdpVideoMixerGetAttributeValues expects a double pointer to
            // the value, so it can either fill in the value, or NULL out the
            // pointer.
            if (get_operation) {
                ptr = *(VdpCSCMatrix const * const *)value;
            }
            else {
                ptr = (VdpCSCMatrix const *)value;
            }
            _vdp_cap_dump_csc_matrix(ptr);
        }
        break;
    case VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL:
    case VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL:
    case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA:
    case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA:
        fprintf(_vdp_cap_data.fp, "%f", *(float const *)value);
        break;
    case VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE:
        fprintf(_vdp_cap_data.fp, "%u", *(uint8_t const *)value);
        break;
    default:
        fputs("???", _vdp_cap_data.fp);
        break;
    }
}

static void _vdp_cap_dump_uint8_t_stream(
    uint32_t        count,
    uint8_t const * values
)
{
    if (!values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
            "%02x",
            values[0]
        );

        --count;
        ++values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_uint32_t_stream(
    uint32_t         count,
    uint32_t const * values
)
{
    if (!values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
            "%08x%s",
            values[0],
            (count > 1) ? " " : ""
        );

        --count;
        ++values;
    }
    fputs("}", _vdp_cap_data.fp);
}

struct _VdpcapPlane {
    void const * data;
    uint32_t     pitch;
    uint32_t     item_count;
    uint32_t     item_size;
    uint32_t     lines;
};

typedef void _VdpcapPlaneDumper(uint32_t count, void const * values);

static void _vdp_cap_dump_plane_list(
    uint32_t             plane_count,
    _VdpcapPlane const * planes
)
{
    if (!planes) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (plane_count) {
        uint32_t lines = planes[0].lines;

        _VdpcapPlaneDumper * dumper;
        if (planes[0].item_size == 4) {
            dumper = (_VdpcapPlaneDumper*)_vdp_cap_dump_uint32_t_stream;
        }
        else {
            dumper = (_VdpcapPlaneDumper*)_vdp_cap_dump_uint8_t_stream;
        }

        fputs("{", _vdp_cap_data.fp);
        uint8_t const * ptr = (uint8_t const *)planes[0].data;
        while (lines) {
            dumper(planes[0].item_count, ptr);
            if (lines > 1) {
                fputs(", ", _vdp_cap_data.fp);
            }

            ptr += planes[0].pitch;
            --lines;
        }
        fputs("}", _vdp_cap_data.fp);

        if (plane_count > 1) {
            fputs(", ", _vdp_cap_data.fp);
        }

        --plane_count;
        ++planes;
    }
    fputs("}", _vdp_cap_data.fp);
}

static bool _vdp_cap_init_planes_for_ycbcr_format(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    VdpYCbCrFormat format,
    uint32_t       region_width,
    uint32_t       region_height
)
{
    switch (format) {
    case VDP_YCBCR_FORMAT_NV12:
        if (*plane_count < 2) {
            return false;
        }
        *plane_count = 2;
        planes[0].item_size = 1;
        planes[0].item_count = region_width;
        planes[0].lines = region_height;
        planes[1].item_size = 1;
        planes[1].item_count = region_width;
        planes[1].lines = region_height / 2;
        break;
    case VDP_YCBCR_FORMAT_YV12:
        if (*plane_count < 3) {
            return false;
        }
        *plane_count = 3;
        planes[0].item_size = 1;
        planes[0].item_count = region_width;
        planes[0].lines = region_height;
        planes[1].item_size = 1;
        planes[1].item_count = region_width / 2;
        planes[1].lines = region_height / 2;
        planes[2].item_size = 1;
        planes[2].item_count = region_width / 2;
        planes[2].lines = region_height / 2;
        break;
    case VDP_YCBCR_FORMAT_UYVY:
    case VDP_YCBCR_FORMAT_YUYV:
        if (*plane_count < 1) {
            return false;
        }
        *plane_count = 1;
        planes[0].item_size = 1;
        planes[0].item_count = region_width * 2;
        planes[0].lines = region_height;
        break;
    case VDP_YCBCR_FORMAT_Y8U8V8A8:
    case VDP_YCBCR_FORMAT_V8U8Y8A8:
        if (*plane_count < 1) {
            return false;
        }
        *plane_count = 1;
        planes[0].item_size = 4;
        planes[0].item_count = region_width;
        planes[0].lines = region_height;
        break;
    default:
        return false;
    }

    return true;
}

static bool _vdp_cap_init_planes_for_rgba_format(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    VdpRGBAFormat  format,
    uint32_t       region_width,
    uint32_t       region_height
)
{
    switch (format) {
    case VDP_RGBA_FORMAT_B8G8R8A8:
    case VDP_RGBA_FORMAT_R8G8B8A8:
    case VDP_RGBA_FORMAT_R10G10B10A2:
    case VDP_RGBA_FORMAT_B10G10R10A2:
        if (*plane_count < 1) {
            return false;
        }
        *plane_count = 1;
        planes[0].item_size = 4;
        break;
    case VDP_RGBA_FORMAT_A8:
        if (*plane_count < 1) {
            return false;
        }
        *plane_count = 1;
        planes[0].item_size = 1;
        break;
    default:
        return false;
    }
    planes[0].item_count = region_width;
    planes[0].lines = region_height;

    return true;
}

static bool _vdp_cap_init_planes_for_indexed_format(
    uint32_t *       plane_count,
    _VdpcapPlane *   planes,
    VdpIndexedFormat format,
    uint32_t         region_width,
    uint32_t         region_height
)
{
    uint32_t width_multiplier;

    switch (format) {
    case VDP_INDEXED_FORMAT_A4I4:
    case VDP_INDEXED_FORMAT_I4A4:
        width_multiplier = 1;
        break;
    case VDP_INDEXED_FORMAT_A8I8:
    case VDP_INDEXED_FORMAT_I8A8:
        width_multiplier = 2;
        break;
    default:
        return false;
    }

    if (*plane_count < 1) {
        return false;
    }
    *plane_count = 1;
    planes[0].item_size = 1;
    planes[0].item_count = region_width * width_multiplier;
    planes[0].lines = region_height;

    return true;
}

static bool _vdp_cap_init_planes_adapt_format_bits_ycbcr(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    uint32_t       surface_format,
    uint32_t       bits_format,
    uint32_t       region_width,
    uint32_t       region_height
)
{
    return _vdp_cap_init_planes_for_ycbcr_format(
        plane_count,
        planes,
        bits_format,
        region_width,
        region_height
    );
}

static bool _vdp_cap_init_planes_adapt_format_surface_rgba(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    uint32_t       surface_format,
    uint32_t       bits_format,
    uint32_t       region_width,
    uint32_t       region_height
)
{
    return _vdp_cap_init_planes_for_rgba_format(
        plane_count,
        planes,
        surface_format,
        region_width,
        region_height
    );
}

static bool _vdp_cap_init_planes_adapt_format_bits_indexed(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    uint32_t       surface_format,
    uint32_t       bits_format,
    uint32_t       region_width,
    uint32_t       region_height
)
{
    return _vdp_cap_init_planes_for_indexed_format(
        plane_count,
        planes,
        bits_format,
        region_width,
        region_height
    );
}

static bool _vdp_cap_init_planes_adapt_surface_video(
    uint32_t   surface,
    uint32_t * surface_format,
    uint32_t * width,
    uint32_t * height
)
{
    VdpStatus ret;

    VdpChromaType chroma_type;
    ret = _vdp_cap_data.vdp_video_surface_get_parameters(
        surface,
        &chroma_type,
        width,
        height
    );
    if (ret != VDP_STATUS_OK) {
        return false;
    }

    *surface_format = chroma_type;

    return true;
}

static bool _vdp_cap_init_planes_adapt_surface_output(
    uint32_t   surface,
    uint32_t * surface_format,
    uint32_t * width,
    uint32_t * height
)
{
    VdpStatus ret;

    VdpRGBAFormat rgba_format;
    ret = _vdp_cap_data.vdp_output_surface_get_parameters(
        surface,
        &rgba_format,
        width,
        height
    );
    if (ret != VDP_STATUS_OK) {
        return false;
    }

    *surface_format = rgba_format;

    return true;
}

static bool _vdp_cap_init_planes_adapt_surface_bitmap(
    uint32_t   surface,
    uint32_t * surface_format,
    uint32_t * width,
    uint32_t * height
)
{
    VdpStatus ret;

    VdpRGBAFormat rgba_format;
    VdpBool       frequently_accessed;
    ret = _vdp_cap_data.vdp_bitmap_surface_get_parameters(
        surface,
        &rgba_format,
        width,
        height,
        &frequently_accessed
    );
    if (ret != VDP_STATUS_OK) {
        return false;
    }

    *surface_format = rgba_format;

    return true;
}

typedef bool _Vdpcap_Init_Planes_Adapt_Format(
    uint32_t *     plane_count,
    _VdpcapPlane * planes,
    uint32_t       surface_format,
    uint32_t       bits_format,
    uint32_t       region_width,
    uint32_t       region_height
);

typedef bool _Vdpcap_Init_Planes_Adapt_Surface(
    uint32_t   surface,
    uint32_t * surface_format,
    uint32_t * width,
    uint32_t * height
);

static bool _vdp_cap_init_planes(
    uint32_t                            surface,
    void const * const *                source_data,
    uint32_t const *                    source_pitches,
    VdpRect const *                     destination_rect,
    uint32_t *                          plane_count,
    _VdpcapPlane *                      planes,
    _Vdpcap_Init_Planes_Adapt_Surface * adapt_surface,
    _Vdpcap_Init_Planes_Adapt_Format *  adapt_format,
    uint32_t                            adapt_format_bits_format
)
{
    bool ok;

    if (!source_data || !source_pitches) {
        return false;
    }

    if (_vdp_cap_data.level < LEVEL_PARAMS) {
        return false;
    }

    uint32_t surface_format;
    uint32_t width;
    uint32_t height;
    ok = adapt_surface(
        surface,
        &surface_format,
        &width,
        &height
    );
    if (!ok) {
        return false;
    }

    if (destination_rect) {
        width = delta<uint32_t>(destination_rect->x0, destination_rect->x1);
        height = delta<uint32_t>(destination_rect->y0, destination_rect->y1);
    }

    ok = adapt_format(
        plane_count,
        planes,
        surface_format,
        adapt_format_bits_format,
        width,
        height
    );
    if (!ok) {
        return false;
    }

    for (uint32_t i = 0; i < *plane_count; ++i) {
        planes[i].data = source_data[i];
        planes[i].pitch = source_pitches[i];
    }

    return true;
}

static void _vdp_cap_dump_bool_list(
    uint32_t        count,
    VdpBool const * values
)
{
    if (!values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
            "%d%s",
            (int)values[0],
            (count > 1) ? ", " : ""
        );

        --count;
        ++values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_void_pointer_list(
    uint32_t             count,
    void const * const * values,
    bool                 zero_count_question_marks
)
{
    if (!values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    if (!count && zero_count_question_marks) {
        fputs("???", _vdp_cap_data.fp);
    }
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
            "%p%s",
            values[0],
            (count > 1) ? ", " : ""
        );

        --count;
        ++values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_uint32_t_list(
    uint32_t         count,
    uint32_t const * values,
    bool             zero_count_question_marks
)
{
    if (!values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    if (!count && zero_count_question_marks) {
        fputs("???", _vdp_cap_data.fp);
    }
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
            "%u%s",
            values[0],
            (count > 1) ? ", " : ""
        );

        --count;
        ++values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_color_list(
    uint32_t         count,
    VdpColor const * colors
)
{
    if (!colors) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (count) {
        _vdp_cap_dump_color(&colors[0]);
        fputs(
            (count > 1) ? ", " : "",
            _vdp_cap_data.fp
        );

        --count;
        ++colors;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_color_table(
    VdpIndexedFormat    indexed_format,
    VdpColorTableFormat format,
    void const *        table
)
{
    if (!table) {
        fprintf(_vdp_cap_data.fp, "NULL");
        return;
    }

    uint32_t count;
    switch (indexed_format) {
    case VDP_INDEXED_FORMAT_A4I4:
    case VDP_INDEXED_FORMAT_I4A4:
        count = 1 << 4;
        break;
    case VDP_INDEXED_FORMAT_A8I8:
    case VDP_INDEXED_FORMAT_I8A8:
        count = 1 << 8;
        break;
    default:
        fprintf(_vdp_cap_data.fp, "???");
        return;
    }

    switch (format) {
    case VDP_COLOR_TABLE_FORMAT_B8G8R8X8:
        break;
    default:
        fprintf(_vdp_cap_data.fp, "???");
        return;
    }

    _vdp_cap_dump_uint32_t_list(
        count,
        (uint32_t const *)table,
        true
    );
}

static void _vdp_cap_dump_bitstream_buffer_list(
    uint32_t                   count,
    VdpBitstreamBuffer const * buffers
)
{
    if (!buffers) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (count) {
        fprintf(
            _vdp_cap_data.fp,
             "{(ver %d)%s %u, ",
            buffers[0].struct_version,
            (buffers[0].struct_version > 0)
                ? "(unsupported; cannot dump all fields)"
                : "",
            buffers[0].bitstream_bytes
        );
        if (_vdp_cap_data.level >= LEVEL_DATA) {
            const uint8_t * ptr = (const uint8_t * )buffers[0].bitstream;
            for (uint32_t i = 0; i < buffers[0].bitstream_bytes; ++i) {
                fprintf(_vdp_cap_data.fp, "%02x ", ptr[i]);
            }
        }
        else {
            fputs("...", _vdp_cap_data.fp);
        }
        fputs(
            (count > 1) ? "}, " : "}",
            _vdp_cap_data.fp
        );

        --count;
        ++buffers;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_video_mixer_feature_list(
    uint32_t                     feature_count,
    VdpVideoMixerFeature const * features
)
{
    _vdp_cap_dump_uint32_t_list(
        feature_count,
        features,
        false
    );
}

static void _vdp_cap_dump_video_mixer_parameter_list(
    uint32_t                       parameter_count,
    VdpVideoMixerParameter const * parameters
)
{
    _vdp_cap_dump_uint32_t_list(
        parameter_count,
        parameters,
        false
    );
}

static void _vdp_cap_dump_video_mixer_attribute_list(
    uint32_t                       attribute_count,
    VdpVideoMixerAttribute const * attributes
)
{
    _vdp_cap_dump_uint32_t_list(
        attribute_count,
        attributes,
        false
    );
}

static void _vdp_cap_dump_video_mixer_parameter_value_list(
    uint32_t                       parameter_count,
    VdpVideoMixerParameter const * parameters,
    void const * const *           parameter_values
)
{
    if (!parameters || !parameter_values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (parameter_count) {
        _vdp_cap_dump_video_mixer_parameter_value(parameters[0], parameter_values[0]);
        fputs((parameter_count > 1) ? ", " : "", _vdp_cap_data.fp);

        --parameter_count;
        ++parameters;
        ++parameter_values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_video_mixer_attribute_value_list(
    uint32_t                       attribute_count,
    VdpVideoMixerAttribute const * attributes,
    void const * const *           attribute_values,
    bool                           get_operation
)
{
    if (!attributes || !attribute_values) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (attribute_count) {
        _vdp_cap_dump_video_mixer_attribute_value(
            attributes[0],
            attribute_values[0],
            get_operation
        );
        fputs((attribute_count > 1) ? ", " : "", _vdp_cap_data.fp);

        --attribute_count;
        ++attributes;
        ++attribute_values;
    }
    fputs("}", _vdp_cap_data.fp);
}

static void _vdp_cap_dump_layers_list(
    uint32_t         layer_count,
    VdpLayer const * layers
)
{
    if (!layers) {
        fputs("NULL", _vdp_cap_data.fp);
        return;
    }

    fputs("{", _vdp_cap_data.fp);
    while (layer_count) {
        fprintf(
            _vdp_cap_data.fp,
            "{(ver %d)%s %u,",
            layers[0].struct_version,
            (layers[0].struct_version > 0)
                ? "(unsupported; cannot dump all fields)"
                : "",
            layers[0].source_surface
        );

        _vdp_cap_dump_rect(layers[0].source_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(layers[0].destination_rect);

        fputs((layer_count > 1) ? "}, " : "}", _vdp_cap_data.fp);

        --layer_count;
        ++layers;
    }
    fputs("}", _vdp_cap_data.fp);
}

static char const * _vdp_cap_get_error_string(
    VdpStatus status
)
{
    char const * ret;

    fputs("vdp_get_error_string(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%d",
            status
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_get_error_string(
        status
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        if (ret) {
            fprintf(_vdp_cap_data.fp, "    -> '%s'\n", ret);
        }
        else {
            fprintf(_vdp_cap_data.fp, "    -> NULL\n");
        }
    }

    return ret;
}

static VdpStatus _vdp_cap_get_api_version(
    /* output parameters follow */
    uint32_t * api_version
)
{
    VdpStatus ret;

    fputs("vdp_get_api_version(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%s",
            api_version ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_get_api_version(
        api_version
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (api_version) {
                fprintf(_vdp_cap_data.fp, ", %u", *api_version);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_get_information_string(
    /* output parameters follow */
    char const * * information_string
)
{
    VdpStatus ret;

    fputs("vdp_get_information_string(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%s",
            information_string ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_get_information_string(
        information_string
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (information_string && *information_string) {
                fprintf(_vdp_cap_data.fp, ", \"%s\"", *information_string);
            }
            else if (information_string) {
                fputs(", (null)", _vdp_cap_data.fp);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_device_destroy(
    VdpDevice device
)
{
    VdpStatus ret;

    fputs("vdp_device_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            device
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_device_destroy(
        device
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_generate_csc_matrix(
    VdpProcamp *     procamp,
    VdpColorStandard standard,
    /* output parameters follow */
    VdpCSCMatrix *   csc_matrix
)
{
    VdpStatus ret;

    fputs("vdp_generate_csc_matrix(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        _vdp_cap_dump_procamp(procamp);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, %s",
            standard,
            csc_matrix ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_generate_csc_matrix(
        procamp,
        standard,
        csc_matrix
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (csc_matrix) {
                fputs(", ", _vdp_cap_data.fp);
                _vdp_cap_dump_csc_matrix(csc_matrix);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_query_capabilities(
    VdpDevice     device,
    VdpChromaType surface_chroma_type,
    /* output parameters follow */
    VdpBool *     is_supported,
    uint32_t *    max_width,
    uint32_t *    max_height
)
{
    VdpStatus ret;

    fputs("vdp_video_surface_query_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s, %s",
            device,
            surface_chroma_type,
            is_supported ? "-" : "NULL",
            max_width ? "-" : "NULL",
            max_height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_query_capabilities(
        device,
        surface_chroma_type,
        is_supported,
        max_width,
        max_height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_width) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_height) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_query_get_put_bits_y_cb_cr_capabilities(
    VdpDevice      device,
    VdpChromaType  surface_chroma_type,
    VdpYCbCrFormat bits_ycbcr_format,
    /* output parameters follow */
    VdpBool *      is_supported
)
{
    VdpStatus ret;

    fputs("vdp_video_surface_query_get_put_bits_y_cb_cr_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %s",
            device,
            surface_chroma_type,
            bits_ycbcr_format,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_query_get_put_bits_y_cb_cr_capabilities(
        device,
        surface_chroma_type,
        bits_ycbcr_format,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_create(
    VdpDevice         device,
    VdpChromaType     chroma_type,
    uint32_t          width,
    uint32_t          height,
    /* output parameters follow */
    VdpVideoSurface * surface
)
{
    VdpStatus ret;

    fputs("vdp_video_surface_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %s",
            device,
            chroma_type,
            width,
            height,
            surface ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_create(
        device,
        chroma_type,
        width,
        height,
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (surface) {
                fprintf(_vdp_cap_data.fp, ", %u", *surface);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_destroy(
    VdpVideoSurface surface
)
{
    VdpStatus ret;

    fputs("vdp_video_surface_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            surface
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_destroy(
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_get_parameters(
    VdpVideoSurface surface,
    /* output parameters follow */
    VdpChromaType * chroma_type,
    uint32_t *      width,
    uint32_t *      height
)
{
    VdpStatus ret;

    fputs("vdp_video_surface_get_parameters(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s, %s, %s",
            surface,
            chroma_type ? "-" : "NULL",
            width ? "-" : "NULL",
            height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_get_parameters(
        surface,
        chroma_type,
        width,
        height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (chroma_type) {
                fprintf(_vdp_cap_data.fp, ", %u", *chroma_type);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (width) {
                fprintf(_vdp_cap_data.fp, ", %u", *width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (height) {
                fprintf(_vdp_cap_data.fp, ", %u", *height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_get_bits_y_cb_cr(
    VdpVideoSurface  surface,
    VdpYCbCrFormat   destination_ycbcr_format,
    void * const *   destination_data,
    uint32_t const * destination_pitches
)
{
    VdpStatus ret;

    _VdpcapPlane planes[3];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        destination_data,
        destination_pitches,
        0,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_video,
        _vdp_cap_init_planes_adapt_format_bits_ycbcr,
        destination_ycbcr_format
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_video_surface_get_bits_y_cb_cr(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            surface,
            destination_ycbcr_format
        );
        _vdp_cap_dump_void_pointer_list(plane_count, destination_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, destination_pitches, true);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_surface_get_bits_y_cb_cr(
        surface,
        destination_ycbcr_format,
        destination_data,
        destination_pitches
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_surface_put_bits_y_cb_cr(
    VdpVideoSurface      surface,
    VdpYCbCrFormat       source_ycbcr_format,
    void const * const * source_data,
    uint32_t const *     source_pitches
)
{
    VdpStatus ret;

    _VdpcapPlane planes[3];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        source_data,
        source_pitches,
        0,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_video,
        _vdp_cap_init_planes_adapt_format_bits_ycbcr,
        source_ycbcr_format
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_video_surface_put_bits_y_cb_cr(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            surface,
            source_ycbcr_format
        );
        _vdp_cap_dump_void_pointer_list(plane_count, source_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, source_pitches, true);
        fputs(", ", _vdp_cap_data.fp);
    }
    fputs(")\n", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    ret = _vdp_cap_data.vdp_video_surface_put_bits_y_cb_cr(
        surface,
        source_ycbcr_format,
        source_data,
        source_pitches
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_query_capabilities(
    VdpDevice     device,
    VdpRGBAFormat surface_rgba_format,
    /* output parameters follow */
    VdpBool *     is_supported,
    uint32_t *    max_width,
    uint32_t *    max_height
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_query_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s, %s",
            device,
            surface_rgba_format,
            is_supported ? "-" : "NULL",
            max_width ? "-" : "NULL",
            max_height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_query_capabilities(
        device,
        surface_rgba_format,
        is_supported,
        max_width,
        max_height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_width) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_height) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_query_get_put_bits_native_capabilities(
    VdpDevice     device,
    VdpRGBAFormat surface_rgba_format,
    /* output parameters follow */
    VdpBool *     is_supported
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_query_get_put_bits_native_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            surface_rgba_format,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_query_get_put_bits_native_capabilities(
        device,
        surface_rgba_format,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_query_put_bits_indexed_capabilities(
    VdpDevice           device,
    VdpRGBAFormat       surface_rgba_format,
    VdpIndexedFormat    bits_indexed_format,
    VdpColorTableFormat color_table_format,
    /* output parameters follow */
    VdpBool *           is_supported
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_query_put_bits_indexed_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %s",
            device,
            surface_rgba_format,
            bits_indexed_format,
            color_table_format,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_query_put_bits_indexed_capabilities(
        device,
        surface_rgba_format,
        bits_indexed_format,
        color_table_format,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_query_put_bits_y_cb_cr_capabilities(
    VdpDevice      device,
    VdpRGBAFormat  surface_rgba_format,
    VdpYCbCrFormat bits_ycbcr_format,
    /* output parameters follow */
    VdpBool *      is_supported
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_query_put_bits_y_cb_cr_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %s",
            device,
            surface_rgba_format,
            bits_ycbcr_format,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_query_put_bits_y_cb_cr_capabilities(
        device,
        surface_rgba_format,
        bits_ycbcr_format,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_create(
    VdpDevice          device,
    VdpRGBAFormat      rgba_format,
    uint32_t           width,
    uint32_t           height,
    /* output parameters follow */
    VdpOutputSurface * surface
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %s",
            device,
            rgba_format,
            width,
            height,
            surface ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_create(
        device,
        rgba_format,
        width,
        height,
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (surface) {
                fprintf(_vdp_cap_data.fp, ", %u", *surface);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_destroy(
    VdpOutputSurface surface
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            surface
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_destroy(
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_get_parameters(
    VdpOutputSurface surface,
    /* output parameters follow */
    VdpRGBAFormat *  rgba_format,
    uint32_t *       width,
    uint32_t *       height
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_get_parameters(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s, %s, %s",
            surface,
            rgba_format ? "-" : "NULL",
            width ? "-" : "NULL",
            height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_get_parameters(
        surface,
        rgba_format,
        width,
        height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (rgba_format) {
                fprintf(_vdp_cap_data.fp, ", %u", *rgba_format);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (width) {
                fprintf(_vdp_cap_data.fp, ", %u", *width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (height) {
                fprintf(_vdp_cap_data.fp, ", %u", *height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_get_bits_native(
    VdpOutputSurface  surface,
    VdpRect const *   source_rect,
    void * const *    destination_data,
    uint32_t const *  destination_pitches
)
{
    VdpStatus ret;

    _VdpcapPlane planes[1];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        destination_data,
        destination_pitches,
        source_rect,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_output,
        _vdp_cap_init_planes_adapt_format_surface_rgba,
        0
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_output_surface_get_bits_native(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            surface
        );
        _vdp_cap_dump_rect(source_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_void_pointer_list(plane_count, destination_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, destination_pitches, true);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_get_bits_native(
        surface,
        source_rect,
        destination_data,
        destination_pitches
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_put_bits_native(
    VdpOutputSurface     surface,
    void const * const * source_data,
    uint32_t const *     source_pitches,
    VdpRect const *      destination_rect
)
{
    VdpStatus ret;

    _VdpcapPlane planes[1];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        source_data,
        source_pitches,
        destination_rect,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_output,
        _vdp_cap_init_planes_adapt_format_surface_rgba,
        0
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_output_surface_put_bits_native(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            surface
        );
        _vdp_cap_dump_void_pointer_list(plane_count, source_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, source_pitches, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(destination_rect);
    }
    fputs(")\n", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    ret = _vdp_cap_data.vdp_output_surface_put_bits_native(
        surface,
        source_data,
        source_pitches,
        destination_rect
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_put_bits_indexed(
    VdpOutputSurface     surface,
    VdpIndexedFormat     source_indexed_format,
    void const * const * source_data,
    uint32_t const *     source_pitches,
    VdpRect const *      destination_rect,
    VdpColorTableFormat  color_table_format,
    void const *         color_table
)
{
    VdpStatus ret;

    _VdpcapPlane planes[1];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        source_data,
        source_pitches,
        destination_rect,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_output,
        _vdp_cap_init_planes_adapt_format_bits_indexed,
        source_indexed_format
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_output_surface_put_bits_indexed(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            surface,
            source_indexed_format
        );
        _vdp_cap_dump_void_pointer_list(plane_count, source_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, source_pitches, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(destination_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            color_table_format
        );
        _vdp_cap_dump_color_table(
            source_indexed_format,
            color_table_format,
            color_table
        );
    }
    fputs(")\n", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    ret = _vdp_cap_data.vdp_output_surface_put_bits_indexed(
        surface,
        source_indexed_format,
        source_data,
        source_pitches,
        destination_rect,
        color_table_format,
        color_table
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_put_bits_y_cb_cr(
    VdpOutputSurface     surface,
    VdpYCbCrFormat       source_ycbcr_format,
    void const * const * source_data,
    uint32_t const *     source_pitches,
    VdpRect const *      destination_rect,
    VdpCSCMatrix const * csc_matrix
)
{
    VdpStatus ret;

    _VdpcapPlane planes[1];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        source_data,
        source_pitches,
        destination_rect,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_output,
        _vdp_cap_init_planes_adapt_format_bits_ycbcr,
        source_ycbcr_format
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_output_surface_put_bits_y_cb_cr(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            surface,
            source_ycbcr_format
        );
        _vdp_cap_dump_void_pointer_list(plane_count, source_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, source_pitches, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(destination_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_csc_matrix(csc_matrix);
    }
    fputs(")\n", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    ret = _vdp_cap_data.vdp_output_surface_put_bits_y_cb_cr(
        surface,
        source_ycbcr_format,
        source_data,
        source_pitches,
        destination_rect,
        csc_matrix
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_bitmap_surface_query_capabilities(
    VdpDevice     device,
    VdpRGBAFormat surface_rgba_format,
    /* output parameters follow */
    VdpBool *     is_supported,
    uint32_t *    max_width,
    uint32_t *    max_height
)
{
    VdpStatus ret;

    fputs("vdp_bitmap_surface_query_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s, %s",
            device,
            surface_rgba_format,
            is_supported ? "-" : "NULL",
            max_width ? "-" : "NULL",
            max_height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_bitmap_surface_query_capabilities(
        device,
        surface_rgba_format,
        is_supported,
        max_width,
        max_height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_width) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_height) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_bitmap_surface_create(
    VdpDevice         device,
    VdpRGBAFormat     rgba_format,
    uint32_t          width,
    uint32_t          height,
    VdpBool           frequently_accessed,
    /* output parameters follow */
    VdpBitmapSurface * surface
)
{
    VdpStatus ret;

    fputs("vdp_bitmap_surface_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %d, %s",
            device,
            rgba_format,
            width,
            height,
            frequently_accessed,
            surface ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_bitmap_surface_create(
        device,
        rgba_format,
        width,
        height,
        frequently_accessed,
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (surface) {
                fprintf(_vdp_cap_data.fp, ", %u", *surface);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_bitmap_surface_destroy(
    VdpBitmapSurface surface
)
{
    VdpStatus ret;

    fputs("vdp_bitmap_surface_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            surface
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_bitmap_surface_destroy(
        surface
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_bitmap_surface_get_parameters(
    VdpBitmapSurface surface,
    /* output parameters follow */
    VdpRGBAFormat * rgba_format,
    uint32_t *      width,
    uint32_t *      height,
    VdpBool *       frequently_accessed
)
{
    VdpStatus ret;

    fputs("vdp_bitmap_surface_get_parameters(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s, %s, %s, %s",
            surface,
            rgba_format ? "-" : "NULL",
            width ? "-" : "NULL",
            height ? "-" : "NULL",
            frequently_accessed ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_bitmap_surface_get_parameters(
        surface,
        rgba_format,
        width,
        height,
        frequently_accessed
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (rgba_format) {
                fprintf(_vdp_cap_data.fp, ", %u", *rgba_format);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (width) {
                fprintf(_vdp_cap_data.fp, ", %u", *width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (height) {
                fprintf(_vdp_cap_data.fp, ", %u", *height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (frequently_accessed) {
                fprintf(_vdp_cap_data.fp, ", %d", *frequently_accessed);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_bitmap_surface_put_bits_native(
    VdpBitmapSurface     surface,
    void const * const * source_data,
    uint32_t const *     source_pitches,
    VdpRect const *      destination_rect
)
{
    VdpStatus ret;

    _VdpcapPlane planes[1];
    uint32_t    plane_count = _VDP_TRACE_ARSIZE(planes);
    bool dump_data = _vdp_cap_init_planes(
        surface,
        source_data,
        source_pitches,
        destination_rect,
        &plane_count,
        planes,
        _vdp_cap_init_planes_adapt_surface_bitmap,
        _vdp_cap_init_planes_adapt_format_surface_rgba,
        0
    );
    if (!dump_data) {
        plane_count = 0;
    }

    fputs("vdp_bitmap_surface_put_bits_native(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            surface
        );
        _vdp_cap_dump_void_pointer_list(plane_count, source_data, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_uint32_t_list(plane_count, source_pitches, true);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(destination_rect);
    }
    fputs(")\n", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_DATA) {
        fputs("    ... Data: ", _vdp_cap_data.fp);
        if (dump_data) {
            _vdp_cap_dump_plane_list(plane_count, planes);
        }
        else {
           fputs("???", _vdp_cap_data.fp);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    ret = _vdp_cap_data.vdp_bitmap_surface_put_bits_native(
        surface,
        source_data,
        source_pitches,
        destination_rect
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_render_output_surface(
    VdpOutputSurface                         destination_surface,
    VdpRect const *                          destination_rect,
    VdpOutputSurface                         source_surface,
    VdpRect const *                          source_rect,
    VdpColor const *                         colors,
    VdpOutputSurfaceRenderBlendState const * blend_state,
    uint32_t                                 flags
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_render_output_surface(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            destination_surface
        );
        _vdp_cap_dump_rect(destination_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            source_surface
        );
        _vdp_cap_dump_rect(source_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_color_list((flags & VDP_OUTPUT_SURFACE_RENDER_COLOR_PER_VERTEX) ? 4 : 1, colors);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_blend_state(blend_state);
        fprintf(
            _vdp_cap_data.fp,
            ", %u",
            flags
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_render_output_surface(
        destination_surface,
        destination_rect,
        source_surface,
        source_rect,
        colors,
        blend_state,
        flags
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_output_surface_render_bitmap_surface(
    VdpOutputSurface                         destination_surface,
    VdpRect const *                          destination_rect,
    VdpBitmapSurface                         source_surface,
    VdpRect const *                          source_rect,
    VdpColor const *                         colors,
    VdpOutputSurfaceRenderBlendState const * blend_state,
    uint32_t                                 flags
)
{
    VdpStatus ret;

    fputs("vdp_output_surface_render_bitmap_surface(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            destination_surface
        );
        _vdp_cap_dump_rect(destination_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            source_surface
        );
        _vdp_cap_dump_rect(source_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_color_list((flags & VDP_OUTPUT_SURFACE_RENDER_COLOR_PER_VERTEX) ? 4 : 1, colors);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_blend_state(blend_state);
        fprintf(
            _vdp_cap_data.fp,
            ", %u",
            flags
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_output_surface_render_bitmap_surface(
        destination_surface,
        destination_rect,
        source_surface,
        source_rect,
        colors,
        blend_state,
        flags
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_decoder_query_capabilities(
    VdpDevice         device,
    VdpDecoderProfile profile,
    /* output parameters follow */
    VdpBool *         is_supported,
    uint32_t *        max_level,
    uint32_t *        max_macroblocks,
    uint32_t *        max_width,
    uint32_t *        max_height
)
{
    VdpStatus ret;

    fputs("vdp_decoder_query_capabilities(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s, %s, %s, %s",
            device,
            profile,
            is_supported ? "-" : "NULL",
            max_level ? "-" : "NULL",
            max_macroblocks ? "-" : "NULL",
            max_width ? "-" : "NULL",
            max_height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_decoder_query_capabilities(
        device,
        profile,
        is_supported,
        max_level,
        max_macroblocks,
        max_width,
        max_height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_level) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_level);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_macroblocks) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_macroblocks);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_width) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (max_height) {
                fprintf(_vdp_cap_data.fp, ", %u", *max_height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_decoder_create(
    VdpDevice         device,
    VdpDecoderProfile profile,
    uint32_t          width,
    uint32_t          height,
    uint32_t          max_references,
    /* output parameters follow */
    VdpDecoder *      decoder
)
{
    VdpStatus ret;

    fputs("vdp_decoder_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %u, %s",
            device,
            profile,
            width,
            height,
            max_references,
            decoder ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_decoder_create(
        device,
        profile,
        width,
        height,
        max_references,
        decoder
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (decoder) {
                fprintf(_vdp_cap_data.fp, ", %u", *decoder);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_decoder_destroy(
    VdpDecoder decoder
)
{
    VdpStatus ret;

    fputs("vdp_decoder_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            decoder
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_decoder_destroy(
        decoder
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_decoder_get_parameters(
    VdpDecoder          decoder,
    /* output parameters follow */
    VdpDecoderProfile * profile,
    uint32_t *          width,
    uint32_t *          height
)
{
    VdpStatus ret;

    fputs("vdp_decoder_get_parameters(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s, %s, %s",
            decoder,
            profile ? "-" : "NULL",
            width ? "-" : "NULL",
            height ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_decoder_get_parameters(
        decoder,
        profile,
        width,
        height
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (profile) {
                fprintf(_vdp_cap_data.fp, ", %u", *profile);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (width) {
                fprintf(_vdp_cap_data.fp, ", %u", *width);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (height) {
                fprintf(_vdp_cap_data.fp, ", %u", *height);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_decoder_render(
    VdpDecoder                 decoder,
    VdpVideoSurface            target,
    VdpPictureInfo const *     picture_info,
    uint32_t                   bitstream_buffer_count,
    VdpBitstreamBuffer const * bitstream_buffers
)
{
    VdpStatus ret;

    fputs("vdp_decoder_render(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        VdpDecoderProfile profile;
        uint32_t          width;
        uint32_t          height;

        ret = _vdp_cap_data.vdp_decoder_get_parameters(
            decoder,
            &profile,
            &width,
            &height
        );

        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            decoder,
            target
        );
        _vdp_cap_dump_picture_info(profile, picture_info);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            bitstream_buffer_count
        );
        _vdp_cap_dump_bitstream_buffer_list(bitstream_buffer_count, bitstream_buffers);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_decoder_render(
        decoder,
        target,
        picture_info,
        bitstream_buffer_count,
        bitstream_buffers
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}


static VdpStatus _vdp_cap_video_mixer_query_feature_support(
    VdpDevice            device,
    VdpVideoMixerFeature feature,
    /* output parameters follow */
    VdpBool *            is_supported
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_query_feature_support(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            feature,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_query_feature_support(
        device,
        feature,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_query_parameter_support(
    VdpDevice              device,
    VdpVideoMixerParameter parameter,
    /* output parameters follow */
    VdpBool *              is_supported
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_query_parameter_support(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            parameter,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_query_parameter_support(
        device,
        parameter,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_query_attribute_support(
    VdpDevice              device,
    VdpVideoMixerAttribute attribute,
    /* output parameters follow */
    VdpBool *              is_supported
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_query_attribute_support(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            attribute,
            is_supported ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_query_attribute_support(
        device,
        attribute,
        is_supported
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (is_supported) {
                fprintf(_vdp_cap_data.fp, ", %d", *is_supported);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_query_parameter_value_range(
    VdpDevice              device,
    VdpVideoMixerParameter parameter,
    /* output parameters follow */
    void *                 min_value,
    void *                 max_value
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_query_parameter_value_range(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s",
            device,
            parameter,
            min_value ? "-" : "NULL",
            max_value ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_query_parameter_value_range(
        device,
        parameter,
        min_value,
        max_value
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_parameter_value(parameter, min_value);
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_parameter_value(parameter, max_value);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_query_attribute_value_range(
    VdpDevice              device,
    VdpVideoMixerAttribute attribute,
    /* output parameters follow */
    void *                 min_value,
    void *                 max_value
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_query_attribute_value_range(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s",
            device,
            attribute,
            min_value ? "-" : "NULL",
            max_value ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_query_attribute_value_range(
        device,
        attribute,
        min_value,
        max_value
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_attribute_value(attribute, min_value, false);
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_attribute_value(attribute, max_value, false);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_create(
    VdpDevice                      device,
    uint32_t                       feature_count,
    VdpVideoMixerFeature const *   features,
    uint32_t                       parameter_count,
    VdpVideoMixerParameter const * parameters,
    void const * const *           parameter_values,
    /* output parameters follow */
    VdpVideoMixer *                mixer
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", device, feature_count);
        _vdp_cap_dump_video_mixer_feature_list(feature_count, features);
        fprintf(_vdp_cap_data.fp, ", %u, ", parameter_count);
        _vdp_cap_dump_video_mixer_parameter_list(parameter_count, parameters);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_video_mixer_parameter_value_list(
            parameter_count,
            parameters,
            parameter_values
        );
        fprintf(_vdp_cap_data.fp, ", %s", mixer ? "-" : "NULL");
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_create(
        device,
        feature_count,
        features,
        parameter_count,
        parameters,
        parameter_values,
        mixer
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (mixer) {
                fprintf(_vdp_cap_data.fp, ", %u", *mixer);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_set_feature_enables(
    VdpVideoMixer                mixer,
    uint32_t                     feature_count,
    VdpVideoMixerFeature const * features,
    VdpBool const *              feature_enables
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_set_feature_enables(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, feature_count);
        _vdp_cap_dump_video_mixer_feature_list(feature_count, features);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_bool_list(feature_count, feature_enables);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_set_feature_enables(
        mixer,
        feature_count,
        features,
        feature_enables
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_set_attribute_values(
    VdpVideoMixer                  mixer,
    uint32_t                       attribute_count,
    VdpVideoMixerAttribute const * attributes,
    void const * const *           attribute_values
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_set_attribute_values(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, attribute_count);
        _vdp_cap_dump_video_mixer_attribute_list(attribute_count, attributes);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_video_mixer_attribute_value_list(
            attribute_count,
            attributes,
            attribute_values,
            false
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_set_attribute_values(
        mixer,
        attribute_count,
        attributes,
        attribute_values
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_get_feature_support(
    VdpVideoMixer                mixer,
    uint32_t                     feature_count,
    VdpVideoMixerFeature const * features,
    /* output parameters follow */
    VdpBool *                    feature_supports
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_get_feature_support(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, feature_count);
        _vdp_cap_dump_video_mixer_feature_list(feature_count, features);
        fputs(feature_supports ? "-" : "NULL", _vdp_cap_data.fp);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_get_feature_support(
        mixer,
        feature_count,
        features,
        feature_supports
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_bool_list(feature_count, feature_supports);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_get_feature_enables(
    VdpVideoMixer                mixer,
    uint32_t                     feature_count,
    VdpVideoMixerFeature const * features,
    /* output parameters follow */
    VdpBool *                    feature_enables
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_get_feature_enables(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, feature_count);
        _vdp_cap_dump_video_mixer_feature_list(feature_count, features);
        fprintf(_vdp_cap_data.fp, ", %s", feature_enables ? "-" : "NULL");
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_get_feature_enables(
        mixer,
        feature_count,
        features,
        feature_enables
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_bool_list(feature_count, feature_enables);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_get_parameter_values(
    VdpVideoMixer                  mixer,
    uint32_t                       parameter_count,
    VdpVideoMixerParameter const * parameters,
    /* output parameters follow */
    void * const *                 parameter_values
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_get_parameter_values(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, parameter_count);
        _vdp_cap_dump_video_mixer_parameter_list(parameter_count, parameters);
        fprintf(_vdp_cap_data.fp, ", %s", parameter_values ? "-" : "NULL");
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_get_parameter_values(
        mixer,
        parameter_count,
        parameters,
        parameter_values
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_parameter_value_list(
                parameter_count,
                parameters,
                parameter_values
            );
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_get_attribute_values(
    VdpVideoMixer                  mixer,
    uint32_t                       attribute_count,
    VdpVideoMixerAttribute const * attributes,
    /* output parameters follow */
    void * const *                 attribute_values
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_get_attribute_values(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "%u, %u, ", mixer, attribute_count);
        _vdp_cap_dump_video_mixer_attribute_list(attribute_count, attributes);
        fprintf(_vdp_cap_data.fp, ", %s", attribute_values ? "-" : "NULL");
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_get_attribute_values(
        mixer,
        attribute_count,
        attributes,
        attribute_values
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_video_mixer_attribute_value_list(
                attribute_count,
                attributes,
                attribute_values,
                true
            );
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_destroy(
    VdpVideoMixer mixer
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            mixer
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_destroy(
        mixer
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_video_mixer_render(
    VdpVideoMixer                 mixer,
    VdpOutputSurface              background_surface,
    VdpRect const *               background_source_rect,
    VdpVideoMixerPictureStructure current_picture_structure,
    uint32_t                      video_surface_past_count,
    VdpVideoSurface const *       video_surface_past,
    VdpVideoSurface               video_surface_current,
    uint32_t                      video_surface_future_count,
    VdpVideoSurface const *       video_surface_future,
    VdpRect const *               video_source_rect,
    VdpOutputSurface              destination_surface,
    VdpRect const *               destination_rect,
    VdpRect const *               destination_video_rect,
    uint32_t                      layer_count,
    VdpLayer const *              layers
)
{
    VdpStatus ret;

    fputs("vdp_video_mixer_render(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, ",
            mixer,
            background_surface
        );
        _vdp_cap_dump_rect(background_source_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %d, %u, ",
            current_picture_structure,
            video_surface_past_count
        );
        _vdp_cap_dump_uint32_t_list(video_surface_past_count, video_surface_past, false);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, %u, ",
            video_surface_current,
            video_surface_future_count
        );
        _vdp_cap_dump_uint32_t_list(video_surface_future_count, video_surface_future, false);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(video_source_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            destination_surface
        );
        _vdp_cap_dump_rect(destination_rect);
        fputs(", ", _vdp_cap_data.fp);
        _vdp_cap_dump_rect(destination_video_rect);
        fprintf(
            _vdp_cap_data.fp,
            ", %u, ",
            layer_count
        );
        _vdp_cap_dump_layers_list(layer_count, layers);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_video_mixer_render(
        mixer,
        background_surface,
        background_source_rect,
        current_picture_structure,
        video_surface_past_count,
        video_surface_past,
        video_surface_current,
        video_surface_future_count,
        video_surface_future,
        video_source_rect,
        destination_surface,
        destination_rect,
        destination_video_rect,
        layer_count,
        layers
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_target_destroy(
    VdpPresentationQueueTarget presentation_queue_target
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_target_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            presentation_queue_target
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_target_destroy(
        presentation_queue_target
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_create(
    VdpDevice                  device,
    VdpPresentationQueueTarget presentation_queue_target,
    /* output parameters follow */
    VdpPresentationQueue *     presentation_queue
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_create(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            presentation_queue_target,
            presentation_queue ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_create(
        device,
        presentation_queue_target,
        presentation_queue
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (presentation_queue) {
                fprintf(_vdp_cap_data.fp, ", %u", *presentation_queue);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_destroy(
    VdpPresentationQueue presentation_queue
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_destroy(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u",
            presentation_queue
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_destroy(
        presentation_queue
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_set_background_color(
    VdpPresentationQueue presentation_queue,
    VdpColor * const     background_color
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_set_background_color(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, ",
            presentation_queue
        );
        _vdp_cap_dump_color(background_color);
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_set_background_color(
        presentation_queue,
        background_color
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_get_background_color(
    VdpPresentationQueue presentation_queue,
    VdpColor *           background_color
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_get_background_color(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s",
            presentation_queue,
            background_color ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_get_background_color(
        presentation_queue,
        background_color
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fputs(", ", _vdp_cap_data.fp);
            _vdp_cap_dump_color(background_color);
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_get_time(
    VdpPresentationQueue presentation_queue,
    /* output parameters follow */
    VdpTime *            current_time
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_get_time(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %s",
            presentation_queue,
            current_time ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_get_time(
        presentation_queue,
        current_time
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (current_time) {
                fprintf(_vdp_cap_data.fp, ", %" PRIu64, *current_time);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_display(
    VdpPresentationQueue presentation_queue,
    VdpOutputSurface     surface,
    uint32_t             clip_width,
    uint32_t             clip_height,
    VdpTime              earliest_presentation_time
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_display(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %u, %u, %" PRIu64,
            presentation_queue,
            surface,
            clip_width,
            clip_height,
            earliest_presentation_time
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_display(
        presentation_queue,
        surface,
        clip_width,
        clip_height,
        earliest_presentation_time
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_block_until_surface_idle(
    VdpPresentationQueue presentation_queue,
    VdpOutputSurface     surface,
    /* output parameters follow */
    VdpTime *            first_presentation_time
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_block_until_surface_idle(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            presentation_queue,
            surface,
            first_presentation_time ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_block_until_surface_idle(
        presentation_queue,
        surface,
        first_presentation_time
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (first_presentation_time) {
                fprintf(_vdp_cap_data.fp, ", %" PRIu64, *first_presentation_time);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_query_surface_status(
    VdpPresentationQueue         presentation_queue,
    VdpOutputSurface             surface,
    /* output parameters follow */
    VdpPresentationQueueStatus * status,
    VdpTime *                    first_presentation_time
)
{
    VdpStatus ret;

    fputs("vdp_presentation_queue_query_surface_status(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s, %s",
            presentation_queue,
            surface,
            status ? "-" : "NULL",
            first_presentation_time ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_query_surface_status(
        presentation_queue,
        surface,
        status,
        first_presentation_time
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (status) {
                fprintf(_vdp_cap_data.fp, ", %d", *status);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
            if (first_presentation_time) {
                fprintf(_vdp_cap_data.fp, ", %" PRIu64, *first_presentation_time);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_preemption_callback_register(
    VdpDevice             device,
    VdpPreemptionCallback callback,
    void *                context
)
{
    VdpStatus ret;

    fputs("vdp_preemption_callback_register(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %p, %p",
            device,
            callback,
            context
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_preemption_callback_register(
        device,
        callback,
        context
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d\n", ret);
    }

    return ret;
}

static VdpStatus _vdp_cap_presentation_queue_target_create_x11(
    VdpDevice                    device,
    Drawable                     drawable,
    /* output parameters follow */
    VdpPresentationQueueTarget * target
)
{
    VdpStatus ret;

    fprintf(_vdp_cap_data.fp, "vdp_presentation_queue_target_create_x11(");
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %lu, %s",
            device,
            drawable,
            target ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    ret = _vdp_cap_data.vdp_presentation_queue_target_create_x11(
        device,
        drawable,
        target
    );

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            if (target) {
                fprintf(_vdp_cap_data.fp, ", %u", *target);
            }
            else {
                fputs(", ???", _vdp_cap_data.fp);
            }
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

static VdpStatus _vdp_cap_get_proc_address(
    VdpDevice device,
    VdpFuncId function_id,
    /* output parameters follow */
    void * *  function_pointer
)
{
    VdpStatus ret;

    fputs("vdp_get_proc_address(", _vdp_cap_data.fp);
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%u, %u, %s",
            device,
            function_id,
            function_pointer ? "-" : "NULL"
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    if (device != _vdp_cap_data.vdp_device) {
        _VDP_TRACE_ERROR_BREAKPOINT();
        ret =  VDP_STATUS_ERROR;
    }
    else if (!function_pointer) {
        _VDP_TRACE_ERROR_BREAKPOINT();
        ret =  VDP_STATUS_ERROR;
    }
    else {
        ret = VDP_STATUS_OK;
        *function_pointer = 0;

        switch (function_id) {
        case VDP_FUNC_ID_GET_ERROR_STRING:
            if (_vdp_cap_data.vdp_get_error_string) {
                *function_pointer = (void *)&_vdp_cap_get_error_string;
            }
            break;
        case VDP_FUNC_ID_GET_PROC_ADDRESS:
            if (_vdp_cap_data.vdp_get_proc_address) {
                *function_pointer = (void *)&_vdp_cap_get_proc_address;
            }
            break;
        case VDP_FUNC_ID_GET_API_VERSION:
            if (_vdp_cap_data.vdp_get_api_version) {
                *function_pointer = (void *)&_vdp_cap_get_api_version;
            }
            break;
        case VDP_FUNC_ID_GET_INFORMATION_STRING:
            if (_vdp_cap_data.vdp_get_information_string) {
                *function_pointer = (void *)&_vdp_cap_get_information_string;
            }
            break;
        case VDP_FUNC_ID_DEVICE_DESTROY:
            if (_vdp_cap_data.vdp_device_destroy) {
                *function_pointer = (void *)&_vdp_cap_device_destroy;
            }
            break;
        case VDP_FUNC_ID_GENERATE_CSC_MATRIX:
            if (_vdp_cap_data.vdp_generate_csc_matrix) {
                *function_pointer = (void *)&_vdp_cap_generate_csc_matrix;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES:
            if (_vdp_cap_data.vdp_video_surface_query_capabilities) {
                *function_pointer = (void *)&_vdp_cap_video_surface_query_capabilities;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES:
            if (_vdp_cap_data.vdp_video_surface_query_get_put_bits_y_cb_cr_capabilities) {
                *function_pointer = (void *)&_vdp_cap_video_surface_query_get_put_bits_y_cb_cr_capabilities;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_CREATE:
            if (_vdp_cap_data.vdp_video_surface_create) {
                *function_pointer = (void *)&_vdp_cap_video_surface_create;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_DESTROY:
            if (_vdp_cap_data.vdp_video_surface_destroy) {
                *function_pointer = (void *)&_vdp_cap_video_surface_destroy;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS:
            if (_vdp_cap_data.vdp_video_surface_get_parameters) {
                *function_pointer = (void *)&_vdp_cap_video_surface_get_parameters;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR:
            if (_vdp_cap_data.vdp_video_surface_get_bits_y_cb_cr) {
                *function_pointer = (void *)&_vdp_cap_video_surface_get_bits_y_cb_cr;
            }
            break;
        case VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR:
            if (_vdp_cap_data.vdp_video_surface_put_bits_y_cb_cr) {
                *function_pointer = (void *)&_vdp_cap_video_surface_put_bits_y_cb_cr;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES:
            if (_vdp_cap_data.vdp_output_surface_query_capabilities) {
                *function_pointer = (void *)&_vdp_cap_output_surface_query_capabilities;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES:
            if (_vdp_cap_data.vdp_output_surface_query_get_put_bits_native_capabilities) {
                *function_pointer = (void *)&_vdp_cap_output_surface_query_get_put_bits_native_capabilities;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES:
            if (_vdp_cap_data.vdp_output_surface_query_put_bits_indexed_capabilities) {
                *function_pointer = (void *)&_vdp_cap_output_surface_query_put_bits_indexed_capabilities;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES:
            if (_vdp_cap_data.vdp_output_surface_query_put_bits_y_cb_cr_capabilities) {
                *function_pointer = (void *)&_vdp_cap_output_surface_query_put_bits_y_cb_cr_capabilities;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_CREATE:
            if (_vdp_cap_data.vdp_output_surface_create) {
                *function_pointer = (void *)&_vdp_cap_output_surface_create;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY:
            if (_vdp_cap_data.vdp_output_surface_destroy) {
                *function_pointer = (void *)&_vdp_cap_output_surface_destroy;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS:
            if (_vdp_cap_data.vdp_output_surface_get_parameters) {
                *function_pointer = (void *)&_vdp_cap_output_surface_get_parameters;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE:
            if (_vdp_cap_data.vdp_output_surface_get_bits_native) {
                *function_pointer = (void *)&_vdp_cap_output_surface_get_bits_native;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE:
            if (_vdp_cap_data.vdp_output_surface_put_bits_native) {
                *function_pointer = (void *)&_vdp_cap_output_surface_put_bits_native;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED:
            if (_vdp_cap_data.vdp_output_surface_put_bits_indexed) {
                *function_pointer = (void *)&_vdp_cap_output_surface_put_bits_indexed;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR:
            if (_vdp_cap_data.vdp_output_surface_put_bits_y_cb_cr) {
                *function_pointer = (void *)&_vdp_cap_output_surface_put_bits_y_cb_cr;
            }
            break;
        case VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES:
            if (_vdp_cap_data.vdp_bitmap_surface_query_capabilities) {
                *function_pointer = (void *)&_vdp_cap_bitmap_surface_query_capabilities;
            }
            break;
        case VDP_FUNC_ID_BITMAP_SURFACE_CREATE:
            if (_vdp_cap_data.vdp_bitmap_surface_create) {
                *function_pointer = (void *)&_vdp_cap_bitmap_surface_create;
            }
            break;
        case VDP_FUNC_ID_BITMAP_SURFACE_DESTROY:
            if (_vdp_cap_data.vdp_bitmap_surface_destroy) {
                *function_pointer = (void *)&_vdp_cap_bitmap_surface_destroy;
            }
            break;
        case VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS:
            if (_vdp_cap_data.vdp_bitmap_surface_get_parameters) {
                *function_pointer = (void *)&_vdp_cap_bitmap_surface_get_parameters;
            }
            break;
        case VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE:
            if (_vdp_cap_data.vdp_bitmap_surface_put_bits_native) {
                *function_pointer = (void *)&_vdp_cap_bitmap_surface_put_bits_native;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE:
            if (_vdp_cap_data.vdp_output_surface_render_output_surface) {
                *function_pointer = (void *)&_vdp_cap_output_surface_render_output_surface;
            }
            break;
        case VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE:
            if (_vdp_cap_data.vdp_output_surface_render_bitmap_surface) {
                *function_pointer = (void *)&_vdp_cap_output_surface_render_bitmap_surface;
            }
            break;
        case VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES:
            if (_vdp_cap_data.vdp_decoder_query_capabilities) {
                *function_pointer = (void *)&_vdp_cap_decoder_query_capabilities;
            }
            break;
        case VDP_FUNC_ID_DECODER_CREATE:
            if (_vdp_cap_data.vdp_decoder_create) {
                *function_pointer = (void *)&_vdp_cap_decoder_create;
            }
            break;
        case VDP_FUNC_ID_DECODER_DESTROY:
            if (_vdp_cap_data.vdp_decoder_destroy) {
                *function_pointer = (void *)&_vdp_cap_decoder_destroy;
            }
            break;
        case VDP_FUNC_ID_DECODER_GET_PARAMETERS:
            if (_vdp_cap_data.vdp_decoder_get_parameters) {
                *function_pointer = (void *)&_vdp_cap_decoder_get_parameters;
            }
            break;
        case VDP_FUNC_ID_DECODER_RENDER:
            if (_vdp_cap_data.vdp_decoder_render) {
                *function_pointer = (void *)&_vdp_cap_decoder_render;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT:
            if (_vdp_cap_data.vdp_video_mixer_query_feature_support) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_query_feature_support;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT:
            if (_vdp_cap_data.vdp_video_mixer_query_parameter_support) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_query_parameter_support;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT:
            if (_vdp_cap_data.vdp_video_mixer_query_attribute_support) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_query_attribute_support;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE:
            if (_vdp_cap_data.vdp_video_mixer_query_parameter_value_range) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_query_parameter_value_range;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE:
            if (_vdp_cap_data.vdp_video_mixer_query_attribute_value_range) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_query_attribute_value_range;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_CREATE:
            if (_vdp_cap_data.vdp_video_mixer_create) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_create;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES:
            if (_vdp_cap_data.vdp_video_mixer_set_feature_enables) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_set_feature_enables;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES:
            if (_vdp_cap_data.vdp_video_mixer_set_attribute_values) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_set_attribute_values;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT:
            if (_vdp_cap_data.vdp_video_mixer_get_feature_support) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_get_feature_support;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES:
            if (_vdp_cap_data.vdp_video_mixer_get_feature_enables) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_get_feature_enables;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES:
            if (_vdp_cap_data.vdp_video_mixer_get_parameter_values) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_get_parameter_values;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES:
            if (_vdp_cap_data.vdp_video_mixer_get_attribute_values) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_get_attribute_values;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_DESTROY:
            if (_vdp_cap_data.vdp_video_mixer_destroy) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_destroy;
            }
            break;
        case VDP_FUNC_ID_VIDEO_MIXER_RENDER:
            if (_vdp_cap_data.vdp_video_mixer_render) {
                *function_pointer = (void *)&_vdp_cap_video_mixer_render;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY:
            if (_vdp_cap_data.vdp_presentation_queue_target_destroy) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_target_destroy;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE:
            if (_vdp_cap_data.vdp_presentation_queue_create) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_create;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY:
            if (_vdp_cap_data.vdp_presentation_queue_destroy) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_destroy;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR:
            if (_vdp_cap_data.vdp_presentation_queue_set_background_color) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_set_background_color;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR:
            if (_vdp_cap_data.vdp_presentation_queue_get_background_color) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_get_background_color;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME:
            if (_vdp_cap_data.vdp_presentation_queue_get_time) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_get_time;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY:
            if (_vdp_cap_data.vdp_presentation_queue_display) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_display;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE:
            if (_vdp_cap_data.vdp_presentation_queue_block_until_surface_idle) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_block_until_surface_idle;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS:
            if (_vdp_cap_data.vdp_presentation_queue_query_surface_status) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_query_surface_status;
            }
            break;
        case VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER:
            if (_vdp_cap_data.vdp_preemption_callback_register) {
                *function_pointer = (void *)&_vdp_cap_preemption_callback_register;
            }
            break;
        case VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11:
            if (_vdp_cap_data.vdp_presentation_queue_target_create_x11) {
                *function_pointer = (void *)&_vdp_cap_presentation_queue_target_create_x11;
            }
            break;
        default:
            fprintf(
                _vdp_cap_data.fp,
                "VDPAU capture: Not able to proxy function %d",
                function_id
            );
            ret = _vdp_cap_data.vdp_get_proc_address(device, function_id, function_pointer);
            break;
        }

        if ((ret == VDP_STATUS_OK) && !*function_pointer) {
            ret = VDP_STATUS_INVALID_FUNC_ID;
        }
    }

    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", ret);
        if (ret == VDP_STATUS_OK) {
            fprintf(
                _vdp_cap_data.fp,
                ", %p",
                *function_pointer
            );
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return ret;
}

extern "C" void vdp_trace_set_backend_handle(
    void * driver_dll_handle
)
{
    _vdp_cap_data.dll = driver_dll_handle;
}

extern "C" VdpDeviceCreateX11 vdp_trace_device_create_x11;

VdpStatus vdp_trace_device_create_x11(
    Display *             display,
    int                   screen,
    /* output parameters follow */
    VdpDevice *           device,
    VdpGetProcAddress * * get_proc_address
)
{
    if (!device || !get_proc_address) {
        _VDP_TRACE_ERROR_BREAKPOINT();
        return VDP_STATUS_INVALID_POINTER;
    }

    // For now, the capture library only allows a single VdpDevice
    // This could probably be fixed by dynamically associating most of
    // _vdp_cap_data with a VdpDevice handle.
    if (_vdp_cap_data.fp) {
        fprintf(_vdp_cap_data.fp,
            "VDPAU trace: Multiple devices created; "
            "will return get_proc_address results from the latest only\n"
        );
    }
    else {
        _vdp_cap_data.level = 0;
        char const * vdpau_trace = secure_getenv("VDPAU_TRACE");
        if (vdpau_trace) {
            _vdp_cap_data.level = atoi(vdpau_trace);
        }

        _vdp_cap_data.fp = 0;
        char const * vdpau_trace_file = secure_getenv("VDPAU_TRACE_FILE");
        if (vdpau_trace_file && strlen(vdpau_trace_file)) {
            if (vdpau_trace_file[0] == '&') {
                int fd = atoi(&vdpau_trace_file[1]);
                _vdp_cap_data.fp = fdopen(fd, "wt");
            }
            else {
                _vdp_cap_data.fp = fopen(vdpau_trace_file, "wt");
            }
            if (!_vdp_cap_data.fp) {
                fprintf(
                    stderr,
                    "VDPAU capture: ERROR: Can't open '%s' for writing, defaulting to stderr\n",
                    vdpau_trace_file
                );
            }
        }
        if (!_vdp_cap_data.fp) {
            _vdp_cap_data.fp = stderr;
        }
        fprintf(_vdp_cap_data.fp, "VDPAU capture: Enabled\n");
    }

    fprintf(_vdp_cap_data.fp, "vdp_imp_device_create_x11(");
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(
            _vdp_cap_data.fp,
            "%p, %d, -, -",
            display,
            screen
        );
    }
    fputs(")\n", _vdp_cap_data.fp);

    VdpStatus vdp_st = VDP_STATUS_ERROR;

    VdpDeviceCreateX11 * vdp_imp_device_create_x11;
    vdp_imp_device_create_x11 = (VdpDeviceCreateX11*)dlsym(
        _vdp_cap_data.dll,
         "vdp_imp_device_create_x11"
    );
    if (!vdp_imp_device_create_x11) {
        _VDP_TRACE_ERROR_BREAKPOINT();
        vdp_st = VDP_STATUS_NO_IMPLEMENTATION;
        goto done;
    }

    vdp_st = vdp_imp_device_create_x11(
        display,
        screen,
        &_vdp_cap_data.vdp_device,
        &_vdp_cap_data.vdp_get_proc_address
    );
    if (vdp_st != VDP_STATUS_OK) {
        _VDP_TRACE_ERROR_BREAKPOINT();
        goto done;
    }

    *device = _vdp_cap_data.vdp_device;
    *get_proc_address = _vdp_cap_get_proc_address;

#define GET_POINTER(_id_, _var_) \
    vdp_st = _vdp_cap_data.vdp_get_proc_address( \
        _vdp_cap_data.vdp_device, \
        (_id_), \
        (void * *)&_vdp_cap_data._var_ \
    ); \
    if (vdp_st != VDP_STATUS_OK) { \
        _vdp_cap_data._var_ = 0; \
    }

    GET_POINTER(VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER,                          vdp_preemption_callback_register);
    GET_POINTER(VDP_FUNC_ID_GET_ERROR_STRING,                                      vdp_get_error_string);
    GET_POINTER(VDP_FUNC_ID_GET_API_VERSION,                                       vdp_get_api_version);
    GET_POINTER(VDP_FUNC_ID_GET_INFORMATION_STRING,                                vdp_get_information_string);
    GET_POINTER(VDP_FUNC_ID_DEVICE_DESTROY,                                        vdp_device_destroy);
    GET_POINTER(VDP_FUNC_ID_GENERATE_CSC_MATRIX,                                   vdp_generate_csc_matrix);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES,                      vdp_video_surface_query_capabilities);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES, vdp_video_surface_query_get_put_bits_y_cb_cr_capabilities);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_CREATE,                                  vdp_video_surface_create);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_DESTROY,                                 vdp_video_surface_destroy);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS,                          vdp_video_surface_get_parameters);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR,                        vdp_video_surface_get_bits_y_cb_cr);
    GET_POINTER(VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR,                        vdp_video_surface_put_bits_y_cb_cr);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES,                     vdp_output_surface_query_capabilities);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES, vdp_output_surface_query_get_put_bits_native_capabilities);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES,    vdp_output_surface_query_put_bits_indexed_capabilities);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES,    vdp_output_surface_query_put_bits_y_cb_cr_capabilities);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_CREATE,                                 vdp_output_surface_create);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY,                                vdp_output_surface_destroy);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS,                         vdp_output_surface_get_parameters);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE,                        vdp_output_surface_get_bits_native);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE,                        vdp_output_surface_put_bits_native);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED,                       vdp_output_surface_put_bits_indexed);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR,                       vdp_output_surface_put_bits_y_cb_cr);
    GET_POINTER(VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES,                     vdp_bitmap_surface_query_capabilities);
    GET_POINTER(VDP_FUNC_ID_BITMAP_SURFACE_CREATE,                                 vdp_bitmap_surface_create);
    GET_POINTER(VDP_FUNC_ID_BITMAP_SURFACE_DESTROY,                                vdp_bitmap_surface_destroy);
    GET_POINTER(VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS,                         vdp_bitmap_surface_get_parameters);
    GET_POINTER(VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE,                        vdp_bitmap_surface_put_bits_native);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE,                  vdp_output_surface_render_output_surface);
    GET_POINTER(VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE,                  vdp_output_surface_render_bitmap_surface);
    GET_POINTER(VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES,                            vdp_decoder_query_capabilities);
    GET_POINTER(VDP_FUNC_ID_DECODER_CREATE,                                        vdp_decoder_create);
    GET_POINTER(VDP_FUNC_ID_DECODER_DESTROY,                                       vdp_decoder_destroy);
    GET_POINTER(VDP_FUNC_ID_DECODER_GET_PARAMETERS,                                vdp_decoder_get_parameters);
    GET_POINTER(VDP_FUNC_ID_DECODER_RENDER,                                        vdp_decoder_render);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT,                     vdp_video_mixer_query_feature_support);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT,                   vdp_video_mixer_query_parameter_support);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT,                   vdp_video_mixer_query_attribute_support);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE,               vdp_video_mixer_query_parameter_value_range);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE,               vdp_video_mixer_query_attribute_value_range);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_CREATE,                                    vdp_video_mixer_create);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES,                       vdp_video_mixer_set_feature_enables);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES,                      vdp_video_mixer_set_attribute_values);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT,                       vdp_video_mixer_get_feature_support);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES,                       vdp_video_mixer_get_feature_enables);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES,                      vdp_video_mixer_get_parameter_values);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES,                      vdp_video_mixer_get_attribute_values);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_DESTROY,                                   vdp_video_mixer_destroy);
    GET_POINTER(VDP_FUNC_ID_VIDEO_MIXER_RENDER,                                    vdp_video_mixer_render);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY,                     vdp_presentation_queue_target_destroy);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE,                             vdp_presentation_queue_create);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY,                            vdp_presentation_queue_destroy);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR,               vdp_presentation_queue_set_background_color);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR,               vdp_presentation_queue_get_background_color);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME,                           vdp_presentation_queue_get_time);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY,                            vdp_presentation_queue_display);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE,           vdp_presentation_queue_block_until_surface_idle);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS,               vdp_presentation_queue_query_surface_status);
    GET_POINTER(VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER,                          vdp_preemption_callback_register);
    GET_POINTER(VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11,                  vdp_presentation_queue_target_create_x11);

    vdp_st = VDP_STATUS_OK;

done:
    if (_vdp_cap_data.level >= LEVEL_PARAMS) {
        fprintf(_vdp_cap_data.fp, "    -> %d", vdp_st);
        if (vdp_st == VDP_STATUS_OK) {
            fprintf(
                _vdp_cap_data.fp,
                ", %x, %p",
                _vdp_cap_data.vdp_device,
                _vdp_cap_data.vdp_get_proc_address
            );
        }
        fputs("\n", _vdp_cap_data.fp);
    }

    return vdp_st;
}

