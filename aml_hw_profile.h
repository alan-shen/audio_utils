#ifndef _AML_HW_PROFILE_H_
#define _AML_HW_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CARD_NUM    2

#define SOUND_CARDS_PATH  "/proc/asound/cards"
#define SOUND_PCM_PATH    "/proc/asound/pcm";

#define PCM_I2S_STRING    "I2S"
#define PCM_SPDIF_STRING  "SPDIF"
#define PCM_PCM2BT_STRING "pcm2bt-pcm"

typedef enum PCM_STREAM_TYPE {
	AML_STREAM_TYPE_PLAYBACK = 0,
	AML_STREAM_TYPE_CAPTURE  = 1,
	AML_STREAM_TYPE_MAX,
} ePcmStreamType;

int aml_get_sound_card_main(void);
int aml_get_sound_card_ext(ePcmStreamType type);

int aml_get_i2s_port(void);
int aml_get_spdif_port(void);
int aml_get_pcm2bt_port(void);

char* aml_get_hdmi_sink_cap(const char *keys);
char* aml_get_hdmi_arc_cap(unsigned *ad, int maxsize, const char *keys);

#ifdef __cplusplus
}
#endif

#endif
