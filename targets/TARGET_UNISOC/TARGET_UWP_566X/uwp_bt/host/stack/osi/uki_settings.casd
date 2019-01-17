#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "uki_settings.h"
#include <settings/settings.h>
#include <bluetooth/bluetooth.h>
#include "uki_zif.h"

struct settings_dup_check_arg {
	const char *name;
	const char *val;
	int is_dup;
};

struct settings_store *settings_save_dst;

char *settings_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len)
{
#if 0
	if (BASE64_ENCODE_SIZE(vp_len) > buf_len) {
		return NULL;
	}

	size_t enc_len;

	base64_encode(buf, buf_len, &enc_len, vp, vp_len);

	return buf;
#else
    //TODO fix me
    return NULL;
#endif
}

void bt_settings_encode_key(char *path, size_t path_size, const char *subsys,
			    bt_addr_le_t *addr, const char *key)
{
	if (key) {
		snprintf(path, path_size,
			 "bt/%s/%02x%02x%02x%02x%02x%02x%u/%s", subsys,
			 addr->a.val[5], addr->a.val[4], addr->a.val[3],
			 addr->a.val[2], addr->a.val[1], addr->a.val[0],
			 addr->type, key);
	} else {
		snprintf(path, path_size,
			 "bt/%s/%02x%02x%02x%02x%02x%02x%u", subsys,
			 addr->a.val[5], addr->a.val[4], addr->a.val[3],
			 addr->a.val[2], addr->a.val[1], addr->a.val[0],
			 addr->type);
	}

	BT_DBG("Encoded path %s", path);
}

static void settings_dup_check_cb(char *name, char *val, void *cb_arg)
{
	struct settings_dup_check_arg *cdca = (struct settings_dup_check_arg *)
					      cb_arg;

	if (strcmp(name, cdca->name)) {
		return;
	}
	if (!val) {
		if (!cdca->val || cdca->val[0] == '\0') {
			cdca->is_dup = 1;
		} else {
			cdca->is_dup = 0;
		}
	} else {
		if (cdca->val && !strcmp(val, cdca->val)) {
			cdca->is_dup = 1;
		} else {
			cdca->is_dup = 0;
		}
	}
}

int settings_save_one(const char *name, char *value)
{
	struct settings_store *cs;
	struct settings_dup_check_arg cdca;

	cs = settings_save_dst;
	if (!cs) {
		return -ENOENT;
	}

	/*
	 * Check if we're writing the same value again.
	 */
	cdca.name = name;
	cdca.val = value;
	cdca.is_dup = 0;
	cs->cs_itf->csi_load(cs, settings_dup_check_cb, &cdca);
	if (cdca.is_dup == 1) {
		return 0;
	}
	return cs->cs_itf->csi_save(cs, name, value);
}

int bt_settings_decode_key(char *key, bt_addr_le_t *addr)
{
	bool high;
	int i;

	if (strlen(key) != 13) {
		return -EINVAL;
	}

	if (key[12] == '0') {
		addr->type = BT_ADDR_LE_PUBLIC;
	} else if (key[12] == '1') {
		addr->type = BT_ADDR_LE_RANDOM;
	} else {
		return -EINVAL;
	}

	for (i = 5, high = true; i >= 0; key++) {
		u8_t nibble;

		if (*key >= '0' && *key <= '9') {
			nibble = *key - '0';
		} else if (*key >= 'a' && *key <= 'f') {
			nibble = *key - 'a' + 10;
		} else {
			return -EINVAL;
		}

		if (high) {
			addr->a.val[i] = nibble << 4;
			high = false;
		} else {
			addr->a.val[i] |= nibble;
			high = true;
			i--;
		}
	}

	BT_DBG("Decoded %s as %s", key, bt_addr_le_str(addr));

	return 0;
}

int settings_bytes_from_str(char *val_str, void *vp, int *len)
{
#if 0
	int err;
	int rc;

	err = base64_decode(vp, *len, &rc, val_str, strlen(val_str));

	if (err) {
		return -1;
	}

	*len = rc;
#endif
	return 0;
}

int bt_settings_init(void)
{
#if 0
	int err;

	BT_DBG("");

	err = settings_subsys_init();
	if (err) {
		BT_ERR("settings_subsys_init failed (err %d)", err);
		return err;
	}

	err = settings_register(&bt_settings);
	if (err) {
		BT_ERR("settings_register failed (err %d)", err);
		return err;
	}
#endif
	return 0;
}
