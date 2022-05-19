/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once
#include <string>
#include <mutex>
#include <atomic>

#include <thread/thread_object.h>
#include "logs_config.h"


 /* logs_print_task */
class logs_print_task : public thread_object
{
public:
	logs_print_task();
	virtual ~logs_print_task();

public:
	virtual void run();
	virtual void posted(tag_log *log);
};
