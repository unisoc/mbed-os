#ifndef __BLUETOOTH_UKI_LOG_H
#define __BLUETOOTH_UKI_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int uki_log_init(void);
void uki_log_print(unsigned char level, unsigned char type, const char* fmt, ...);

void hex_dump_block(char *tag, unsigned char *bin, size_t binsz);
#define UKI_DUMP(data, length) hex_dump_block(NULL, data, length)
#define HCI_DUMP(tag, data, length) hex_dump_block(tag, data, length)


#if 0
#define UKI_DBG(...) uki_log_print(0, 0, ##__VA_ARGS__)
#define BT_INFO(...) uki_log_print(0, 0, ##__VA_ARGS__)
#define BT_WARN(...) uki_log_print(0, 0, ##__VA_ARGS__)
#define BT_ERR(...) uki_log_print(0, 0, ##__VA_ARGS__)
#define BT_DBG(...) uki_log_print(0, 0, ##__VA_ARGS__)
#endif

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

extern int bt_log_level;

#define BT_ERR(fmt, ...) 									\
        do {                                                \
            if (bt_log_level >= LOG_LEVEL_ERROR) {      \
                printf(fmt"\n", ##__VA_ARGS__);             \
            }                                               \
        }while(0)
    
    
#define BT_WARN(fmt, ...) 									\
        do {                                                \
            if (bt_log_level >= LOG_LEVEL_WARNING) {    \
                printf(fmt"\n", ##__VA_ARGS__);             \
            }                                               \
        }while(0)
    
#define BT_INFO(fmt, ...) 									\
        do {                                                \
            if (bt_log_level >= LOG_LEVEL_INFO) {       \
                printf(fmt"\n", ##__VA_ARGS__);             \
            }                                               \
        }while(0)
    
#define BT_DBG(fmt, ...) 									\
        do {                                                \
            if (bt_log_level >= LOG_LEVEL_DEBUG) {      \
                printf(fmt"\n", ##__VA_ARGS__);             \
            }                                               \
        }while(0)
    
#define UKI_DBG  BT_INFO

#ifdef __cplusplus
}
#endif
#endif /* __BLUETOOTH_UKI_LOG_H */
