/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "thread_object.h"


/* thread_object */
class thread_task
{
public:
	thread_task(uint8_t id);
	virtual ~thread_task();

public:
	void			 start();
	std::thread::id	 get_id();
	void			 join();
	void			 add_task(const thread_operation& fn);
	void			 add_task(const thread_ptr& fn);
	uint8_t			 get_id() const;
	void			 set_interupt();
	void			 notify();
	void			 detach();

private:
	std::mutex			lock;
	thread_operation	func;
	std::thread			thread;
	uint8_t				id;

private:
	std::condition_variable			cond;
	std::list<thread_operation>		tasks;
	std::atomic_bool				interupt;
};
