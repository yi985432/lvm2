/*
 * device-mapper.h
 *
 * Copyright (C) 2001 Sistina Software
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Changelog
 *
 *     14/08/2001 - First version [Joe Thornber]
 */

#ifndef DEVICE_MAPPER_H
#define DEVICE_MAPPER_H

#include <linux/major.h>

/* FIXME: Use value from local range for now, for co-existence with LVM 1 */
#define DM_BLK_MAJOR 124

struct dm_table;
struct text_region;
typedef unsigned int offset_t;

typedef void (*dm_error_fn)(const char *message, void *private);

/* constructor, destructor and map fn types */
typedef void *(*dm_ctr_fn)(struct dm_table *t,
			 offset_t b, offset_t l,
			 struct text_region *args);

typedef void (*dm_dtr_fn)(struct dm_table *t, void *c);
typedef int (*dm_map_fn)(struct buffer_head *bh, int rw, void *context);
typedef int (*dm_err_fn)(struct buffer_head *bh, int rw, void *context);

/*
 * information about a target type
 */
struct target_type {
        struct list_head list;
        const char *name;
        long use;
        struct module *module;
        dm_ctr_fn ctr;
        dm_dtr_fn dtr;
        dm_map_fn map;
	dm_err_fn err;
	unsigned long flags;
};

#define TF_BMAP	0x0001	/* Target supports bmap operation */

int dm_register_target(struct target_type *t);
int dm_unregister_target(struct target_type *t);

struct block_device *dm_blkdev_get(const char *path);
int dm_blkdev_put(struct block_device *);

struct text_region {
        const char *b;
        const char *e;
};

/*
 * These may be useful for people writing target
 * types.
 */
int dm_get_number(struct text_region *txt, unsigned int *n);
int dm_get_line(struct text_region *txt, struct text_region *line);
int dm_get_word(struct text_region *txt, struct text_region *word);
void dm_txt_copy(char *dest, size_t max, struct text_region *txt);
void dm_eat_space(struct text_region *txt);

#endif /* DEVICE_MAPPER_H */

/*
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
