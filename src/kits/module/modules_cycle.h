/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once

#include <string>
#include <vector>
#include <queue>
using namespace std;

#include <core_config.h>
#include <alloc/alloc_pool.h>

/* tag_cycle */
struct ST_CYCLE
{
	void						 ****conf_ctx;

	tag_log							*log;
	tag_pool						*pool;
	tag_pool						*pool_temp;

	tag_event						*read_event;
	tag_event						*write_event;

	std::string						 prefix;
	std::string						 prefix_conf;
	std::string						 prefix_logs;

	tag_cycle						*old_cycle;
	unsigned char					 modules_used : 1;

	std::string						 conf_file;
	std::string						 conf_param;
	std::string						 conf_path;

	std::vector<tag_module*>		 modules;
	unsigned int					 modules_size;

	std::vector<tag_listening*>		 listening;

	unsigned int					 conn_size;
	tag_connection					*conn_ptr;
	std::queue<tag_connection*>		 conn_free_queue;
};
