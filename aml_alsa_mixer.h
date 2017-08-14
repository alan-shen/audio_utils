#ifndef _AML_ALSA_MIXER_H_
#define _AML_ALSA_MIXER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Value of the Alsa Mixer Control Point
 **/
/* Audio i2s mute */
typedef enum MIXER_AUDIO_I2S_MUTE {
	I2S_MUTE_OFF = 0,
	I2S_MUTE_ON  = 1,
	I2S_MUTE_MAX,
} eMixerAudioI2sMute;

/* Audio spdif mute */
typedef enum MIXER_SPDIF_MUTE {
	SPDIF_MUTE_OFF = 0,
	SPDIF_MUTE_ON  = 1,
	SPDIF_MUTE_MAX,
} eMixerSpdifMute;

/* Audio spdif enable */
typedef enum MIXER_SPDIF_ENABLE {
	SPDIF_EN_DISABLE = 0,
	SPDIF_EN_ENABLE  = 1,
	SPDIF_EN_MAX,
} eMixerSpdifEnable;

/* Audio In Source */
typedef enum MIXER_AUDIO_IN_SOURCE {
	AUDIOIN_SRC_LINEIN  = 0,
	AUDIOIN_SRC_ATV     = 1,
	AUDIOIN_SRC_HDMI    = 2,
	AUDIOIN_SRC_SPDIFIN = 3,
	AUDIOIN_SRC_MAX,
} eMixerAudioInSrc;

/* Audio I2SIN Audio Type */
typedef enum MIXER_I2SIN_AUDIO_TYPE {
	I2SIN_AUDIO_TYPE_LPCM      = 0,
	I2SIN_AUDIO_TYPE_NONE_LPCM = 1,
	I2SIN_AUDIO_TYPE_UN_KNOWN  = 2,
	I2SIN_AUDIO_TYPE_MAX,
} eMixerI2sInAudioType;

/* Audio SPDIFIN Audio Type */
typedef enum MIXER_SPDIFIN_AUDIO_TYPE {
	SPDIFIN_AUDIO_TYPE_LPCM   = 0,
	SPDIFIN_AUDIO_TYPE_AC3    = 1,
	SPDIFIN_AUDIO_TYPE_EAC3   = 2,
	SPDIFIN_AUDIO_TYPE_DTS    = 3,
	SPDIFIN_AUDIO_TYPE_DTSHD  = 4,
	SPDIFIN_AUDIO_TYPE_TRUEHD = 5,
	SPDIFIN_AUDIO_TYPE_MAX,
} eMixerApdifinAudioType;

/* Hardware resample enable */
typedef enum MIXER_HW_RESAMPLE_ENABLE {
	HW_RESAMPLE_DISABLE = 0,
	HW_RESAMPLE_48K     = 1,
	HW_RESAMPLE_44K     = 2,
	HW_RESAMPLE_32K     = 3,
	HW_RESAMPLE_LOCK    = 4,
	HW_RESAMPLE_UNLOCK  = 5,
	HW_RESAMPLE_MAX,
} eMixerHwResample;

/* Output Swap */
typedef enum MIXER_OUTPUT_SWAP {
	OUTPUT_SWAP_LR = 0,
	OUTPUT_SWAP_LL = 1,
	OUTPUT_SWAP_RR = 2,
	OUTPUT_SWAP_RL = 3,
	OUTPUT_SWAP_MAX,
} eMixerOutputSwap;

struct aml_mixer_ctrl {
	int  ctrl_idx;
	char ctrl_name[50];
};

/*
 *  Alsa Mixer Control Command List
 **/
typedef enum AML_MIXER_CTRL_ID {
	AML_MIXER_ID_I2S_MUTE = 0,
	AML_MIXER_ID_SPDIF_MUTE,
	AML_MIXER_ID_SPDIF_ENABLE,
	AML_MIXER_ID_AUDIO_IN_SRC,
	AML_MIXER_ID_I2SIN_AUDIO_TYPE,
	AML_MIXER_ID_SPDIFIN_AUDIO_TYPE,
	AML_MIXER_ID_HW_RESAMPLE_ENABLE,
	AML_MIXER_ID_OUTPUT_SWAP,
	AML_MIXER_ID_MAX,
} eMixerCtrlID;

struct aml_mixer_list {
	int  id;
	char mixer_name[50];
};

/*
 * get interface
 **/
int aml_mixer_ctrl_get_int(int mixer_id);
//int aml_mixer_ctrl_get_str(int mixer_id, char *value);
// or
#if 0
int aml_mixer_get_audioin_src(int mixer_id);
int aml_mixer_get_i2sin_type(int mixer_id);
int aml_mixer_get_spdifin_type(int mixer_id);
#endif

/*
 * set interface
 **/
int aml_mixer_ctrl_set_int(int mixer_id, int value);
int aml_mixer_ctrl_set_str(int mixer_id, char *value);

#ifdef __cplusplus
}
#endif

#endif
