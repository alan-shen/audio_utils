#ifndef _AML_ANDROID_UTILS_H_
#define _AML_ANDROID_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AML_BOOLEAN_DEFINE {
	eAML_BOOL_FALSE = 0,
	eAML_BOOL_TRUE  = 1,
}

/*
 * Android Property Interface
 */
int aml_getprop_bool(const char *path);
int aml_getprop_int(const char *path);

/*
 * Sys Fs Interface
 */
int aml_sysfs_get_int(const char *path);
int aml_sysfs_get_int16(const char *path, unsigned *value);
int aml_sysfs_set_int(const char *path, int value);
char *aml_sfsfs_get_string(const char *path);
int aml_sysfs_set_string(const char *path, const char *value);

/*
 * Others
 */
int aml_strstr(char *mystr,char *substr);

#ifdef __cplusplus
}
#endif

#endif
