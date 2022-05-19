/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#if (WINX)
#include <windows.h>
#include <stdio.h>
#else
#include <pthread.h>
#include <stdio.h>
#endif

#include "circuit_mutex.h"

#if (!WINX)
static bool attr_initalized = false;
static pthread_mutexattr_t attr;
#endif

/* circuit_mutex */
circuit_mutex::circuit_mutex()
{
#if (WINX)
	InitializeCriticalSection(&mutex_);
#else
	if (!attr_initalized)
	{
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
		attr_initalized = true;
	}

	pthread_mutex_init(&mutex_, &attr);
#endif
}

circuit_mutex::~circuit_mutex()
{
#if (WINX)
	DeleteCriticalSection(&mutex_);
#else
	pthread_mutex_destroy(&mutex_);
#endif
}

void circuit_mutex::lock()
{
#if (WINX)
	EnterCriticalSection(&mutex_);
#else
	pthread_mutex_lock(&mutex_);
#endif
}

void circuit_mutex::unlock()
{
#if (WINX)
	LeaveCriticalSection(&mutex_);
#else
	pthread_mutex_unlock(&mutex_);
#endif
}

bool circuit_mutex::try_lock()
{
#if (!WINX)
	return (pthread_mutex_trylock(&mutex_) == 0);
#endif
	return 0;
}


/*	circuit_mutex_guard */
circuit_mutex_guard::circuit_mutex_guard(circuit_mutex& mtx) : mutex_(mtx) {
	mutex_.lock();
}

circuit_mutex_guard::~circuit_mutex_guard() {
	mutex_.unlock();
}
