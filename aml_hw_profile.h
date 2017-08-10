#ifndef _AML_HW_PROFILE_H_
#define _AML_HW_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

int aml_get_sound_card_main(void);
int aml_get_sound_card_ext(int type);

int aml_get_spdif_port(void);
int aml_get_pcm_bt_port(void);

char* aml_get_hdmi_sink_cap(const char *keys);
char* aml_get_hdmi_arc_cap(unsigned *ad, int maxsize, const char *keys);

#ifdef __cplusplus
}
#endif

#endif
