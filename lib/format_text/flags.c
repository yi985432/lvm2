/*
 * Copyright (C) 2001 Sistina Software (UK) Limited.
 *
 * This file is released under the LGPL.
 */

#include "log.h"
#include "metadata.h"
#include "import-export.h"
#include "lvm-string.h"

/*
 * Bitsets held in the 'status' flags get
 * converted into arrays of strings.
 */
struct flag {
	int mask;
	char *description;
};

static struct flag _vg_flags[] = {
	{ACTIVE, "ACTIVE"},
	{EXPORTED_VG, "EXPORTED"},
	{RESIZEABLE_VG, "RESIZEABLE"},
	{CLUSTERED, "CLUSTERED"},
	{SHARED, "SHARED"},
	{0, NULL}
};

static struct flag _pv_flags[] = {
	{ACTIVE, "ACTIVE"},
	{ALLOCATABLE_PV, "ALLOCATABLE"},
	{0, NULL}
};

static struct flag _lv_flags[] = {
	{ACTIVE, "ACTIVE"},
	{LVM_READ, "READ"},
	{LVM_WRITE, "WRITE"},
	{ALLOC_SIMPLE, "ALLOC_SIMPLE"},
	{ALLOC_STRICT, "ALLOC_STRICT"},
	{ALLOC_CONTIGUOUS, "ALLOC_CONTIGUOUS"},
	{SNAPSHOT, "SNASHOT"},
	{SNAPSHOT_ORG, "SNAPSHOT_ORIGIN"},
	{0, NULL}
};

static struct flag *_get_flags(int type)
{
	switch (type) {
	case VG_FLAGS:
		return _vg_flags;

	case PV_FLAGS:
		return _pv_flags;

	case LV_FLAGS:
		return _lv_flags;
	}

	log_err("Unknown flag set requested.");
	return NULL;
}

static int _emit(char **buffer, size_t *size, const char *fmt, ...)
{
	size_t n;
	va_list ap;

	va_start(ap, fmt);
	n = vsnprintf(*buffer, *size, fmt, ap);
	va_end(ap);

	if (n < 0 || (n == *size))
		return 0;

	*buffer += n;
	*size -= n;
	return 1;
}

/*
 * Converts a bitset to an array of string values,
 * using one of the tables defined at the top of
 * the file.
 */
int print_flags(uint32_t status, int type, char *buffer, size_t size)
{
	int f, first = 1;
	struct flag *flags;

	if (!(flags = _get_flags(type))) {
		stack;
		return 0;
	}

	if (!_emit(&buffer, &size, "["))
		return 0;

	for (f = 0; flags[f].mask; f++) {
		if (status & flags[f].mask) {
			if (!first) {
				if (!_emit(&buffer, &size, ", "))
					return 0;

			} else
				first = 0;

			if (!_emit(&buffer, &size, "\"%s\"",
				   flags[f].description))
				return 0;

			status &= ~flags[f].mask;
		}
	}

	if (!_emit(&buffer, &size, "]"))
		return 0;

	if (status)
		/* FIXME: agk is this the correct log level ? */
		log_print("Not all flags were successfully exported, "
			  "possible bug.");

	return 1;
}

int read_flags(uint32_t *status, int type, struct config_value *cv)
{
	int f;
	uint32_t s = 0;
	struct flag *flags;

	if (!(flags = _get_flags(type))) {
		stack;
		return 0;
	}

	while (cv) {
		if (cv != CFG_STRING) {
			log_err("Status value is not a string.");
			return 0;
		}

		for (f = 0; flags[f].description; f++)
			if (!strcmp(flags[f].description, cv->v.str)) {
				(*status) &= flags[f].mask;
				break;
			}

		if (!flags[f].description) {
			log_err("Unknown status flag.");
			return 0;
		}

		cv = cv->next;
	}

	*status = s;
	return 1;
}
