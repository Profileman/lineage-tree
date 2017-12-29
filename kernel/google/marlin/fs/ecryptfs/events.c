/**
 * eCryptfs: Linux filesystem encryption layer
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/string.h>
#include <linux/ecryptfs.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/random.h>
#include "ecryptfs_kernel.h"

static DEFINE_MUTEX(events_mutex);
struct ecryptfs_events *events_ptr = NULL;

void ecryptfs_free_events(void)
{
	mutex_lock(&events_mutex);
	if (events_ptr != NULL) {
		kfree(events_ptr);
		events_ptr = NULL;
	}

	mutex_unlock(&events_mutex);
}

/**
 * Given ecryptfs data, the function
 * returns appropriate key size.
 */
size_t ecryptfs_get_key_size(const void *data)
{

	struct ecryptfs_crypt_stat *stat = NULL;

	if (!data)
		return 0;

	stat = (struct ecryptfs_crypt_stat *)data;
	return stat->key_size;
}

/**
 * Given ecryptfs data, the function
 * returns appropriate salt size.
 *
 * !!! crypt_stat cipher name and mode must be initialized
 */
size_t ecryptfs_get_salt_size(const void *data)
{
	if (!data) {
		ecryptfs_printk(KERN_ERR,
				"ecryptfs_get_salt_size: invalid data parameter\n");
		return 0;
	}

	return ecryptfs_get_salt_size_for_cipher(data);

}

/**
 * Given ecryptfs data, the function
 * returns file encryption key.
 */
const unsigned char *ecryptfs_get_key(const void *data)
{

	struct ecryptfs_crypt_stat *stat = NULL;

	if (!data) {
		ecryptfs_printk(KERN_ERR,
			"ecryptfs_get_key: invalid data parameter\n");
		return NULL;
	}
	stat = (struct ecryptfs_crypt_stat *)data;
	return stat->key;
}

/**
 * Given ecryptfs data, the function
 * returns file encryption salt.
 */
const unsigned char *ecryptfs_get_salt(const void *data)
{
	struct ecryptfs_crypt_stat *stat = NULL;

	if (!data) {
		ecryptfs_printk(KERN_ERR,
			"ecryptfs_get_salt: invalid data parameter\n");
		return NULL;
	}
	stat = (struct ecryptfs_crypt_stat *)data;
	return stat->key + ecryptfs_get_salt_size(data);
}

/**
 * Returns ecryptfs events pointer
 */
inline struct ecryptfs_events *get_events(void)
{
	return events_ptr;
}

/**
 * If external crypto module requires salt in addition to key,
 * we store it as part of key array (if there is enough space)
 * Checks whether a salt key can fit into array allocated for
 * regular key
 */
bool ecryptfs_check_space_for_salt(const size_t key_size,
		const size_t salt_size)
{
	if ((salt_size + key_size) > ECRYPTFS_MAX_KEY_BYTES)
		return false;

	return true;
}

/*
 * If there is salt that is used by external crypto module, it is stored
 * in the same array where regular key is. Salt is going to be used by
 * external crypto module only, so for all internal crypto operations salt
 * should be ignored.
 *
 * Get key size in cases where it is going to be used for data encryption
 * or for all other general purposes
 */
size_t ecryptfs_get_key_size_to_enc_data(
		const struct ecryptfs_crypt_stat *crypt_stat)
{
	if (!crypt_stat)
		return 0;

	return crypt_stat->key_size;
}

/*
 * If there is salt that is used by external crypto module, it is stored
 * in the same array where regular key is. Salt is going to be used by
 * external crypto module only, but we still need to save and restore it
 * (in encrypted form) as part of ecryptfs header along with the regular
 * key.
 *
 * Get key size in cases where it is going to be stored persistently
 *
 * !!! crypt_stat cipher name and mode must be initialized
 */
size_t ecryptfs_get_key_size_to_store_key(
		const struct ecryptfs_crypt_stat *crypt_stat)
{
	size_t salt_size = 0;

	if (!crypt_stat)
		return 0;

	salt_size = ecryptfs_get_salt_size(crypt_stat);

	if (!ecryptfs_check_space_for_salt(crypt_stat->key_size, salt_size)) {
		ecryptfs_printk(KERN_WARNING,
			"ecryptfs_get_key_size_to_store_key: not enough space for salt\n");
		return crypt_stat->key_size;
	}

	return crypt_stat->key_size + salt_size;
}

/*
 * If there is salt that is used by external crypto module, it is stored
 * in the same array where regular key is. Salt is going to be used by
 * external crypto module only, but we still need to save and restore it
 * (in encrypted form) as part of ecryptfs header along with the regular
 * key.
 *
 * Get key size in cases where it is going to be restored from storage
 *
 * !!! crypt_stat cipher name and mode must be initialized
 */
size_t ecryptfs_get_key_size_to_restore_key(size_t stored_key_size,
		const struct ecryptfs_crypt_stat *crypt_stat)
{
	size_t salt_size = 0;

	if (!crypt_stat)
		return 0;

	salt_size = ecryptfs_get_salt_size_for_cipher(crypt_stat);

	if (salt_size >= stored_key_size) {
		ecryptfs_printk(KERN_WARNING,
			"ecryptfs_get_key_size_to_restore_key: salt %zu >= stred size %zu\n",
			salt_size, stored_key_size);

		return stored_key_size;
	}

	return stored_key_size - salt_size;
}

/**
 * Given crypt_stat, the function returns appropriate salt size.
 */
size_t ecryptfs_get_salt_size_for_cipher(
		const struct ecryptfs_crypt_stat *crypt_stat)
{
	if (!get_events() || !(get_events()->get_salt_key_size_cb))
		return 0;

	return get_events()->get_salt_key_size_cb(crypt_stat);
}

/**
 * Given mount_crypt_stat, the function returns appropriate salt size.
 */
size_t ecryptfs_get_salt_size_for_cipher_mount(
		const struct ecryptfs_mount_crypt_stat *crypt_stat)
{
	if (!get_events() || !(get_events()->get_salt_key_size_cb))
		return 0;

	return get_events()->get_salt_key_size_cb(crypt_stat);
}

