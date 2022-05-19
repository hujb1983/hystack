/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */
#pragma once

#include <vector>
#include <stdint.h>
#include "thread_object.h"
#include "thread_task.h"

using thread_task_ptr = std::shared_ptr<thread_task>;

/* thread_collection */
class thread_collection
{
private:
	thread_collection() = delete;
	thread_collection(thread_collection &pool) = delete;
	thread_collection operator = (const thread_collection&) = delete;

public:
	thread_collection(const uint8_t& nsize);
	~thread_collection();

private:
	void create_thread(const uint8_t& nsize);

public:
	void dispatch(const uint64_t& index, const thread_operation& fn);
	void dispatch(const uint64_t& index, const thread_ptr& fn);

private:
	std::vector<thread_task_ptr>  threads;
};
