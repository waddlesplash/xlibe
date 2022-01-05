/*
 * Copyright 2022, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <pthread.h>
#include <private/shared/AutoLocker.h>

class PthreadRWLockReadLocking {
public:
	inline bool Lock(pthread_rwlock_t* lock)
	{
		return pthread_rwlock_rdlock(lock) == 0;
	}

	inline void Unlock(pthread_rwlock_t* lock)
	{
		pthread_rwlock_unlock(lock);
	}
};

class PthreadRWLockWriteLocking {
public:
	inline bool Lock(pthread_rwlock_t* lock)
	{
		return pthread_rwlock_wrlock(lock) == 0;
	}

	inline void Unlock(pthread_rwlock_t* lock)
	{
		pthread_rwlock_unlock(lock);
	}
};

typedef AutoLocker<pthread_rwlock_t, PthreadRWLockReadLocking> PthreadReadLocker;
typedef AutoLocker<pthread_rwlock_t, PthreadRWLockWriteLocking> PthreadWriteLocker;
