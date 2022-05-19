/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once
#include <list>
#include <mutex>
#include <thread>
#include <atomic>
#include "thread_task.h"


/* thread_pool */
class thread_pool
{
public:
	thread_pool() = delete;
	thread_pool(thread_pool& pool) = delete;
	thread_pool operator = (const thread_pool&) = delete;

public:
	void add_task(thread_ptr param);
	void add_task(thread_operation task);

	unsigned char get_thread_count() const;
	unsigned char get_idle_thread_count() const;

public:
	thread_pool(unsigned char nsize);
	~thread_pool();

private:
	void create_thread();
	void thread_work();
	void handle(thread_operation param);
	
private:
	unsigned char				idlethreadcount;
	std::list<std::thread>		threads;
	std::atomic_bool			stop;
	std::condition_variable		cond;
	std::mutex					lock;
	std::list<thread_operation>	tasks;
};
