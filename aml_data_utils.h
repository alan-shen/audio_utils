/**
 ** aml_data_utils.h
 ** author: shen pengru
 **
 */
#ifndef _AML_DATA_UTILS_H_
#define _AML_DATA_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  Channel name index, we should transfor the idx to real bit mask on i2s data
 *  lines here, for the PRODUCT.
 *  For example:
 *      -----------------------------------------------------
 *                                      || IDX     || BitMask
 *      -----------------------------------------------------
 *      case 1: [2.0][normal][pulpfiction]
 *                                         L/R     -> I2S_01
 *      case 2: [2.0][matrix]
 *                                         L/R     -> I2S_23
 *      case 3: [4.1][missionimpossible]
 *                                         L/R     -> I2S_01
 *                                         LFE/LFE -> I2S_23
 *                                         Ls/Rs   -> I2S_45
 *      case 4: [5.1]
 *                                         L/R     -> I2S_01
 *                                         C/LFE   -> I2S_23
 *                                         Ls/Rs   -> I2S_45
 *      case 5: [5.1.2][atmos]
 *                                         L/R     -> I2S_01
 *                                         C/LFE   -> I2S_23
 *                                         Lt/Rt   -> I2S_45
 *                                         Ls/Rs   -> I2S_67
 *      -----------------------------------------------------
 *  
 *****************************************************************************/
typedef enum CHANNEL_CONTENT_INDEX {
	AML_CH_IDX_L         = 0,
	AML_CH_IDX_R         = 1,
	AML_CH_IDX_C         = 2,
	AML_CH_IDX_LFE       = 3,
	AML_CH_IDX_LS        = 4,
	AML_CH_IDX_RS        = 5,
	AML_CH_IDX_LBS       = 6,
	AML_CH_IDX_RBS       = 7,
	AML_CH_IDX_LT        = 8,
	AML_CH_IDX_RT        = 9,
	AML_CH_IDX_MAX       = 10,
	AML_CH_IDX_5_1_ALL   = 0x003F,
	AML_CH_IDX_7_1_ALL   = 0x00FF,
	AML_CH_IDX_5_1_2_ALL = 0x033F,
} eChannelContentIdx;

/******************************************************************************
 *  AML I2S Channel Bit Map:
 *  ----------------------
 *  I2S Port ||    I2S Ch
 *  ----------------------
 *    I2S_01 -- CHANNEL_0
 *           `- CHANNEL_1
 *    I2S_23 -- CHANNEL_2
 *           `- CHANNEL_3
 *    I2S_45 -- CHANNEL_4
 *           `- CHANNEL_5
 *    I2S_67 -- CHANNEL_6
 *           `- CHANNEL_7
 *
 *****************************************************************************/
typedef enum I2S_DATALINE_INDEX {
	AML_I2S_PORT_IDX_01   = 0,
	AML_I2S_PORT_IDX_23   = 1,
	AML_I2S_PORT_IDX_45   = 2,
	AML_I2S_PORT_IDX_67   = 3,
	AML_I2S_PORT_IDX_NULL = 0x8000,
	AML_I2S_PORT_IDX_MAX  = 0xFFFF,
} eI2SDataLineIdx;

typedef enum CHANNEL_ON_I2S_BIT_MASK{
	AML_I2S_CHANNEL_0    = 0x1<<0,
	AML_I2S_CHANNEL_1    = 0x1<<1,
	AML_I2S_CHANNEL_2    = 0x1<<2,
	AML_I2S_CHANNEL_3    = 0x1<<3,
	AML_I2S_CHANNEL_4    = 0x1<<4,
	AML_I2S_CHANNEL_5    = 0x1<<5,
	AML_I2S_CHANNEL_6    = 0x1<<6,
	AML_I2S_CHANNEL_7    = 0x1<<7,
	AML_I2S_CHANNEL_NULL = 0x1<<31,
} eChOnI2SBitMask;

#define AML_I2S_CHANNEL_COUNT  (8)

struct aml_channel_map {
	eChannelContentIdx channel_idx;
	// WARNNING: support map to only one i2s data line 
	eI2SDataLineIdx    i2s_idx;
	// WARNNING: may be one channel will map to 2 i2s data channel
	// eg, i2s_23 -> lfe/lfe
	eChOnI2SBitMask    bit_mask;
};

#if 0
struct aml_product_channel_maps {
	struct aml_channel_map l;   // left
	struct aml_channel_map r;   // right
	struct aml_channel_map c;   // center
	struct aml_channel_map lfe; // lfe
	struct aml_channel_map ls;  // left  side surround
	struct aml_channel_map rs;  // rihgt side surround
	struct aml_channel_map lbs; // left  back surround
	struct aml_channel_map rbs; // right back surround
	struct aml_channel_map lt;  // left  top
	struct aml_channel_map rt;  // right top
};
#endif

/******************************************************************************
 * Function: data_load_product_config()
 * Description:
 *      load channel maps for current product
 * Input:  NULL
 * Output: NULL
 * Return: Hw I2S Ch Maps
 *****************************************************************************/
struct aml_channel_map *data_load_product_config(void);

/******************************************************************************
 * Function: data_get_product_chmaps()
 * Description:
 *      get channel maps of current product
 * Input:  NULL
 * Output: NULL
 * Return: Hw I2S Ch Maps
 *****************************************************************************/
struct aml_channel_map *data_get_product_chmaps(void);

/******************************************************************************
 * Function: data_get_channel_bit_mask()
 * Description:
 *       get hw i2s bit mask for channel "channelName"
 * Input:  eChannelContentIdx
 * Output:
 * Return: eChOnI2SBitMask
 *****************************************************************************/
int data_get_channel_bit_mask(
	struct aml_channel_map *map,
	eChannelContentIdx channelName);

/******************************************************************************
 * Function: data_remix_to_lr_channel()
 * Description: re-mix data to LR channel
 * Input:
 *      buf                        - Input Buffer
 *      frames                     - Count of frames
 *      framesz                    - Frame size(eg, 16bits->2, 32bits->4)
 *      channels                   - Count of channels
 *      channel_remix_src_bit_mask - Which channel will be remix
 * Output:
 *      buf                        - Output Buffer
 * Return: Zero if success
 *****************************************************************************/
int data_remix_to_lr_channel(
	void    *buf,
	size_t  frames,
    size_t  framesz,
	int     channels,
	int     channel_remix_src_bit_mask);

/******************************************************************************
 * Function: data_extend_to_8channels()
 * Description: extend the data 
 * Input:
 *     out_channels - channel count of output
 *     out_maps     - Channel Map of output
 *     out_framesz  - Frame size(eg, 16bits->2, 32bits->4)
 *     in_buf       - Input Buffer
 *     in_channels  - channel count of input
 *     in_framesz   - Frame size(eg, 16bits->2, 32bits->4)
 *     in_maps      - Channel Map of input
 *     frames       - frame count
 * Output:
 *     out_buf      - output buffer
 * Return: Zero if success
 *****************************************************************************/
int data_extend_to_8channels(
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	struct  aml_channel_map out_maps;
	void    *in_buf,
	size_t  in_channels,
	size_t  in_framesz,
	struct  aml_channel_map in_maps;
	size_t  frames);

/******************************************************************************
 * Function: data_extract_channels()
 * Description: extract channel data
 * Input:
 *     out_channels             - channel count of output
 *     out_framesz              - frame size of output data
 *     in_buf                   - input buffer
 *     in_channels              - channel count of input
 *     in_framesz               - frame szie of input data 
 *     frames                   - frame count
 *     channel_extract_bit_mask - eChOnI2SBitMask, ch mask will be extract
 * Output:
 *     out_buf                  - output buffer
 * Return: Zero if success
 *****************************************************************************/
int data_extract_channels(
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	void    *in_buf,
	size_t  in_channels,
	size_t  in_framesz,
	size_t  frames,
	int     channel_extract_bit_mask);

/******************************************************************************
 * Function: data_exchange_i2s_channels()
 * Description:
 *     exchange data between i2s_idx1 and i2s_idx2
 *     only support exchange between different i2s port
 * Input:
 *     buf      - input data
 *     channels - channel count
 *     frames   - frame count
 *     framesz  - frame size
 *     i2s_idx1 - index of i2s data line 1 for exchange
 *     i2s_idx2 - index of i2s data line 2 for exchange
 * Output:
 *     buf      - output data
 * Return: Zero if success
 *****************************************************************************/
int data_exchange_i2s_channels(
	void    *buf,
	size_t  channels,
	size_t  frames,
	size_t  framesz,
	int     i2s_idx1,
	int     i2s_idx2);

/******************************************************************************
 * Function: data_invert_channels()
 * Description:
 *     Invert the data of masked channels
 * Input:
 *     buf                     - input data
 *     channels                - channel count
 *     frames                  - frame count
 *     framesz                 - frame size
 *     channel_invert_bit_mask - eChOnI2SBitMask, bit mask of channels which will
 *                               be invert
 * Output:
 *     buf                     - output data
 * Return: Zero if success
 *****************************************************************************/
int data_invert_channels(
	void    *buf,
	int     channels,
	size_t  frames,
	size_t  framesz,
	int     channel_invert_bit_mask);

/******************************************************************************
 * Function: data_concat_channels()
 * Description:
 *     connect 4*2ch data to one 8ch data.
 * Input:
 *     out_channels - channel count of output
 *     out_framesz  - frame size of output data
 *     in_buf1      - input 2ch buffer 1
 *     in_buf2      - input 2ch buffer 2
 *     in_buf3      - input 2ch buffer 3
 *     in_buf4      - input 2ch buffer 4
 *     in_channels  - channel count of all input data
 *     in_framesz   - frame size of input data
 *     frames       - frame count
 * Output:
 *     out_buf      - output data
 * Return: Zero if success
 *****************************************************************************/
int data_concat_channels(
	void    *out_buf,
	size_t  out_channels,
	size_t  out_framesz,
	void    *in_buf1, void *in_buf2, void *in_buf3, void *in_buf4,
	size_t  in_channels,
	size_t  in_framesz,
	size_t  frames);

/******************************************************************************
 * Function: data_empty_channels()
 * Description:
 *     clean channel data
 * Input:
 *     buf                    - input data
 *     frames                 - frame count
 *     framesz                - frame size
 *     channels               - channel count
 *     channel_empty_bit_mask - eChOnI2SBitMask, bit mask of channels which will
 *                              be empty
 * Output:
 *     buf                    - output data
 * Return: Zero if success
 *****************************************************************************/
int data_empty_channels(
	void    *buf,
	size_t  frames,
	size_t  framesz,
	int     channels,
	int     channel_empty_bit_mask);

/******************************************************************************
 * Function: data_add_ditter_to_channels()
 * Description:
 *     add ditter
 * Input:
 *     buffer                  - input data
 *     channels                - channel count
 *     frames                  - frame count
 *     framesz                 - frame size
 *     channel_ditter_bit_mask - eChOnI2SBitMask, bit mask of channels which will
 *                               be add ditter
 * Output:
 *     buffer                  - output data
 * Return: Zero if success
 *****************************************************************************/
int data_add_ditter_to_channels(
	void    *buffer,
	size_t  frames,
	size_t  framesz,
	int     channels,
	int     channel_ditter_bit_mask);

/******************************************************************************
 * Function: data_replace_lfe_data()
 * Description:
 *     replace lfe data
 * Input:
 *     out_channles            - channel count of putput data
 *     out_framesz             - frame size of output
 *     input_lfe_buffer        - input lfe data
 *     in_channles             - channel count of input data
 *     in_framesz              - frame size of input
 *     frames                  - frame count
 *     channel_insert_bit_mask - eChOnI2SBitMask, bit mask of lfe channel
 * Output:
 *     out_buf                 - output data
 * Return: Zero if success
 *****************************************************************************/
int data_replace_lfe_data(
	void    *out_buf,
	size_t  out_channles,
	size_t  out_framesz,
	void    *input_lfe_buffer,
	size_t  in_channles,
	size_t  in_framesz,
	size_t  frames,
	int     channel_insert_bit_mask);

#ifdef __cplusplus
}
#endif

#endif
