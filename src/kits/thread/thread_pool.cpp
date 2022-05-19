/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */
#include "thread_pool.h"

/* thread_pool::thread_pool */
thread_pool::thread_pool(unsigned char nsize)
{
	idlethreadcount = nsize;
	stop.store(false);

	for (auto i = 0; i < nsize; i++) {
		create_thread();
	}
}

/* thread_pool::~thread_pool */
thread_pool::~thread_pool()
{
	stop.store(true);
	cond.notify_all();

	for (auto & info : threads)
	{
		if (info.joinable()) {

			info.join();
		}
	}
}

/* thread_pool::create_thread */
void thread_pool::create_thread() {

	threads.emplace_back(&thread_pool::thread_work, this);
}

/* thread_pool::thread_work */
void thread_pool::thread_work()
{
	while (stop.load() != true) {

		std::unique_lock<std::mutex> lock_(lock);

		while (tasks.empty()) {
			if (cond.wait_for(lock_, std::chrono::microseconds(500)) == std::cv_status::timeout)
			{
				break;
			}
		}

		if (!tasks.empty())
		{
			auto task = tasks.front();
			tasks.pop_front();
			lock_.unlock();
			idlethreadcount--;
			handle(std::move(task));
			idlethreadcount++;
		}
	}
}

/* thread_pool::handle */
void thread_pool::handle(thread_operation task)
{
	if (task != nullptr) {
		task();
	}
}


/* thread_pool::add_task */
void thread_pool::add_task(thread_ptr task)
{
	if (task == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock_(lock);
	tasks.emplace_back(std::bind(&thread_object::run, task));
	cond.notify_one();
}


/* thread_pool::add_task */
void thread_pool::add_task(thread_operation task)
{
	if (task == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock_(lock);
	tasks.emplace_back(task);
	cond.notify_one();
}

/* thread_pool::get_thread_count */
unsigned char thread_pool::get_thread_count() const {
	return static_cast<unsigned char>(threads.size());
}

/* thread_pool::get_idle_thread_count */
unsigned char thread_pool::get_idle_thread_count() const {
	return idlethreadcount;
}
