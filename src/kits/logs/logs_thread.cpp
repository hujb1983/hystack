/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
using namespace std;
using namespace chrono;

#include <alloc/alloc_buffer.h>
#include <alloc/alloc_pool.h>

#include "logs.h"
#include "logs_config.h"
#include "logs_stream.h"
#include "logs_thread.h"

extern tag_log				logs_root;
extern std::atomic_bool		logs_thread_tasks_flag;
static const int			LOGS_INTERVAL = (1000L * 6L);

logs_print_task::logs_print_task()
{

}

logs_print_task::~logs_print_task()
{

}

void logs_print_task::run()
{
	tag_log	*log = &logs_root;

	while (logs_thread_tasks_flag)
	{
		log = &logs_root;

		while (log)
		{
			{
				std::lock_guard<std::mutex> lock(log->mutex);

				tag_buffer *data = (tag_buffer*)log->data;
				if (data == nullptr) {
					goto next_log;
				}

				log->file_size = logs_stream(data->ptr_pos, data->ptr_last).write(log->file_path);
				data->ptr_last = data->ptr_start;
			}

		next_log:
			log = log->ptr_next;
		}

		this_thread::sleep_for(chrono::milliseconds(LOGS_INTERVAL));
	};
}

void logs_print_task::posted(tag_log *log)
{

}

