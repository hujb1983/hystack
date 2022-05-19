/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <memory>
#include <string>

#include "modules.h"
#include "modules_conf.h"
#include "modules_cycle.h"
#include "modules_config.h"
#include "modules_nodes.h"
#include "process/process.h"
#include "process/process_mgr.h"

#include <nodes.h>
#include <nodes_cycle.h>
#include <nodes_cmdline.h>

#define	nodes_init_conf_(a, b) { if (a.size() == 0) { a = b;}}
unsigned int nodes_max_module;

/* ptr_nodes_cycle */
std::shared_ptr<nodes_cycle>    ptr_nodes_cycle;

/* nodes_block */
static char * nodes_block(tag_conf *cf, tag_command *cmd, void * ptr)
{
	// u_int			 mi;
	char				*rc;
	void				*rv;
	tag_nodes_module	*core_module;
	tag_conf			 pcf;
	void			  ***ctx;

	nodes_max_module = modules_conf::count(cf->cycle, NODES_MODULE);

	ctx = (void***)alloc_pool_ptr_null(cf->pool, sizeof(void *));
	if (ctx == nullptr) {
		return nullptr;
	}

	*ctx = (void**)alloc_pool_ptr_null(cf->pool, nodes_max_module * sizeof(void *));
	if (*ctx == nullptr) {
		return nullptr;
	}

	*(void **)ptr = ctx;

	for (auto m : cf->cycle->modules)
	{
		if (m->type != NODES_MODULE) {
			continue;
		}

		core_module = static_cast<tag_nodes_module *> (m->context);
		// mi = m->ctx_index;

		if (core_module->create_conf != nullptr)
		{
			rv = core_module->create_conf(cf);
			if (nullptr == rv) {
				return nullptr;
			}

			(*ctx)[m->ctx_index] = static_cast<void *> (rv);
		}
	}

	pcf.context = cf->context;
	pcf.module_type = cf->module_type;
	pcf.cmd_type = cf->cmd_type;

	cf->context = ctx;
	cf->module_type = NODES_MODULE;
	cf->cmd_type = NODES_CONF;

	rc = modules_parse::include_file(cf);
	if (rc == nullptr) {
		return nullptr;
	}

	cf->context = pcf.context;
	cf->module_type = pcf.module_type;
	cf->cmd_type = pcf.cmd_type;

	for (auto m : cf->cycle->modules)
	{
		if (m->type != NODES_MODULE) {
			continue;
		}

		core_module = static_cast<tag_nodes_module *> (m->context);
		// mi = m->ctx_index;

		if (core_module->init_conf != nullptr)
		{
			rv = core_module->init_conf(cf, (*ctx)[m->ctx_index]);
			if (nullptr == rv) {
				return nullptr;
			}
		}
	}

	/* exception */
	nodes_exception_init(MODULE_PREFIX);

	/* create a nodes_cycle for core_solution */
	ptr_nodes_cycle = std::make_shared<nodes_cycle>();
	process_mgr::set_current_cycle(ptr_nodes_cycle.get()); 

	top_process_option_cmdline = nodes_test_commandline;
	return (char *)ctx;
}

/* nodes_commands */
static tag_command nodes_commands[] = {

	{	init_string("nodes"),
		CMD_MAIN_CONF | CONF_BLOCK | CONF_NOARGS,
		nodes_block,
		0,
		0,
		nullptr
	},

	init_command_null
};

/* nodes_init_conf */
void * nodes_init_conf(tag_conf *cf, void *conf)
{
	return conf;
}

/* nodes_module_ctx */
tag_nodes_module nodes_module_ctx = {
	init_string("nodes"),
	nullptr,
	nodes_init_conf
};

/* tag_accounts */
struct tag_nodes_accounts
{
	tag_nodes_accounts()
	{
		auth_api = "http://dev.aosings.top/api/product-auth-service/tokenByCdnNode";
		stat_api = "ws://dev.aosings.top/api/product-node-log-service/ws";
		logs_api = "ws://dev.aosings.top/api/product-node-log-service/log_ws";
		sync_api = "ws://dev.aosings.top/api/product-sync-service/ws";
		task_api = "ws://dev.aosings.top/api/product-sync-service/ws";
		pull_api = "ws://dev.aosings.top/api/product-sync-service/ws";
		back_api = "ws://dev.aosings.top/api/product-sync-service/ws";
		accounts = "node01";
		password = "asy@123456";
		secret = "RyQFL4UsGf9RFxj9tDIA1jdBaRDxyod2r";
	}

	std::string		auth_api;
	std::string		stat_api;
	std::string		logs_api;
	std::string		sync_api;
	std::string		task_api;
	std::string		pull_api;
	std::string		back_api;
	std::string		accounts;
	std::string		password;
	std::string		secret;
};

/* nodes_init_process */
int nodes_init_process(tag_cycle *cycle)
{
	/*
	void			  ***cf;
	tag_nodes_core		*ncf;

	cf = modules_get_conf(cycle->conf_ctx, nodes_module);
	ncf = (tag_nodes_core*)(*cf)[nodes_core_module.ctx_index];

	std::shared_ptr<nodes_accounts> accounts;
	accounts = std::make_shared<nodes_accounts>();

	std::string node_path;
	node_path = ncf->conf + "/node.json";
	accounts->read_accounts(node_path);

	tag_nodes_accounts acct;
	if (accounts->accounts_handler(acct) == -1) {
		return -1;
	} */

	return 0;
}


/* nodes_module */
tag_module nodes_module = {
	MODULE_V1,
	&nodes_module_ctx,						  /* module context */
	nodes_commands,							  /* module directives */
	MODULE_CORE,							  /* module type */
	nullptr,                                  /* init master */
	nullptr,                                  /* init module */
	nullptr,								  /* init process */
	nullptr,								  /* init thread */
	nullptr,                                  /* exit thread */
	nullptr,								  /* exit process */
	nullptr,                                  /* exit master */
	MODULE_V1_PADDING
};


static tag_command  nodes_core_commands[] = {

	{	init_string("edge_path"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, prefix),
		nullptr },

	{	init_string("edge_conf"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, conf),
		nullptr },

	{	init_string("edge_sbin"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, sbin),
		nullptr },

	{	init_string("edge_cert"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, cert),
		nullptr },

	{	init_string("edge_html"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, html),
		nullptr },

	{	init_string("edge_lua"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, lua),
		nullptr },

	{	init_string("edge_cache"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, cache),
		nullptr },

	{	init_string("edge_logs"),
		NODES_CONF | CONF_1MORE,
		modules_parse::string,
		0,
		offsetof(tag_nodes_core, logs),
		nullptr },

	init_string_null
};

void * nodes_core_create_conf(tag_conf *cf)
{
	tag_nodes_core	*nodes_core;

	nodes_core = (tag_nodes_core *)alloc_pool_ptr(cf->pool, sizeof(tag_nodes_core));
	if (nullptr == nodes_core) {
		return nullptr;
	}

	new(&nodes_core->prefix)(string);
	new(&nodes_core->conf)(string);
	new(&nodes_core->sbin)(string);
	new(&nodes_core->cache)(string);
	new(&nodes_core->cert)(string);
	new(&nodes_core->html)(string);
	new(&nodes_core->logs)(string);
	new(&nodes_core->lua)(string);
	return nodes_core;
}

void * nodes_core_init_conf(tag_conf *cf, void *conf)
{
	tag_nodes_core	*nodes_core;
	nodes_core = (tag_nodes_core *)conf;

#if (WINX)
	nodes_core->prefix = u8R"(d:\hystack\auto\)";
	nodes_core->conf = u8R"(d:\hystack\auto\conf\)";
	nodes_core->sbin = u8R"(d:\hystack\auto\sbin\)";
	nodes_core->cache = u8R"(d:\hystack\auto\cache\)";
	nodes_core->cert = u8R"(d:\hystack\auto\cert\)";
	nodes_core->html = u8R"(d:\hystack\auto\html\)";
	nodes_core->logs = u8R"(d:\hystack\auto\logs\)";
	nodes_core->lua = u8R"(d:\hystack\auto\lua\)";
#else
	nodes_init_conf_(nodes_core->prefix, "/usr/local/openresty/nginx/");
	nodes_init_conf_(nodes_core->conf, "/usr/local/openresty/nginx/conf");
	nodes_init_conf_(nodes_core->sbin, "/usr/local/openresty/nginx/sbin");
	nodes_init_conf_(nodes_core->cache, "/usr/local/openresty/nginx/cache");
	nodes_init_conf_(nodes_core->cert, "/usr/local/openresty/nginx/cert");
	nodes_init_conf_(nodes_core->html, "/usr/local/openresty/nginx/html");
	nodes_init_conf_(nodes_core->logs, "/usr/local/openresty/nginx/logs");
	nodes_init_conf_(nodes_core->lua, "/usr/local/openresty/nginx/lua");
#endif

	return nodes_core;
}

tag_nodes_module node_core_module_ctx = {
	init_string("core"),
	nodes_core_create_conf,					/* create configuration */
	nodes_core_init_conf,					/* init configuration */
};

tag_module  nodes_core_module = {
	MODULE_V1,
	&node_core_module_ctx,					/* module context */
	nodes_core_commands,					/* module directives */
	NODES_MODULE,							/* module type */
	nullptr,								/* init master */
	nullptr,								/* init module */
	nullptr,								/* init process */
	nullptr,								/* init thread */
	nullptr,								/* exit thread */
	nullptr,								/* exit process */
	nullptr,								/* exit master */
	MODULE_V1_PADDING
};
