/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */

#include <iostream>
#include "thread_task.h"


thread_task::thread_task(uint8_t _id)
{
	id = _id;
	interupt.store(false);
}

thread_task::~thread_task()
{
	if (thread.joinable())
	{
		thread.join();
	}
}

uint8_t thread_task::get_id() const {
	return id;
}

void thread_task::start()
{
	auto func = [this]() {

		while (interupt.load() == false)
		{
			std::unique_lock<std::mutex> lock_(lock);

			cond.wait(lock_, [this] {return interupt || !tasks.empty(); });

			if (!tasks.empty())
			{
				auto task = tasks.front();
				tasks.pop_front();
				lock_.unlock();
				task();
			}
		}
	};

	std::thread th(func);
	thread = std::move(th);
}

std::thread::id thread_task::get_id()
{
	return std::this_thread::get_id();
}

void thread_task::join()
{
	if (thread.joinable()) {
		thread.join();
	}
}

void thread_task::add_task(const thread_ptr& task)
{
	if (task == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock_(lock);
	tasks.emplace_back(std::bind(&thread_object::run, task));
	cond.notify_one();
}

void thread_task::add_task(const thread_operation& task)
{
	if (task == nullptr) {
		return;
	}

	std::lock_guard<std::mutex> lock_(lock);
	tasks.emplace_back(task);
	cond.notify_one();
}

void thread_task::set_interupt() {
	interupt.store(true);
}

void thread_task::notify() {
	cond.notify_one();
}

void thread_task::detach() {
	thread.detach();
}
