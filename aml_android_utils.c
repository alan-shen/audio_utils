#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

/*
 * Android Property
 */
int aml_getprop_bool(const char * path)
{
    char buf[PROPERTY_VALUE_MAX];
    int ret = -1;

    ret = property_get(path, buf, NULL);
    if (ret > 0) {
        if (strcasecmp(buf, "true") == 0 || strcmp(buf, "1") == 0)
            return 1;
    }

    return 0;
}
                                                                              
int aml_getprop_int(const char *path)
{
    char buf[PROPERTY_VALUE_MAX];
    int ret = -1;
    int value = 0;

    ret = property_get(path, buf, NULL);
    if (ret > 0) {
        sscanf(buf, "%d", &value);
    }
    return value;
}

/*
 * Linux Sys Fs Set/Get Interface
 */
int aml_sysfs_get_int (const char *path)                                            
{                                                                               
  int val = 0;                                                                  
  int fd = open (path, O_RDONLY);                                               
  if (fd >= 0)                                                                  
    {                                                                           
      char bcmd[16];                                                            
      read (fd, bcmd, sizeof (bcmd));                                           
      val = strtol (bcmd, NULL, 10);                                            
      close (fd);                                                               
    }                                                                           
  else                                                                          
    {                                                                           
      LOGFUNC ("[%s]open %s node failed! return 0\n", path, __FUNCTION__);      
    }                                                                           
  return val;                                                                   
}

int aml_sysfs_get_int16(const char *path,unsigned *value)                                              
{                                                                                                  
    int fd;                                                                                        
    char valstr[64];                                                                               
    unsigned  val = 0;                                                                             
    fd = open(path, O_RDONLY);                                                                     
    if (fd >= 0) {                                                                                 
        memset(valstr, 0, 64);                                                                     
        read(fd, valstr, 64 - 1);                                                                  
        valstr[strlen(valstr)] = '\0';                                                             
        close(fd);                                                                                 
    } else {                                                                                       
        ALOGE("unable to open file %s\n", path);                                                   
        return -1;                                                                                 
    }                                                                                              
    if (sscanf(valstr, "0x%lx", &val) < 1) {                                                       
        ALOGE("unable to get pts from: %s", valstr);                                               
        return -1;                                                                                 
    }                                                                                              
    *value = val;                                                                                  
    return 0;                                                                                      
} 

int aml_sysfs_set_str(const char *path, const char *val)                     
{                                                                               
    int fd;                                                                     
    int bytes;                                                                  
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);                          
    if (fd >= 0) {                                                              
        bytes = write(fd, val, strlen(val));                                    
        close(fd);                                                                    
        return 0;                                                                     
    } else {                                                                    
        ALOGE("unable to open file %s,err: %s", path, strerror(errno));         
    }                                                                           
    return -1;                                                                  
}

/*
 * Aml private strstr
 */
int aml_strstr(char *mystr,char *substr) {
    int i=0;
    int j=0;
    int score = 0;
    int substrlen = strlen(substr);
    int ok = 0;
    for (i =0;i < 1024 - substrlen;i++) {
        for (j = 0;j < substrlen;j++) {
            score += (substr[j] == mystr[i+j])?1:0;
        }
        if (score == substrlen) {
           ok = 1;
                   break;
        }
        score = 0;
        }
    return ok;
}
