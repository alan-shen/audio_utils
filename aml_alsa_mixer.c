/**
 ** aml_alsa_mixer.c
 **
 ** This program is APIs for read/write mixers of alsa.
 ** author: shen pengru
 **
 */
#define LOG_TAG "audio_data_utils"                                              
                                                                                
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <cutils/log.h>
#include <fcntl.h>

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
	{I2S_MUTE_ON,  ""},
	{I2S_MUTE_OFF, ""},
};

static struct aml_mixer_ctrl gCtlSpdifMute {
	{SPDIF_MUTE_ON, ""},
	{SPDIF_MUTE_OFF, ""},
};

static struct aml_mixer_ctrl gCtlSpdifEn {
	{SPDIF_EN_ENABLE, ""},
	{SPDIF_EN_DISABLE, ""},
};

static struct aml_mixer_ctrl gCtlAudioInSrc {
	{AUDIOIN_SRC_LINEIN, ""},
	{AUDIOIN_SRC_ATV, ""},
	{AUDIOIN_SRC_HDMI, ""},
	{AUDIOIN_SRC_SPDIFIN, ""},
};

static struct aml_mixer_ctrl gCtlI2SInType {
	{I2SIN_AUDIO_TYPE_LPCM, ""},
	{I2SIN_AUDIO_TYPE_NONE_LPCM, ""},
	{I2SIN_AUDIO_TYPE_UN_KNOWN, ""},
};

static struct aml_mixer_ctrl gCtlSpdifInType {
	{SPDIFIN_AUDIO_TYPE_LPCM, ""},
	{SPDIFIN_AUDIO_TYPE_AC3, ""},
	{SPDIFIN_AUDIO_TYPE_EAC3, ""},
	{SPDIFIN_AUDIO_TYPE_DTS, ""},
	{SPDIFIN_AUDIO_TYPE_DTSHD, ""},
	{SPDIFIN_AUDIO_TYPE_TRUEHD, ""},
};

static struct aml_mixer_ctrl gCtlHwResample {
	{HW_RESAMPLE_DISABLE, ""},
	{HW_RESAMPLE_48K, ""},
	{HW_RESAMPLE_44K, ""},
	{HW_RESAMPLE_32K, ""},
	{HW_RESAMPLE_LOCK, ""},
	{HW_RESAMPLE_UNLOCK, ""},
};

static struct aml_mixer_ctrl gCtlOutputSwap {
	{OUTPUT_SWAP_LR, ""},
	{OUTPUT_SWAP_LL, ""},
	{OUTPUT_SWAP_RR, ""},
	{OUTPUT_SWAP_RL, ""},
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

static int _get_aml_sound_card(void) {
    static const char * const SOUND_CARDS_PATH = "/proc/asound/cards";
	int card = -1, err = -1;
	int fd = -1;
	unsigned fileSize = 512;
	char *read_buf = NULL, *pd = NULL;

	fd = open(SOUND_CARDS_PATH, O_RDONLY);
	if (fd < 0) {
		ALOGE("ERROR: failed to open config file %s error: %d\n",
			SOUND_CARDS_PATH, errno);
		return -EINVAL;
	}

	read_buf = (char *) malloc(fileSize);
	if (!read_buf) {
		ALOGE("Failed to malloc read_buf");
		close(fd);
		return -ENOMEM;
	}
	memset(read_buf, 0x0, fileSize);
	err = read(fd, read_buf, fileSize);
	if (err < 0) {
		ALOGE("ERROR: failed to read config file %s error: %d\n",
			SOUND_CARDS_PATH, errno);
		close(fd);
		free(read_buf);
		return -EINVAL;
	}
	pd = strstr(read_buf, "AML");
	card = *(pd - 3) - '0';

	free(read_buf);
	close(fd);

	return card;
}

static struct mixer *_get_mixer_handle(int mixer_id)
{
	int card = 0;
	struct mixer *pmixer = NULL;

	card = _get_aml_sound_card();
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

	pMixer = _get_mixer_handle();
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

	pMixer = _get_mixer_handle();
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

	pMixer = _get_mixer_handle();
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

	pMixer = _get_mixer_handle();
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
