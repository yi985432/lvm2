/*
 * Copyright (C) 2001 Sistina Software (UK) Limited.
 *
 * This file is released under the LGPL.
 *
 */

#include "log.h"
#include "locking.h"
#include "locking_types.h"
#include "lvm-string.h"
#include "activate.h"

#include <signal.h>

static struct locking_type _locking;
static sigset_t _oldset;

static int _lock_count = 0;	/* Number of locks held */
static int _signals_blocked = 0;

static void _block_signals(void)
{
	sigset_t set;

	if (_signals_blocked)
		return;

	if (sigfillset(&set)) {
		log_sys_error("sigfillset", "_block_signals");
		return;
	}

	if (sigprocmask(SIG_SETMASK, &set, &_oldset)) {
		log_sys_error("sigprocmask", "_block_signals");
		return;
	}

	_signals_blocked = 1;

	return;
}

static void _unblock_signals(void)
{
	/* Don't unblock signals while any locks are held */
	if (!_signals_blocked || _lock_count)
		return;

	if (sigprocmask(SIG_SETMASK, &_oldset, NULL)) {
		log_sys_error("sigprocmask", "_block_signals");
		return;
	}

	_signals_blocked = 0;

	return;
}

static inline void _update_lock_count(int flags)
{
	if ((flags & LCK_TYPE_MASK) == LCK_UNLOCK)
		_lock_count--;
	else
		_lock_count++;
}

/*
 * Select a locking type
 */
int init_locking(int type, struct config_file *cf)
{
	switch (type) {
	case 0:
		init_no_locking(&_locking, cf);
		log_print("WARNING: Locking disabled. Be careful! "
			  "This could corrupt your metadata.");
		return 1;

	case 1:
		if (!init_file_locking(&_locking, cf))
			break;
		log_very_verbose("File-based locking enabled.");
		return 1;

	case 2:
		if (!init_external_locking(&_locking, cf))
			break;
		log_very_verbose("External locking enabled.");
		return 1;

	default:
		log_error("Unknown locking type requested.");
		return 0;
	}

	if (!ignorelockingfailure())
		return 0;

	log_verbose("Locking disabled - only read operations permitted.");

	init_no_locking(&_locking, cf);

	return 1;
}

void fin_locking(void)
{
	_locking.fin_locking();
}

/*
 * VG locking is by VG name.
 * FIXME This should become VG uuid.
 */
int _lock_vol(struct cmd_context *cmd, const char *resource, int flags)
{
	_block_signals();

	if (!(_locking.lock_resource(cmd, resource, flags))) {
		_unblock_signals();
		return 0;
	}

	_update_lock_count(flags);
	_unblock_signals();

	return 1;
}

int lock_vol(struct cmd_context *cmd, const char *vol, int flags)
{
	char resource[258];

	switch (flags & LCK_SCOPE_MASK) {
	case LCK_VG:		/* Lock VG to change on-disk metadata. */
	case LCK_LV:		/* Suspends LV if it's active. */
		strncpy(resource, (char *) vol, sizeof(resource));
		break;
	default:
		log_error("Unrecognised lock scope: %d",
			  flags & LCK_SCOPE_MASK);
		return 0;
	}

	if (!_lock_vol(cmd, resource, flags))
		return 0;

	/* Perform immediate unlock unless LCK_HOLD set */
	if (!(flags & LCK_HOLD) && ((flags & LCK_TYPE_MASK) != LCK_UNLOCK)) {
		if (!_lock_vol(cmd, resource,
			       (flags & ~LCK_TYPE_MASK) | LCK_UNLOCK))
			return 0;
	}

	return 1;
}
