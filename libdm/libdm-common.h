/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef LIB_DMCOMMON_H
#define LIB_DMCOMMON_H

#include "libdevmapper.h"

#define DEV_NAME(dmt) (dmt->mangled_dev_name ? : dmt->dev_name)

int mangle_name(const char *str, size_t len, char *buf,
		size_t buf_len, dm_string_mangling_t mode);

int unmangle_name(const char *str, size_t len, char *buf,
		  size_t buf_len, dm_string_mangling_t mode);

struct target *create_target(uint64_t start,
			     uint64_t len,
			     const char *type, const char *params);

int add_dev_node(const char *dev_name, uint32_t minor, uint32_t major,
		 uid_t uid, gid_t gid, mode_t mode, int check_udev, unsigned rely_on_udev);
int rm_dev_node(const char *dev_name, int check_udev, unsigned rely_on_udev);
int rename_dev_node(const char *old_name, const char *new_name,
		    int check_udev, unsigned rely_on_udev);
int get_dev_node_read_ahead(const char *dev_name, uint32_t major, uint32_t minor,
			    uint32_t *read_ahead);
int set_dev_node_read_ahead(const char *dev_name, uint32_t major, uint32_t minor,
			    uint32_t read_ahead, uint32_t read_ahead_flags);
void update_devs(void);
void selinux_release(void);

void inc_suspended(void);
void dec_suspended(void);

#endif
