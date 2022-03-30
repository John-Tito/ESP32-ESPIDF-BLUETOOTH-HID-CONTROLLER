#ifndef _USER_LOGI_H
#define _USER_LOGI_H
#include <esp_log.h>
#define TRACE_LOG(fmt, ...) ESP_LOGI("[%s:%d]:" ##fmt,  __FILE__, __LINE__, ##__VA_ARGS__)
// #define TRACE_LOG(LEVEL, INFO) printf("[%c][%s:%d] %s:%s\n", ((LEVEL == 0) ? ('I') : ((LEVEL == 1) ? ('W') : ('E'))), __FILE__, __LINE__, __func__, INFO)
// #define TRACE_LOG(LEVEL,fmt, ...) printf("[%c][%s:%d]:"fmt, ((LEVEL == 0) ? ('I') : ((LEVEL == 1) ? ('W') : ('E'))), __FILE__, __LINE__, ##__VA_ARGS__)
// #define printf_my(fmt, ...) printf("%s %s %s %s %d:" fmt, __FILE__, __FUNCTION__, __DATE__, __TIME__, __LINE__, ##__VA_ARGS__)
#endif