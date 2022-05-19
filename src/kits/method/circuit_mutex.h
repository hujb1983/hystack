/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once

#if (!WINX)
#include <pthread.h>
#endif


 /* circuit_mutex */
class  circuit_mutex
{
public:
	circuit_mutex();
	virtual ~circuit_mutex();

public:
	void lock();
	void unlock();
	bool try_lock();

protected:

#if (WINX)
	CRITICAL_SECTION	mutex_;
#else
	pthread_mutex_t		mutex_;
#endif
};


/*	circuit_mutex_guard */
class circuit_mutex_guard
{
public:
	circuit_mutex_guard(circuit_mutex &mtx);
	virtual ~circuit_mutex_guard();

protected:
	circuit_mutex &mutex_;
};
