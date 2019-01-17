#ifndef __BLUETOOTH_UKI_SETTINGS_H
#define __BLUETOOTH_UKI_SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "uki_log.h"
#include <settings/settings.h>

typedef void (*load_cb)(char *name, char *val, void *cb_arg);
struct settings_store_itf {
	int (*csi_load)(struct settings_store *cs, load_cb cb, void *cb_arg);
	int (*csi_save_start)(struct settings_store *cs);
	int (*csi_save)(struct settings_store *cs, const char *name,
			const char *value);
	int (*csi_save_end)(struct settings_store *cs);
};

char *settings_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len);


#endif
