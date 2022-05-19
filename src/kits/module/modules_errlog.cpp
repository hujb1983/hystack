/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <core_config.h>

#include <logs/logs.h>
#include <logs/logs_config.h>
#include <logs/logs_thread.h>

#include <alloc/alloc_buffer.h>
#include <alloc/alloc_pool.h>

#include <thread/thread_object.h>
#include <thread/thread_task.h>

#include "modules.h"
#include "modules_conf.h"
#include "modules_config.h"

/* tasks_thread */
tag_log								logs_root;
std::shared_ptr<thread_task>		logs_thread;
std::shared_ptr<logs_print_task>	logs_thread_task;
std::atomic_bool					logs_thread_tasks_flag;
static const int					LOGS_INTERVAL = (1000L * 6L);

/* error level */
static tag_string error_level_string[] = {

	init_string("STDERR"),
	init_string("EMERG"),
	init_string("ALERT"),
	init_string("CRIT"),
	init_string("ERROR"),
	init_string("WARN"),
	init_string("NOTICE"),
	init_string("INFO"),
	init_string("TRACE"),
	init_string("DEBUG"),
	init_string_null
};

/* error_log */
static char * error_log(tag_conf *cf, tag_command *cmd, void *conf)
{
	tag_log		*new_log;

	new_log = (tag_log *)alloc_pool_ptr_null(cf->pool, sizeof(tag_log));
	new(new_log)(tag_log);

	new_log->level = logs::Error;

	new_log->ptr_handler = logs_handler_write_buffer;
	new_log->buffer = nullptr;

	new_log->ptr_writer = logs_handler_write_files;
	new_log->data = nullptr;

	tag_buffer * ptr = alloc_buffer_create(cf->pool, const_logs_page_size + 1);
	new_log->data = ptr;

	new_log->file_name = cf->args[1];
	new_log->file_path = cf->args[1];

	new_log->nickname.clear();

	if (cf->args.size() == 3) {

		for (auto c : cf->args[2]) {
			new_log->nickname += toupper(c);
		}

		for (int i = 0; i < 0xf; i++)
		{
			if (error_level_string[i].len == 0) {
				break;
			}

			if (strcmp((char *)error_level_string[i].data, new_log->nickname.data()) == 0) {
				new_log->level = i;
				break;
			}
		}
	}

	new_log->ptr_next = nullptr;

	tag_log *log = &logs_root;
	while (log->ptr_next != nullptr) {
		log = log->ptr_next;
	}

	new(&log->action)(std::string);
	log->ptr_next = new_log;

	return (char *)new_log;
}

/* errlog_commands */
static tag_command  errlog_commands[] = {

	{	init_string("error_log"),
		CMD_MAIN_CONF | CONF_1MORE,
		error_log,
		0,
		0,
		nullptr },

	init_command_null
};

/* errlog_module_ctx */
static tag_core_module  errlog_module_ctx = {

	init_string("errlog"),
	nullptr,
	nullptr
};

/* errlog_init_process */
static int errlog_init_process(tag_cycle *cycle) 
{
	logs_thread = std::make_shared<thread_task>(0);
	logs_thread_task = std::make_shared<logs_print_task>();

	logs_thread_tasks_flag = true;
	logs_thread->add_task(logs_thread_task);
	logs_thread->start();
	return RET_OK;
}

/* errlog_exit_process */
static void errlog_exit_process(tag_cycle *cycle) {
	logs_thread_tasks_flag = false;
}

/* errlog_module */
tag_module errlog_module = {

	MODULE_V1,
	&errlog_module_ctx,				/* module context */
	errlog_commands,				/* module directives */
	MODULE_CORE,					/* module type */
	nullptr,						/* init master */
	nullptr,						/* init module */
	errlog_init_process,			/* init process */
	nullptr,						/* init thread */
	nullptr,						/* exit thread */
	errlog_exit_process,			/* exit process */
	nullptr,						/* exit master */
	MODULE_V1_PADDING
};
