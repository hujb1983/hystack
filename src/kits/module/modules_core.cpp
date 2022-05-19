/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "modules.h"
#include "modules_conf.h"
#include "modules_core.h"
#include "modules_cycle.h"
#include "modules_config.h"

#include <core.h>
#include <core_config.h>

#include <alloc/alloc_pool.h>
#include <process/process.h>

static char * set_worker_processes(tag_conf *cf, tag_command *cmd, void *conf);

static tag_command  core_commands[] = {

	{	init_string("daemon"),
		CMD_MAIN_CONF | CMD_DIRECT_CONF | CONF_FLAG,
		modules_parse::flag, 0, offsetof(tag_core_conf, daemon), nullptr
	},

	{	init_string("master_process"),
		CMD_MAIN_CONF | CMD_DIRECT_CONF | CONF_FLAG,
		modules_parse::flag, 0, offsetof(tag_core_conf, master), nullptr
	},

	{	init_string("worker_processes"),
		CMD_MAIN_CONF | CMD_DIRECT_CONF | CONF_TAKE1,
		set_worker_processes, 0, 0, nullptr
	},

	init_string_null
};

void * core_module_create_conf(tag_cycle *cycle)
{
	tag_core_conf * core_conf;

	core_conf = (tag_core_conf *)alloc_pool_ptr(cycle->pool, sizeof(tag_core_conf));
	if (nullptr == core_conf) {
		return nullptr;
	}

	new(&core_conf->pid)string();
	new(&core_conf->old_pid)string();
	new(&core_conf->env)vector<string>();
	new(&core_conf->environment)string();
	new(&core_conf->prefix_path)string();
	new(&core_conf->working_directory)string();

	core_conf->daemon = CMD_CONF_UNSET_UINT;
	core_conf->master = CMD_CONF_UNSET_UINT;
	core_conf->timer_resolution = CMD_CONF_UNSET_MSEC;
	core_conf->shutdown_timeout = CMD_CONF_UNSET_MSEC;
	core_conf->worker_processes = CMD_CONF_UNSET;

	new(&core_conf->env)vector<string>();
	core_conf->env.clear();

#if (!WINX)
	new(&core_conf->username)string();
	core_conf->user = CMD_CONF_UNSET_UINT;
	core_conf->group = CMD_CONF_UNSET_UINT;
#endif

	return static_cast<void *>(core_conf);
}

void * core_module_init_conf(tag_cycle *cycle, void *conf)
{
	tag_core_conf * core_conf = nullptr;
	core_conf = static_cast<tag_core_conf*>(conf);

	if (core_conf->daemon == CMD_CONF_UNSET_UINT) {
		core_conf->daemon = 1;
	}

	if (core_conf->master == CMD_CONF_UNSET_UINT) {
		core_conf->master = PROCESS_MASTER;
	}

	if (core_conf->timer_resolution == CMD_CONF_UNSET_MSEC) {
		core_conf->timer_resolution = 0;
	}

	if (core_conf->shutdown_timeout == CMD_CONF_UNSET_MSEC) {
		core_conf->shutdown_timeout = 0;
	}

	if (core_conf->worker_processes == CMD_CONF_UNSET_UINT) {
		core_conf->worker_processes = 1;
	}

	if (core_conf->pid.size() == 0) {
		core_conf->pid = cycle->prefix;
		core_conf->pid += "/sbin/";
		core_conf->pid += HYSTACK;
		core_conf->pid += ".pid";
	}

	if (core_conf->old_pid.size() == 0) {
		core_conf->old_pid = cycle->prefix;
		core_conf->old_pid += "/sbin/";
		core_conf->old_pid += HYSTACK;
		core_conf->old_pid += MODULE_OLDPID_EXT;
	}

#if (!WINX)
	if (core_conf->user == (uid_t)CMD_CONF_UNSET_UINT && geteuid() == 0) {
		struct group   *grp;
		struct passwd  *pwd;

		errno = 0;
		pwd = getpwnam(AS_USER);
		if (pwd == nullptr) {
			return nullptr;
		}

		core_conf->username = AS_USER;
		core_conf->user = pwd->pw_uid;

		errno = 0;
		grp = getgrnam(AS_GROUP);
		if (pwd == nullptr) {
			return nullptr;
		}

		core_conf->group = grp->gr_gid;
	}
#endif

	return static_cast<void*>(core_conf);
}

static tag_core_module core_module_ctx = {
	init_string("core"),
	core_module_create_conf,
	core_module_init_conf
};


tag_module core_module = {
	MODULE_V1,
	&core_module_ctx,						/* module context */
	core_commands,							/* module directives */
	MODULE_CORE,							/* module type */
	nullptr,                                /* init master */
	nullptr,                                /* init module */
	nullptr,                                /* init process */
	nullptr,                                /* init thread */
	nullptr,                                /* exit thread */
	nullptr,                                /* exit process */
	nullptr,                                /* exit master */
	MODULE_V1_PADDING
};


static char *
set_worker_processes(tag_conf *cf, tag_command *cmd, void *conf)
{
	char				*szValue;
	tag_core_conf		*core_conf;

	core_conf = static_cast<tag_core_conf *>(conf);

	if (core_conf->worker_processes != CMD_CONF_UNSET) {
		return nullptr;
	}

	szValue = const_cast<char *> (cf->args[1].data());
	if (strcmp(szValue, "auto") == 0) {
		core_conf->worker_processes = 2;
		return szValue;
	}

	core_conf->worker_processes = atoi(szValue);
	if (core_conf->worker_processes <= 0) {
		return nullptr;
	}

	return szValue;
}