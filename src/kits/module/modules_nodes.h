/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#pragma once

#include "modules.h"
#include "modules_conf.h"
#include "modules_config.h"

#define NODES_MODULE			0x3A1E5635  /* "NODE" */
#define NODES_CONF				0x02000000

typedef struct ST_NODES_CORE	tag_nodes_core;
typedef struct ST_NODES_CONF	tag_nodes_conf;

extern tag_module  nodes_module;
extern tag_module  nodes_core_module;

/* tag_nodes_core */
struct ST_NODES_CORE
{
	std::string		 prefix;
	std::string		 conf;
	std::string		 sbin;
	std::string		 cache;
	std::string		 cert;
	std::string		 html;
	std::string		 logs;
	std::string		 lua;
};

/* tag_nodes_conf */
struct ST_NODES_CONF
{
	void		   **main_conf;
	tag_pool		*pool;

	std::string		 auth_api;
	std::string		 stat_api;
	std::string		 logs_api;
	std::string		 sync_api;
	std::string		 accounts;
	std::string		 password;
	std::string		 secret;
};

/* tag_nodes_module */
typedef struct
{
	tag_string		 name;
	void			*(*create_conf)(tag_conf *cf);
	void			*(*init_conf)(tag_conf *cf, void *conf);
} tag_nodes_module;


#define nodes_get_conf(conf_ctx, module)									\
		(*(modules_get_conf(conf_ctx, nodes_module))) [module.ctx_index]

