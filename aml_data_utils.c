/**
 ** aml_data_utils.c
 **
 ** This program is the factory of PCM data. Such as,
 **        re-mix
 **        extend
 **        extract
 **        exchange
 **        invert
 **        concat
 **        empty
 **        ditter
 **        replace
 ** author: shen pengru
 */
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <cutils/log.h>
#include <fcntl.h>
#include "aml_data_utils.h"

#define LOG_TAG "audio_data_utils"

static struct aml_audio_channel_name gAudioChName[AML_I2S_CHANNEL_COUNT] {
	{AML_CH_IDX_L,   "Left"},
	{AML_CH_IDX_R,   "Right"},
	{AML_CH_IDX_C,   "Center"},
	{AML_CH_IDX_LFE, "Lfe"},
	{AML_CH_IDX_LS,  "Left Surround"},
	{AML_CH_IDX_RS,  "Right Surround"},
	{AML_CH_IDX_LBS, "Left Back Surround"},
	{AML_CH_IDX_RBS, "Right Back Surround"},
	{AML_CH_IDX_LT,  "Left Top"},
	{AML_CH_IDX_RT,  "Right Top"},
};

static inline int16_t _clamp16(int32_t sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

static char *_get_ch_name(eChannelContentIdx idx)
{
	int i = 0;

	for (i=AML_CH_IDX_L; i<AML_I2S_CHANNEL_COUNT; i++) {
		if (idx == gAudioChName[i].ch_idx) {
			return gAudioChName[i].ch_name;
		}
	}

	switch (idx) {
	case AML_CH_IDX_5_1_ALL:
		return "5.1Ch";
	case AML_CH_IDX_7_1_ALL:
		return "7.1Ch";
	case AML_CH_IDX_5_1_2_ALL:
		return "5.1.2Ch";
	default:
		break;
	}

	return "Invalid";
}
#define SHENPENGRU_TEST (1)

#if SHENPENGRU_TEST
struct aml_channel_map test_maps[AML_I2S_CHANNEL_COUNT] {
	{AML_CH_IDX_L,   AML_I2S_PORT_IDX_01,   AML_I2S_CHANNEL_0},
	{AML_CH_IDX_R,   AML_I2S_PORT_IDX_01,   AML_I2S_CHANNEL_1},
	{AML_CH_IDX_C,   AML_I2S_PORT_IDX_23,   AML_I2S_CHANNEL_2},
	{AML_CH_IDX_LFE, AML_I2S_PORT_IDX_23,   AML_I2S_CHANNEL_3},
	{AML_CH_IDX_LS,  AML_I2S_PORT_IDX_67,   AML_I2S_CHANNEL_6},
	{AML_CH_IDX_RS,  AML_I2S_PORT_IDX_67,   AML_I2S_CHANNEL_7},
	{AML_CH_IDX_LT,  AML_I2S_PORT_IDX_45,   AML_I2S_CHANNEL_4},
	{AML_CH_IDX_RT,  AML_I2S_PORT_IDX_45,   AML_I2S_CHANNEL_5},
};
#endif

/* WARNNING: initial function, only need called once!! */
struct aml_channel_map *data_load_product_config(void)
{
	struct aml_channel_map *maps = NULL;
	int i = 0;

#if SHENPENGRU_TEST
	return &test_maps;
#else
	maps = malloc(sizeof(struct aml_channel_map*AML_I2S_CHANNEL_COUNT));
	if (!maps) {
		return NULL;
	}

	//TODO: load from tv.config
	/*
	 * map[0].channel_idx = *;
	 * map[0].i2s_idx     = *;
	 * map[0].bit_mask    = *;
	 **/

	return maps;
#endif
}

int data_get_channel_i2s_port(
	struct aml_channel_map *map, eChannelContentIdx channelName)
{
	int i = 0;

	if (map == NULL) {
		return AML_I2S_PORT_IDX_NULL;
	}

	for (i=AML_CH_IDX_L; i<AML_CH_IDX_MAX; i++) {
		if (map[i]->channel_idx == channelName) {
			ALOGD("%s: name: %s i2s-port: %d\n", __func__,
				_get_ch_name(channelName),
				map[i]->i2s_idx);
			return map[i]->i2s_idx;
		}
	}

	return AML_I2S_PORT_IDX_NULL;
}

int data_get_channel_bit_mask(
	struct aml_channel_map *map, eChannelContentIdx channelName)
{
	int i = 0;
	int bit_mask = AML_I2S_CHANNEL_NULL;

	if (map == NULL) {
		return bit_mask;
	}

	for (i=AML_CH_IDX_L; i<AML_CH_IDX_MAX; i++) {
		if (map[i]->channel_idx == channelName) {
			bit_mask = map[i]->bit_mask;
			ALOGD("%s: name: %s i2s-bit-mask: 0x%08x\n", __func__,
				_get_ch_name(channelName),
				bit_mask);
			return bit_mask;
		}
	}

	switch (channelName) {
	case AML_CH_IDX_5_1_ALL:
	case AML_CH_IDX_7_1_ALL:
	case AML_CH_IDX_5_1_2_ALL:
		for (i=0; i<AML_CH_IDX_MAX; i++) {
			bit_mask |= map[i]->bit_mask;
		}
		ALOGD("%s: name: %s i2s-bit-mask: 0x%08x\n", __func__,
			_get_ch_name(channelName),
			bit_mask);
		return bit_mask; 
	default:
		break;
	}

	return bit_mask;
}

eChannelContentIdx data_get_channel_content_idx(
	struct aml_channel_map *map, int bitmask)
{
	int idx = AML_CH_IDX_NULL;
	int i = 0;
	int bit_mask = AML_I2S_CHANNEL_NULL;

	if (map == NULL) {
		return idx;
	}

	for (i=AML_CH_IDX_L; i<AML_CH_IDX_MAX; i++) {
		if (map[i]->bit_mask == bitmask) {
			//TODO: how about one channel -> two bitmask?
			idx = map[i]->channel_idx;
			break;
		}
	}

	return idx;
}

static int _data_remix_center_to_lr(void *buf, size_t frames, size_t framesz, int channels, int bitmask)
{
	int16_t center16;
	int32_t center32;
	int16_t *buf16  = (int16_t *)buf;
	int32_t *buf32  = (int32_t *)buf;
	int     i, tmp;

	///< TODO: should use bitmask to find the center channel
	///< buf is 6ch pcm(L,R,C,lfe,Lr,Rs)
	if (channels != 6) {
		ALOGD("%s: only support 6 ch now!\n", __func__);
		return -1;
	}

	if (framesz != e16BitPerFrame) {
		ALOGD("%s: only support 16bit now!\n", __func__);
		return -1;
	}

	switch (framesz) {
	case e16BitPerFrame:
		///< 3/0 input L_out/R_out =  = 0.707*(L/R + 0.707*C);
		for (i=0; i <frames; i++) {
			/* save data of center */
			center16 = buf16[channels*i + 2];
			/* calculate L */
			tmp                   = buf16[channels*i + 0] << 12;
			buf16[channels*i + 0] = _clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12*center16) >>12))>>12);
			/* calculate R */
			tmp                   = buf16[channels*i + 1] << 12;
			buf16[channels*i + 1] = _clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12*center16) >>12))>>12);
		}
		break;
	case e32BitPerFrame:
		//TODO:
		break;
	default:
		break;
	}

	return 0;
}

static int _data_remix_all_to_lr(void *buf,	size_t frames, size_t framesz, int channels, int bitmask)
{
    /*
     * --------------------------------------
     * 3/2 input module:
     * L_out = 0.707*(L + 0.707*C + 0.707*Ls)
     * R_out = 0.707*(R + 0.707*C + 0.707*Rs)
     * --------------------------------------
     * our channel sequeces:
     * 0->L
     * 1->R
     * 2->C
     * 3->lfe
     * 4->Ls
     * 5->Rs
     * --------------------------------------
     */
	int16_t l_16, r_16, c_16, ls_16, rs_16;
	int16_t l_32, r_32, c_32, ls_32, rs_32;
	int16_t *buf16  = (int16_t *)buf;
	int32_t *buf32  = (int32_t *)buf;
	int     i, j, tmp;

	///< buf is 6ch pcm(L,R,C,lfe,Lr,Rs)
	if (channels != 6) {
		ALOGD("%s: only support 6 ch now!\n", __func__);
		return -1;
	}

	if (framesz != e16BitPerFrame) {
		ALOGD("%s: only support 16bit now!\n", __func__);
		return -1;
	}

	switch (framesz) {
	case e16BitPerFrame:
		for (i = 0; i < frames; i++) {
			/* save l/r/c/ls/rs */
			l_16  = buf16[channels*i + 0];
			r_16  = buf16[channels*i + 1];
			c_16  = buf16[channels*i + 2];
			ls_16 = buf16[channels*i + 4];
			rs_16 = buf16[channels*i + 5];
			/* handle L channel */
			tmp = l_16+ << 12;
			buf16[channels*i] =
				_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12 * c_16 + MINUS_3_DB_IN_Q19_12 * ls_16) >> 12)) >> 12);
			/* handle R channel */
			tmp = r_16 << 12;
			buf16[channels*i + 1] =
				_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12 * c_16 + MINUS_3_DB_IN_Q19_12 * rs_16) >> 12)) >> 12);
		}
		break;
	case e32BitPerFrame:
		//TODO:
		break;
	default:
		break;
	}

	return 0;
}

int data_remix_to_lr_channel(
	struct  aml_channel_map *map,
	void    *buf,
	size_t  frames,
	size_t  framesz,
	int     channels,
	eChannelContentIdx chIdx)
{
    int bit_mask = data_get_channel_bit_mask(map, chIdx);

	switch (chIdx) {
	case AML_CH_IDX_C:
		return _data_remix_center_to_lr(buf, frames, framesz, channels, bit_mask);
	case AML_CH_IDX_5_1_ALL:
	case AML_CH_IDX_7_1_ALL:
	case AML_CH_IDX_5_1_2_ALL:
		return _data_remix_all_to_lr(buf, frames, framesz, channels, bit_mask);
	default:
		break;
	}

	return 0
}

int data_empty_channels(
	struct  aml_channel_map *map,
	void    *buf,
	size_t  frames,
	size_t  framesz,
	int     channels,
	int     channel_empty_bit_mask)
{
	int i, j;
	int16_t *buf16 = (int16_t *)buf;
	int32_t *buf32 = (int32_t *)buf;

	switch (framesz) {
	case e16BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<channels; j++) {
				if (channel_empty_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
					buf16[channels*i + j] = 0x00;
				}
			}
		}
		break;
	case e32BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<channels; j++) {
				if (channel_empty_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
					buf32[channels*i + j] = 0x00;
				}
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

int data_exchange_i2s_channels(
	void    *buf,
	size_t  channels,
	size_t  frames,
	size_t  framesz,
	int     i2s_idx1,
	int     i2s_idx2)
{
	int i, j;
	int16_t *buf16 = (int16_t *)buf;
	int32_t *buf32 = (int32_t *)buf;
	int16_t tmp16[AML_CH_CNT_PER_PORT];
	int32_t tmp32[AML_CH_CNT_PER_PORT];

	if (channels < 2*AML_CH_CNT_PER_PORT) {
		ALOGE("%s: at least 2 i2s port is needed!\n", __func__);
		return -1;
	}

	switch (framesz) {
	case e16BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<AML_CH_CNT_PER_PORT; j++) {
				tmp16[j]                                = buf16[i2s_idx1*AML_CH_CNT_PER_PORT + j];
				buf16[i2s_idx1*AML_CH_CNT_PER_PORT + j] = buf16[i2s_idx2*AML_CH_CNT_PER_PORT + j];
				buf16[i2s_idx2*AML_CH_CNT_PER_PORT + j] = tmp16[j];
			}
		}
		break;
	case e32BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<AML_CH_CNT_PER_PORT; j++) {
				tmp32[j]                                = buf32[i2s_idx1*AML_CH_CNT_PER_PORT + j];
				buf32[i2s_idx1*AML_CH_CNT_PER_PORT + j] = buf32[i2s_idx2*AML_CH_CNT_PER_PORT + j];
				buf32[i2s_idx2*AML_CH_CNT_PER_PORT + j] = tmp32[j];
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

int data_replace_lfe_data(
	void    *out_buf,
	size_t  out_channles,
	size_t  out_framesz,
	void    *input_lfe_buffer,
	size_t  in_channles,
	size_t  in_framesz,
	size_t  frames,
	int     channel_insert_bit_mask)
{
	int i, j;
	int16_t *buf_out16 = (int16_t *)out_buf;
	int32_t *buf_out32 = (int32_t *)out_buf;
	int16_t *buf_in16  = (int16_t *)input_lfe_buffer;
	int32_t *buf_in32  = (int32_t *)input_lfe_buffer;
	int     lfe_base;
	int     lfe_cnt;

	if (out_channles < 6) {
		ALOGE("%s: only support 5.1 channels\n", __func__);
		return -1;
	}

	if (in_channles != 2) {
		ALOGE("%s: only support replace 2 channels\n", __func__);
		return -1;
	}

	if (in_framesz != out_framesz) {
		ALOGE("%s: only support same bit(all 16bit/all 32bit) now\n", __func__);
		return -1;
	}

	//TODO: should get from channel_insert_bit_mask
	//case_1: ch2,ch3 are all lfe
	if ((AML_I2S_CHANNEL_2|AML_I2S_CHANNEL_3) == channel_insert_bit_mask) {
		lfe_base = 2;
		lfe_cnt  = 2;
	}
	//case_1:     ch3  is lfe
	if (AML_I2S_CHANNEL_3 == channel_insert_bit_mask) {
		lfe_base = 3;
		lfe_cnt  = 1;
	}

	switch (out_framesz) {
	case e16BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			for (i=0; i<frames; i++) {
				for (j=0; j<lfe_cnt; j++) {
					buf_out16[out_channels*i + lfe_base + j] = buf_in16[in_channles*i + j];
				}
			}
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}		
		break;
	case e32BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			//TODO:
			break;
		case e32BitPerFrame:
			for (i=0; i<frames; i++) {
				for (j=0; j<lfe_cnt; j++) {
					buf_out32[out_channels*i + lfe_base + j] = buf_in32[in_channles*i + j];
				}
			}
			break;
		default:
			break;
		}		
		break;
	default:
		break;
	}

	return 0;
}

int data_invert_channels(
	void    *buf,
	int     channels,
	size_t  frames,
	size_t  framesz,
	int     channel_invert_bit_mask)
{
	int i, j;
	int16_t *buf16 = (int16_t *)buf;
	int32_t *buf32 = (int32_t *)buf;
	int16_t tmp16;
	int32_t tmp32;

	//_find_index_need_effect(channel_invert_bit_mask);
	switch (framesz) {
	case e16BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<channels; j++) {
				if (channel_invert_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
					tmp16                 = *buf16[channels*i + j];
					buf16[channels*i + j] = -tmp16;
				}
			}
		}
		break;
	case e32BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<channels; j++) {
				if (channel_invert_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
					tmp32                 = *buf32[channels*i + j];
					buf32[channels*i + j] = -tmp32;
				}
			}
		}
		break;
	default:
		break;
	}

	return 0;	
}

int data_concat_channels(
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	void    *in_buf1, void *in_buf2, void *in_buf3, void *in_buf4,
	size_t  in_channels,
	size_t  in_framesz,
	size_t  frames)
{
	int i, j;
	int16_t *buf_out16  = (int16_t *)out_buf;
	int32_t *buf_out32  = (int32_t *)out_buf;

	int16_t *buf_in1_16 = (int16_t *)in_buf1;
	int16_t *buf_in2_16 = (int16_t *)in_buf2;
	int16_t *buf_in3_16 = (int16_t *)in_buf3;
	int16_t *buf_in4_16 = (int16_t *)in_buf4;

	int32_t *buf_in1_32 = (int32_t *)in_buf1;
	int32_t *buf_in2_32 = (int32_t *)in_buf2;
	int32_t *buf_in3_32 = (int32_t *)in_buf3;
	int32_t *buf_in4_32 = (int32_t *)in_buf4;

	if (in_channels != 2 && in_channels != 4 && in_channels != 6 && in_channels != 8) {
		ALOGE("%s: only support concat 2/4/6/8 channels together!\n", __func__);
		return -EINVAL;
	}

	switch (out_framesz) {
	case e16BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			for (i=0; i<frames; i++) {
				if (in_channels >= 2) {
					buf_out16[out_channels*i + 0] = buf_in1_16[2*i + 0];
					buf_out16[out_channels*i + 1] = buf_in1_16[2*i + 1];
				}
				if (in_channels >= 4) {
					buf_out16[out_channels*i + 2] = buf_in2_16[2*i + 0];
					buf_out16[out_channels*i + 3] = buf_in2_16[2*i + 1];
				}
				if (in_channels >= 6) {
					buf_out16[out_channels*i + 4] = buf_in3_16[2*i + 0];
					buf_out16[out_channels*i + 5] = buf_in3_16[2*i + 1];
				}
				if (in_channels >= 8) {
					buf_out16[out_channels*i + 6] = buf_in4_16[2*i + 0];
					buf_out16[out_channels*i + 7] = buf_in4_16[2*i + 1];
				}
			}
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}
		break;
	case e32BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			//TODO:
			break;
		case e32BitPerFrame:
			for (i=0; i<frames; i++) {
				if (in_channels >= 2) {
					buf_out32[out_channels*i + 0] = buf_in1_32[2*i + 0];
					buf_out32[out_channels*i + 1] = buf_in1_32[2*i + 1];
				}
				if (in_channels >= 4) {
					buf_out32[out_channels*i + 2] = buf_in2_32[2*i + 0];
					buf_out32[out_channels*i + 3] = buf_in2_32[2*i + 1];
				}
				if (in_channels >= 6) {
					buf_out32[out_channels*i + 4] = buf_in3_32[2*i + 0];
					buf_out32[out_channels*i + 5] = buf_in3_32[2*i + 1];
				}
				if (in_channels >= 8) {
					buf_out32[out_channels*i + 6] = buf_in4_32[2*i + 0];
					buf_out32[out_channels*i + 7] = buf_in4_32[2*i + 1];
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	

	return 0;
}

#define COUNT_DITTER_8CH (32)
int16_t ditter_8ch[COUNT_DITTER_8CH] = {
	0xffff, 0x0001, 0xffff, 0x0000, 0xffff, 0x0000, 0x0001, 0xffff,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0001, 0xffff, 0xffff, 0x0001,
	0xffff, 0x0001, 0xffff, 0x0000, 0xffff, 0x0001, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0001, 0xffff, 0xffff
};

int data_add_ditter_to_channels(
	void    *buffer,
	size_t  frames,
	size_t  framesz,
	int     channels,
	int     channel_ditter_bit_mask)
{
	int i, j;
	int16_t *buf16  = (int16_t *)buffer;
	int32_t *buf32  = (int32_t *)buffer;

	int32_t tmpbuf;
	int cnt = 0;

	if (channels != 8) {
		ALOGE("%s: only support 5.1.2 ch(8channels) now\n", __func__);
		return -1;
	}

	if (framesz != e16BitPerFrame) {
		ALOGE("%s: only support 16bit now\n", __func__);
		return -1;
	}

	switch (framesz) {
	case e16BitPerFrame:
		for (i=0; i<frames; i++) {
			for (j=0; j<channels; j++) {
				if (channel_ditter_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
					tmpbuf = (int32_t)buf16[channels*i + j] + (int32_t)ditter_8ch[cnt%COUNT_DITTER_8CH];
				}
				cnt++;
				if (tmpbuf > 32767 || tmpbuf < -32768) {
					continue;
				} else {
					buf16[channels*i + j] = (int16_t)tmpbuf;
				}
			}
		}
		break;
	case e32BitPerFrame:
		//TODO:
		break;
	default:
		break;
	}

	return 0;
}

int data_extend_channels(
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	void    *in_buf,
	size_t  in_channels,
	size_t  in_framesz,
	size_t  frames)
{
	int i, j;
	int16_t *buf_in16   = (int16_t *)in_buf;
	int32_t *buf_in32   = (int32_t *)in_buf;
	int16_t *buf_out16  = (int16_t *)out_buf;
	int32_t *buf_out32  = (int32_t *)out_buf;

	if (out_channels < in_channels) {
		ALOGE("%s: only support extend channels\n", __func__);
		return -1;
	}

	if (out_framesz != e16BitPerFrame || in_framesz != e16BitPerFrame) {
		ALOGE("%s: only support 16bit now\n", __func__);
		return -1;
	}

	switch (out_framesz) {
	case e16BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			for (i=0; i<frames; i++) {
				for (j=0; j<out_channels; j++) {
					if (j <= in_channels) {
						buf_out16[out_channels*i + j] = buf_in16[in_channels*i + j];
					} else {
						buf_out16[out_channels*i + j] = 0x00;
					}
				}
			}
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}
		break;
	case e32BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			//TODO:
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}

int data_extract_channels(
	struct  aml_channel_map *map,
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	void    *in_buf,
	size_t  in_channels,
	size_t  in_framesz,
	size_t  frames,
	int     channel_extract_bit_mask)
{
	int i, j;
	int16_t *buf_in16   = (int16_t *)in_buf;
	int32_t *buf_in32   = (int32_t *)in_buf;
	int16_t *buf_out16  = (int16_t *)out_buf;
	int32_t *buf_out32  = (int32_t *)out_buf;
	int cnt;

	if (out_channels > in_channels) {
		ALOGE("%s: only support extract channels\n", __func__);
		return -1;
	}

	if (out_framesz != e16BitPerFrame || in_framesz != e16BitPerFrame) {
		ALOGE("%s: only support 16bit now\n", __func__);
		return -1;
	}

	switch (out_framesz) {
	case e16BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			for (i=0; i<out_channels; i++) {
				cnt = 0;
				for (j=0; j<in_channels; j++) {
					if (channel_extract_bit_mask & (AML_I2S_CHANNEL_0<<j)) {
						buf_out16[out_channels*i + cnt] = buf_in16[in_channels*i + j];
						cnt++;
					}
				}
			}
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}
		break;
	case e32BitPerFrame:
		switch (in_framesz) {
		case e16BitPerFrame:
			//TODO:
			break;
		case e32BitPerFrame:
			//TODO:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}
