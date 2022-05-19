/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once
#include <functional>
#include <memory>

class thread_object
{
public:
	virtual ~thread_object() {};
	virtual void run() = 0;
};

using thread_operation = std::function<void()>;
using thread_ptr = std::shared_ptr<thread_object>;
