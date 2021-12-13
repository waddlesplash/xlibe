/* This file is part of BeXlib. */
#pragma once

#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
} LockInfoRec;

#undef _XLockMutex
#undef _XUnlockMutex
#undef _XCreateMutex
#undef _XFreeMutex

#define _XLockMutex(LOCK)	pthread_mutex_lock(&(LOCK)->mutex)
#define _XUnlockMutex(LOCK)	pthread_mutex_unlock(&(LOCK)->mutex)
#define _XCreateMutex(LOCK)	pthread_mutex_init(&(LOCK)->mutex, NULL);
#define _XFreeMutex(LOCK)	pthread_mutex_destroy(&(LOCK)->mutex);
