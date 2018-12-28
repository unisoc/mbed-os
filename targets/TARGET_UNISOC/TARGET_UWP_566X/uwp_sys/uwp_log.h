#ifndef MBED_UWP_LOG_H
#define MBED_UWP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define WIFI_LOG_ERR
#define WIFI_LOG_WRN
#define WIFI_LOG_DBG
#define WIFI_LOG_INF

#ifdef  WIFI_LOG_ERR
#define LOG_ERR(fmt, ...) do {\
            mbed_error_printf("%s ERR: "fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_ERR(fmt, ...)
#endif

#ifdef  WIFI_LOG_WRN
#define LOG_WRN(fmt, ...) do {\
		    printf("%s WRN: "fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_WRN(fmt, ...)
#endif

#ifdef  WIFI_LOG_DBG
#define LOG_DBG(fmt, ...) do {\
		    printf("%s DBG: "fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_DBG(fmt, ...)
#endif

#ifdef  WIFI_LOG_INF
#define LOG_INF(fmt, ...) do {\
		    printf("%s INF: "fmt"\r\n", __func__, ##__VA_ARGS__);\
	}while(0)
#else
#define LOG_INF(fmt, ...)
#endif

#define printk mbed_error_printf

#ifdef __cplusplus
}
#endif

#endif

