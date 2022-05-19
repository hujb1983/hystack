/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */

#include "thread_collection.h"
#include <iostream>


/* thread_collection::thread_collection */
thread_collection::thread_collection(const uint8_t& nsize) {
	create_thread(nsize);
}

/* thread_collection::thread_collection */
thread_collection::~thread_collection()
{
	for (auto& info : threads)
	{
		info->set_interupt();
		info->notify();
	}

	for (auto& info : threads)
	{
		info->join();
	}
}

/* thread_collection::create_thread */
void thread_collection::create_thread(const uint8_t& nsize)
{
	for (auto i = 0; i < nsize; i++)
	{
		auto thread = std::make_shared<thread_task>(i);
		threads.emplace_back(std::move(thread));
	}

	for (auto i = 0; i < nsize; i++)
	{
		threads[i]->start();
	}
}

/* thread_collection::dispatch */
void thread_collection::dispatch(const uint64_t& index, const thread_operation& fn)
{
	auto hashcode = static_cast<uint8_t>(index % threads.size());
	threads[hashcode]->add_task(fn);
}

/* thread_collection::dispatch */
void thread_collection::dispatch(const uint64_t& index, const thread_ptr& fn)
{
	auto hashcode = static_cast<uint8_t>(index % threads.size());
	threads[hashcode]->add_task(fn);
}
