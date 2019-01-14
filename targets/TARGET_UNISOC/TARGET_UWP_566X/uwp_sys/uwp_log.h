#ifndef MBED_UWP_LOG_H
#define MBED_UWP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define WIFI_LOG_ERR
//#define WIFI_LOG_WRN
//#define WIFI_LOG_DBG
//#define WIFI_LOG_INF
//#define WIFI_DUMP

#ifdef  WIFI_LOG_ERR
#define LOG_ERR(fmt, ...) do {\
            mbed_error_printf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_ERR(fmt, ...)
#endif

#ifdef  WIFI_LOG_WRN
#define LOG_WRN(fmt, ...) do {\
		    printf("%s"fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_WRN(fmt, ...)
#endif

#ifdef  WIFI_LOG_DBG
#define LOG_DBG(fmt, ...) do {\
		    mbed_error_printf(fmt"\r\n", ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_DBG(fmt, ...)
#endif

#ifdef  WIFI_LOG_INF
#define LOG_INF(fmt, ...) do {\
		    printf(fmt"\r\n",##__VA_ARGS__);\
	}while(0)
#else
#define LOG_INF(fmt, ...)
#endif

#ifdef  WIFI_DUMP
#define DUMP_DATA(buff, len) do {\
	        u8_t *data = (u8_t *)buff;\
		    for(int i=1;i<=len;i++){\
				mbed_error_printf("%02x ",data[i-1]);\
				if(i%10 == 0)\
					mbed_error_printf("\r\n");\
		    	}\
		    mbed_error_printf("\r\n");\
	}while(0)
#else
#define DUMP_DATA(buff, len)
#endif

#define WIFI_ASSERT(expression,fmt,...) do {\
            if(!(expression)){\
                mbed_error_printf("fatal err:%s   ",__func__);\
                mbed_error_printf(fmt"\r\n",##__VA_ARGS__);\
            }\
       } while(0)

#define printk mbed_error_printf

#ifdef __cplusplus
}
#endif

#endif

