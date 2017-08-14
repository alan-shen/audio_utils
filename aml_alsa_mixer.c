/**
 ** aml_alsa_mixer.c
 **
 ** This program is APIs for read/write mixers of alsa.
 ** author: shen pengru
 **
 */
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <cutils/log.h>
#include <fcntl.h>
#include "aml_hw_profile.h"
#include "aml_alsa_mixer.h"

#define LOG_TAG "audio_alsa_mixer"

static struct aml_mixer_list gAmlMixerList {
	{AML_MIXER_ID_I2S_MUTE,           "Audio i2s mute"},
	{AML_MIXER_ID_SPDIF_MUTE,         "Audio spdif mute"},
	{AML_MIXER_ID_SPDIF_ENABLE,       "Audio spdif enable"},
	{AML_MIXER_ID_AUDIO_IN_SRC,       "Audio In Source"},
	{AML_MIXER_ID_I2SIN_AUDIO_TYPE,   "I2SIN Audio Type"},
	{AML_MIXER_ID_SPDIFIN_AUDIO_TYPE, "SPDIFIN Audio Type"},
	{AML_MIXER_ID_HW_RESAMPLE_ENABLE, "Hardware resample enable"},
	{AML_MIXER_ID_OUTPUT_SWAP,        "Output Swap"},
};

static struct aml_mixer_ctrl gCtlI2sMute {
	{I2S_MUTE_ON,  "On"},
	{I2S_MUTE_OFF, "Off"},
};

static struct aml_mixer_ctrl gCtlSpdifMute {
	{SPDIF_MUTE_ON,  "On"},
	{SPDIF_MUTE_OFF, "Off"},
};

static struct aml_mixer_ctrl gCtlSpdifEn {
	{SPDIF_EN_ENABLE,  "On"},
	{SPDIF_EN_DISABLE, "Off"},
};

static struct aml_mixer_ctrl gCtlAudioInSrc {
	{AUDIOIN_SRC_LINEIN,  "LINEIN"},
	{AUDIOIN_SRC_ATV,     "ATV"},
	{AUDIOIN_SRC_HDMI,    "HDMI"},
	{AUDIOIN_SRC_SPDIFIN, "SPDIFIN"},
};

static struct aml_mixer_ctrl gCtlI2SInType {
	{I2SIN_AUDIO_TYPE_LPCM,      "LPCM"},
	{I2SIN_AUDIO_TYPE_NONE_LPCM, "NONE-LPCM"},
	{I2SIN_AUDIO_TYPE_UN_KNOWN,  "UN-KNOW"},
};

static struct aml_mixer_ctrl gCtlSpdifInType {
	{SPDIFIN_AUDIO_TYPE_LPCM,   "LPCM"},
	{SPDIFIN_AUDIO_TYPE_AC3,    "AC3"},
	{SPDIFIN_AUDIO_TYPE_EAC3,   "EAC3"},
	{SPDIFIN_AUDIO_TYPE_DTS,    "DTS"},
	{SPDIFIN_AUDIO_TYPE_DTSHD,  "DTS-HD"},
	{SPDIFIN_AUDIO_TYPE_TRUEHD, "TRUEHD"},
};

static struct aml_mixer_ctrl gCtlHwResample {
	{HW_RESAMPLE_DISABLE, "Disable"},
	{HW_RESAMPLE_48K,     "Enable:48K"},
	{HW_RESAMPLE_44K,     "Enable:44K"},
	{HW_RESAMPLE_32K,     "Enable:32K"},
	{HW_RESAMPLE_LOCK,    "Lock Resample"},
	{HW_RESAMPLE_UNLOCK,  "Unlock Resample"},
};

static struct aml_mixer_ctrl gCtlOutputSwap {
	{OUTPUT_SWAP_LR, "LR"},
	{OUTPUT_SWAP_LL, "LL"},
	{OUTPUT_SWAP_RR, "RR"},
	{OUTPUT_SWAP_RL, "RL"},
};

static char *_get_mixer_name_by_id(int mixer_id)
{
	int i;
	int cnt_mixer = sizeof(gAmlMixerList)/sizeof(struct aml_mixer_list);

	for (i=0; i<cnt_mixer; i++) {
		if (gAmlMixerList[i].id == mixer_id) {
			return gAmlMixerList[i].mixer_name;
		}
	}

	return NULL;
}

static struct mixer *_open_mixer_handle(int mixer_id)
{
	int card = 0;
	struct mixer *pmixer = NULL;

	card = aml_get_sound_card_main();
	if (card < 0) {
		ALOGE("[%s:%d] Failed to get sound card\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	pmixer = mixer_open(card);
	if (NULL == pmixer) {
		ALOGE("[%s:%d] Failed to open mixer\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	return pmixer;
}

static int _close_mixer_handle(struct mixer *pmixer)
{
	if (NULL != pmixer) {
		mixer_close(pmixer);
	}

	return 0;
}

static struct mixer_ctl *_get_mixer_ctl_handle(struct mixer *pmixer, int mix_id)
{
	struct mixer_ctl *pCtrl = NULL;

	if (_get_mixer_name_by_id(mix_id) != NULL) {
		pCtrl = mixer_get_ctl_by_name(pmixer,
					_get_mixer_name_by_id(mix_id));
	}

	return pCtrl;
}

int aml_mixer_ctrl_get_int(int mixer_id)
{
	struct mixer     *pMixer;
	struct mixer_ctl *pCtrl;
	int value = -1;

	pMixer = _open_mixer_handle();
	if (pMixer == NULL) {
		ALOGE("[%s:%d] Failed to open mixer\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pCtrl = _get_mixer_ctl_handle(pMixer, mixer_id);
	if (pCtrl == NULL) {
		ALOGE("[%s:%d] Failed to open mixer %s\n", __FUNCTION__, __LINE__,
				_get_mixer_name_by_id(mix_id));
		_close_mixer_handle(pMixer);
		return -1;
	}

	value = mixer_ctl_get_value(pctl, 0);

	_close_mixer_handle(pMixer);
	return value;
}

#if 0
int aml_mixer_ctrl_get_str(int mixer_id, char *value)
{
	struct mixer     *pMixer;
	struct mixer_ctl *pCtrl;
	char value[50];

	pMixer = _open_mixer_handle();
	if (pMixer == NULL) {
		ALOGE("[%s:%d] Failed to open mixer\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pCtrl = _get_mixer_ctl_handle(pMixer, mixer_id);
	if (pCtrl == NULL) {
		ALOGE("[%s:%d] Failed to open mixer %s\n", __FUNCTION__, __LINE__,
				_get_mixer_name_by_id(mix_id));
		_close_mixer_handle(pMixer);
		return -1;
	}

	strcpy(value, mixer_ctl_get_value(pctl, 0));

	_close_mixer_handle(pMixer);
	return 0;
}
#endif

int aml_mixer_ctrl_set_int(int mixer_id, int value)
{
	struct mixer     *pMixer;
	struct mixer_ctl *pCtrl;
	int value = -1;

	pMixer = _open_mixer_handle();
	if (pMixer == NULL) {
		ALOGE("[%s:%d] Failed to open mixer\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pCtrl = _get_mixer_ctl_handle(pMixer, mixer_id);
	if (pCtrl == NULL) {
		ALOGE("[%s:%d] Failed to open mixer %s\n", __FUNCTION__, __LINE__,
				_get_mixer_name_by_id(mix_id));
		_close_mixer_handle(pMixer);
		return -1;
	}

	mixer_ctl_set_value(pCtrl, 0, value);

	_close_mixer_handle(pMixer);
	return 0;
}

int aml_mixer_ctrl_set_str(int mixer_id, char *value)
{
	struct mixer     *pMixer;
	struct mixer_ctl *pCtrl;
	int value = -1;

	pMixer = _open_mixer_handle();
	if (pMixer == NULL) {
		ALOGE("[%s:%d] Failed to open mixer\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pCtrl = _get_mixer_ctl_handle(pMixer, mixer_id);
	if (pCtrl == NULL) {
		ALOGE("[%s:%d] Failed to open mixer %s\n", __FUNCTION__, __LINE__,
				_get_mixer_name_by_id(mix_id));
		_close_mixer_handle(pMixer);
		return -1;
	}

	mixer_ctl_set_enum_by_string(pCtrl, value);

	_close_mixer_handle(pMixer);
	return 0;
}
