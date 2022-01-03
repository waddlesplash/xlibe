/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

struct _XLockInfo {
	pthread_mutex_t mutex;
};
typedef struct _XLockInfo LockInfoRec;

#undef _XLockMutex
#undef _XUnlockMutex
#undef _XCreateMutex
#undef _XFreeMutex

static inline void
_XCreateMutex(struct _XLockInfo* info)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&info->mutex, &attr);
}

#define _XLockMutex(LOCK)	pthread_mutex_lock(&(LOCK)->mutex)
#define _XUnlockMutex(LOCK)	pthread_mutex_unlock(&(LOCK)->mutex)
#define _XFreeMutex(LOCK)	pthread_mutex_destroy(&(LOCK)->mutex);

#ifdef __cplusplus
} // extern "C"
#endif
