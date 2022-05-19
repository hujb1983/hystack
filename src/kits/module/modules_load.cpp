/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <memory>
#include <event/event_connection.h>

#include "modules_cycle.h"
#include "modules_event.h"
#include "modules_load.h"
#include "modules_conf.h"

extern tag_cycle	*root_cycle;
extern std::string	 process_signal;

extern tag_module	 core_module;
extern tag_module	 errlog_module;
extern tag_module	 event_module;
extern tag_module	 event_core_module;
extern tag_module	 select_module;
extern tag_module	 nodes_module;
extern tag_module	 nodes_core_module;


const char * modules_name[] = {
	"core_module",
	"errlog_module",
	"event_module",
	"event_core_module",
	"select_module",
	"nodes_module",
	"nodes_core_module",
	"nullptr",
};


tag_cycle * modules_load::add_to_module(tag_cycle *cycle, tag_module * m, char *name)
{
	cycle->modules.push_back((tag_module*)m);
	cycle->modules_size = cycle->modules.size();

	m->index = cycle->modules_size - 1;
	m->name = const_cast<char *> (name);
	return cycle;
}


tag_cycle * modules_load::add_to_cycle(tag_cycle *cycle)
{
	int  i = 0;

	cycle->modules.push_back((tag_module*)&core_module);
	cycle->modules.push_back((tag_module*)&errlog_module);
	cycle->modules.push_back((tag_module*)&event_module);
	cycle->modules.push_back((tag_module*)&event_core_module);
	cycle->modules.push_back((tag_module*)&select_module);
	cycle->modules.push_back((tag_module*)&nodes_module);
	cycle->modules.push_back((tag_module*)&nodes_core_module);

	cycle->modules_size = cycle->modules.size();
	for (auto m : cycle->modules)
	{
		m->index = i;
		m->name = const_cast<char *> (modules_name[i]);
		i++;
	}

	return nullptr;
}


tag_cycle * modules_load::add_to_conf(tag_cycle * old_cycle)
{
	char                *rc;
	void                *rv;
	tag_cycle           *cycle;
	tag_conf			 cf;
	tag_pool*			 pool;
	tag_core_module		*core_module;

	old_cycle->conn_size = 0;

	pool = alloc_pool_create(ALLOC_DEFAULT_POOL_SIZE);
	if (nullptr == pool) {
		return nullptr;
	}

	cycle = (tag_cycle*)alloc_pool_ptr_null(pool, sizeof(tag_cycle));
	if (cycle == nullptr) {
		alloc_pool_destory(pool);
		return nullptr;
	}

	cycle->pool = pool;
	cycle->old_cycle = old_cycle;
	cycle->modules_size = old_cycle->modules_size;

	new(&cycle->conn_free_queue)(queue<tag_connection*>);
	new(&cycle->listening)(vector<tag_listening>);
	new(&cycle->modules)(vector<tag_module*>);

	cycle->modules.clear();
	cycle->modules = old_cycle->modules;

	cycle->conf_ctx = (void****)alloc_pool_ptr_null(pool, cycle->modules_size * sizeof(void *));
	if (cycle->conf_ctx == nullptr) {
		alloc_pool_destory(pool);
		return nullptr;
	}

	for (auto m : cycle->modules)
	{
		if (m->type != MODULE_CORE) {
			continue;
		}

		core_module = static_cast<tag_core_module *> (m->context);
		if (core_module->create_conf != nullptr)
		{
			rv = core_module->create_conf(cycle);
			if (nullptr == rv) {
				alloc_pool_destory(pool);
				return nullptr;
			}
			cycle->conf_ctx[m->index] = static_cast<void ***> (rv);
		}
	}

	cf.cycle = cycle;

	new(&cycle->prefix)(string);
	new(&cycle->prefix_conf)(string);
	new(&cycle->prefix_logs)(string);
	new(&cycle->conf_file)(string);

	cycle->prefix = old_cycle->prefix;
	cycle->prefix_conf = old_cycle->prefix_conf;
	cycle->prefix_logs = old_cycle->prefix_logs;

	cycle->conf_file = cycle->prefix;
	cycle->conf_file += "/conf/server.conf";
	cf.conf_file = cycle->conf_file;

	cf.pool = pool;
	cf.context = (void *)cycle->conf_ctx;
	cf.module_type = MODULE_CORE;
	cf.cmd_type = MODULE_CORE;

	rc = modules_parse::include_file(&cf);
	if (rc == nullptr) {
		alloc_pool_destory(pool);
		return nullptr;
	}

	for (auto m : cycle->modules)
	{
		if (m->type != MODULE_CORE) {
			continue;
		}

		core_module = static_cast<tag_core_module *> (m->context);
		if (core_module->init_conf != nullptr)
		{
			rv = core_module->init_conf(cycle, cycle->conf_ctx[m->index]);
			if (rv == nullptr) {
				alloc_pool_destory(pool);
				return nullptr;
			}
		}
	}

	if (strcmp(process_signal.data(), "") == 0)
	{
		int ret = event_listening::open(cycle);
		if (ret == -1) {
			alloc_pool_destory(pool);
			return nullptr;
		}
	}

	/* events_listening::open */
	int ret = event_listening::open(cycle);
	if (ret == -1) {
		alloc_pool_destory(pool);
		return nullptr;
	}

	root_cycle = cycle;
	return cycle;
}