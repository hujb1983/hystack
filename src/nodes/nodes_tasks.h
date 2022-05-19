/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#pragma once
#include <core_config.h>

 /* nodes_task_nginx_process */
int nodes_task_dispatch(tag_nodes_task * task);

/* nodes_task_nginx_process */
int nodes_task_nginx_process(tag_nodes_task * task);

/* nodes_task_cache_process */
int nodes_task_cache_process(tag_nodes_task * task);

/* nodes_task_cache_process */
int nodes_task_redis_process(tag_nodes_task * task);

/* nodes_task_ssl_process */
int nodes_task_ssl_process(tag_nodes_task * task);

/* nodes_task_lua_process */
int nodes_task_lua_process(tag_nodes_task * task);

/* nodes_task_step_process */
int nodes_task_step_process(tag_nodes_task * task);

/* nodes_task_errpage_process */
int nodes_task_errpage_process(tag_nodes_task * task);

/* nodes_task_http_process */
int nodes_task_http_process(tag_nodes_task * task);

/* nodes_task_defense_process */
int nodes_task_defense_process(tag_nodes_task * task);

/* tag_nodes_task_backup */
typedef struct
{
	unsigned int    code;
	std::string		request;

	std::string		bak;
	std::string		file;

} tag_nodes_task_backup;

/* nodes_task_backup */
class nodes_task_backup
{
public:
	nodes_task_backup();
	virtual ~nodes_task_backup();

	std::vector<tag_nodes_task_backup>  baks;

public:
	int add_task(tag_nodes_task_backup * backup);
	int roll_back();
};
