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

#define MINUS_3_DB_IN_Q19_12 2896 // -3dB = 0.707 * 2^12 = 2896
/* PCM5.1 */
/*
 * Don't caulate the size here, just use fix value is defined.
 * Because this funtcion will be called frequently.
 **/
#define COUNT_DITTER_6CH (32*6)
int16_t ditter_6ch[COUNT_DITTER_6CH] = {
	/*   L       R       C     LFE    TopL    TopR    Ls      Rs */
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffff, 0x0001, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000
};
/* PCM7.1 */
#define COUNT_DITTER_8CH (32)
int16_t ditter_8ch[COUNT_DITTER_8CH] = {
	0xffff, 0x0001, 0xffff, 0x0000, 0xffff, 0x0000, 0x0001, 0xffff,
	0x0001, 0xffff, 0x0001, 0xffff, 0x0001, 0xffff, 0xffff, 0x0001,
	0xffff, 0x0001, 0xffff, 0x0000, 0xffff, 0x0001, 0x0000, 0x0000,
	0x0001, 0xffff, 0x0001, 0x0001, 0x0000, 0x0001, 0xffff, 0xffff
};

/******************************************************************************
 * utils
 *****************************************************************************/
static inline int16_t _clamp16(int32_t sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}

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

struct aml_channel_map *data_load_product_config(void)
{
	struct aml_channel_map *maps = NULL;
	int i = 0;

	maps = malloc(sizeof(struct aml_channel_map));
	if (!maps) {
		return NULL;
	}

	

	return maps;
}

struct aml_channel_map *data_get_product_chmaps(void)
{
	//TODO: load real product config from tvconfig.conf
	return &test_maps;
}

int data_get_channel_bit_mask(
	struct aml_channel_map *map, eChannelContentIdx channelName)
{
	if (map == NULL) {
		return AML_I2S_CHANNEL_NULL;
	}

	

	return map

	return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 * empty
 *****************************************************************************/
/*
 * 场景:
 *    由于AMl平台输出到 i2s/spdif/arc 时是走同一个 buffer 下去, 
 *    同时送给 is2 port 和 spdif(arc) port.
 *    所以做 Spdif 输出时:
 *        L/R: 会被tvserver mute,
 *        Surround: 需要在hal层将数据写成 0(TODO: 是否有其他好办法) 
 */
/* out channels = 6*/
int data_empty_channels(
		int16_t *out_buf,size_t frames, int channels, int channel_empty_mask)
{
    uint i, j;

    for (i = 0; i < frames; i ++) {
        for (j = 0; j < 2; j ++) {
            out_buf[channels * i + 4 + j] = 0;
        }
    }

    return 0;
}

/******************************************************************************
 * re-mix
 *****************************************************************************/
static int _data_remix_center_to_lr(int16_t *out_buf, size_t frames)
{
    ///< out_buf is 6ch pcm(L,R,C,lfe,Lr,Rs)
    int16_t center;
    int i;
    int tmp;
    
    for (i = 0; i < frames; i++)
    {
         ///< 3/0 input L_out/R_out =  = 0.707*(L/R + 0.707*C);
         center = out_buf[6*i + 2];
         
         tmp = out_buf[6*i] << 12;
         out_buf[6*i] = 
			_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12*center) >>12))>>12);
         
         tmp = out_buf[6*i+1] << 12;
         out_buf[6*i+1] =
			_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12*center) >>12))>>12);
    }

    return 0;
}

static int _data_remix_all_to_lr(int16_t *out_buf, size_t frames)
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
    int i, j, tmp;
    int16_t l,r,c,ls,rs;

    for (i = 0; i < frames; i++) {
        l = out_buf[6*i + 0];
        r = out_buf[6*i + 1];
        c = out_buf[6*i + 2];
        ls = out_buf[6*i + 4];
        rs = out_buf[6*i + 5];

        /* handle L channel */
        tmp = l << 12;
        out_buf[6*i] =
			_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12 * c + MINUS_3_DB_IN_Q19_12 * ls) >> 12)) >> 12);

        /* handle R channel */
        tmp = r << 12;
        out_buf[6*i + 1] =
			_clamp16((MINUS_3_DB_IN_Q19_12 * ((tmp + MINUS_3_DB_IN_Q19_12 * c + MINUS_3_DB_IN_Q19_12 * rs) >> 12)) >> 12);
    }
    return 0;
}

int data_remix_to_lr_channel(int16_t *out_buf, size_t frames, int channel_name)
{
	if (channel_name == CHANNEL_C) {
		return _data_remix_center_to_lr(out_buf, frames);
	}
	if (channel_name == CHANNEL_ALL) {
		return _data_remix_all_to_lr(out_buf, frames)
	}

	return 0
}

/******************************************************************************
 * extract
 *****************************************************************************/
int _data_extract_channel_lr(int16_t *out_buf, int16_t *in_buf, size_t frames, uint in_ch)
{
    uint i, j;
    if (out_buf == NULL || in_buf == NULL) {
        ALOGE("NULL pointer!");
        return -EINVAL;
    }

    if (in_ch < 2) {
        ALOGE("input channels less than output 2 channels!");
        return -EINVAL;
    }

    for (i = 0; i < frames; i++) {
        for (j = 0; j < 2; j++) {
            out_buf[2*i + j] = in_buf[in_ch*i + j];
        }
    }
    return 0;
}

int _data_extract_channel_lfe(int16_t *out_buf, int16_t *in_buf, size_t frames, uint in_ch)
{///< return 2ch lfe, be same. input 3ch(L+R+lfe), 6ch(L+R+C+lfe+Ls+Rs)
    uint i, j;
    if (out_buf == NULL || in_buf == NULL) {
        ALOGE("NULL pointer!");
        return -EINVAL;
    }

    if (in_ch != 3 && in_ch != 6) {
        ALOGE("input channels less than output 2 channels!");
        return -EINVAL;
    }

    if (in_ch == 3) {
        for (i = 0; i < frames; i++) {
            out_buf[2*i] = in_buf[in_ch*i+2];
            out_buf[2*i+1] = out_buf[2*i];
        }
    } else if (in_ch == 6) { //5.1ch 
        for (i = 0; i < frames; i++) {
            //L/R/C/lfe/Ls/Rs
            out_buf[2*i] = in_buf[in_ch*i + 3]; 
            out_buf[2*i+1] = out_buf[2*i];
        }
    }
    return 0;
}

int data_extract_channels
	(int16_t *out_buf, int16_t *in_buf, size_t frames, uint in_ch, int channel_name)
{
	if (channel_name == CHANNEL_LFE) {
		return _data_extract_channel_lfe(out_buf, in_buf, frames, in_ch);
	}
	if (channel_name == (CHANNEL_L|CHANNEL_R)) {
		return _data_extract_channel_lr(out_buf, in_buf, frames, in_ch);
	}

	return 0;
}

/******************************************************************************
 * insert lfe
 *****************************************************************************/
int data_insert_lfe_data(int16_t *out_buf, int16_t *lfe_buffer, size_t frames, int type)
{///< out_buf is 6ch pcm(L,R,C,lfe,Lr,Rs). lfe_buffer is 2ch,2ch is same
    uint i, j;
    int16_t center;
    int tmp;
    if (type == 1)
    {///< type1,for atmos, out_buf[lfe] = lfe_buffer
        for (i = 0; i < frames; i ++) {
            ///<target[lfe]=lfe
            out_buf[6*i+3] = lfe_buffer[2*i];
        }
    }
    else
    {///< type0, for missionimpossible, out_buf[lfe&c] = lfe_buffer,
        for (i = 0; i < frames; i ++) {
            ///< target[c]=lfe, target[lfe]=lfe
            out_buf[6*i+2] = lfe_buffer[2*i];
            out_buf[6*i+3] = lfe_buffer[2*i+1];
        }
    }  
    return 0;
}

/******************************************************************************
 * ditter
 *****************************************************************************/
int data_add_ditter(int16_t *buffer, size_t frames, int type, int ch)
{
	int32_t tmp_buffer = 0;
	uint i, j;

	//TODO: check the input data format is a safe way here
	if (type != 1) {
		/* not ATMOS */
		return 0;
	}

	j = 0;
	for (i=0; i<frames/2; i++) {
		switch(ch) {
		case 8:
			tmp_buffer = (int32_t)buffer[i] + (int32_t)ditter_8ch[j%COUNT_DITTER_8CH];
			break;
		default:
			tmp_buffer = (int32_t)buffer[i] + (int32_t)ditter_6ch[j%COUNT_DITTER_6CH];
			break;
		}
		j++;
		if (tmp_buffer > 32767 || tmp_buffer < -32768) {
			continue;
		} else {
			buffer[i] = (int16_t)tmp_buffer;
		}
	}

	return 0;
}

/******************************************************************************
 * replace
 *****************************************************************************/
/* out channels = 6, in channels = 2 */
int data_mv_23to45_replace_ch23(int16_t *out_buf, int16_t *in_buf, size_t frames)
{
    uint i, j;
    for (i = 0; i < frames; i ++) {
        for (j = 0; j < 2; j ++) {
			// out ch2->ch4
			// out ch3->ch5
            out_buf[6*i + 4 + j] = out_buf[6*i + 2 + j];
			// in  ch0-> out ch2
			// in  ch1-> out ch3
            out_buf[6*i + 2 + j] = in_buf[2*i + j];
        }
    }

    return 0;
}

/******************************************************************************
 * re-map
 *****************************************************************************/
/* AML asume that output is 32bits, 8ch and input always 16bits */
int pcmout_data_8ch_remap(int16_t *out_buf, int16_t *in_buf, size_t in_channels, size_t frames, int type)
{
    uint i,j;
    uint n = in_channels;

    if (in_channels != 2 && in_channels != 4 && in_channels != 6 && in_channels != 8) {
        ALOGE("data remap do not support this channel");
        return -EINVAL;
    }
    ///< output buf is 32bits, 8ch
    memset((char*)out_buf, 0, frames*8*4);
    if (type == 1)
    {///< type1,for atmos, out_buf[6&7] = Ls_bak&Rs_bak, out_buf[4&5] = 0;
        for (i = 0; i < frames; i ++) {
            for (j=0; j < n; j ++) {
                if ((j == 4) || (j == 5))
                {
                    out_buf[8*i + j + 2] = in_buf[n*i + j];
                }
                else if ((j == 6) || (j == 7))
                {
                    continue;
                }
                else 
                {
                    out_buf[8*i + j] = in_buf[n*i + j];
                }
            }
        }
    }
    else
    {///< type0, for missionimpossible, out_buf[4&5] = Ls_bak&Rs_bak out_buf[6&7] = 0
        for (i = 0; i < frames; i ++) {
            for (j=0; j < n; j ++) {
                out_buf[8*i + j] = in_buf[n*i + j];
            }
        }
    }
    

    return 0;
}

int pcmout_data_2ch_to_8ch_remap(int16_t *out_buf, int16_t *in_buf1, int16_t *in_buf2,
                                            int16_t *in_buf3, int16_t *in_buf4,
                                            size_t in_channels, size_t frames)
{
    uint i;
    if (in_channels != 2 && in_channels != 4 && in_channels != 6 && in_channels != 8) {
        ALOGE("data remap do not support this channel");
        return -EINVAL;
    }
    for (i = 0; i < frames; i ++) {
        if (in_channels >= 2) {
            out_buf[8*i + 0] = in_buf1[2*i + 0];
            out_buf[8*i + 1] = in_buf1[2*i + 1];
        }
        if (in_channels >= 4) {
            out_buf[8*i + 2] = in_buf2[2*i + 0];
            out_buf[8*i + 3] = in_buf2[2*i + 1];
        }
        if (in_channels >= 6) {
            out_buf[8*i + 4] = in_buf3[2*i + 0];
            out_buf[8*i + 5] = in_buf3[2*i + 1];
        }
        if (in_channels >= 8) {
            out_buf[8*i + 6] = in_buf4[2*i + 0];
            out_buf[8*i + 7] = in_buf4[2*i + 1];
        }
    }
    return 0;
}

//fecth data for DAP moduel(Dap module need 2ch || 6ch pcm)
int pcmout_data_channel_padding(int16_t *out_buf, int16_t *in_buf,
        size_t frames, uint in_ch, uint out_ch)
{
    uint i, j, m, n;

    m = in_ch;
    n = out_ch;
    if (out_buf == NULL || in_buf == NULL) {
        ALOGE("NULL pointer!");
        return -EINVAL;
    }
    
    if ((in_ch == 3 || in_ch == 2) && (out_ch == 2))
    {
        for (i=0; i<frames; i++ )
        {
            for (j = 0; j < out_ch; j++) 
            {
                out_buf[out_ch*i + j] = in_buf[in_ch*i + j];
            }
        }
    }
    else if ((in_ch == 6) && (out_ch == 6))
    {
        memcpy((char*)out_buf, (char*)in_buf, frames*6*sizeof(int16_t));
    }
    else
    {
        ALOGE("Invalid input ch=%d or output ch=%d !", in_ch, out_ch);
        return -EINVAL;
    }
    return 0;
}


static int delay[4096];
static int delay_frame = 1440 * 4;
static char *delay_start = (char *)delay;

int audio_effect_real_lfe_gain(short* buffer, int frame_size, int LPF_Gain)
{
    int i;
    short data_left, data_right;
    int output_size = frame_size << 2;
    float gain;
    int32_t tmp_sample;
    memcpy((delay_start + delay_frame), buffer, output_size);
    memcpy(buffer, delay_start, output_size);
    memmove(delay_start, (delay_start + output_size), delay_frame);
       
    gain = powf(10, (float)LPF_Gain/20);
    //ALOGE("audio_effect_real_lfe_gain gain %f\n", gain);   
    for (i = 0; i < frame_size; i++) {
        tmp_sample = (int32_t)buffer[i * 2 + 0] * gain;
        buffer[i * 2 + 0] = _clamp16(tmp_sample);
        tmp_sample = (int32_t)buffer[i * 2 + 1] * gain;
        buffer[i * 2 + 1] = _clamp16(tmp_sample);
    }

	return 0;
}

