/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   27, SEP, 2021
 */

#include <core_config.h>
#include <core_cycle.h>

#include "process.h"
#include "process_mgr.h"

#include "module/modules.h"
#include "module/modules_conf.h"
#include "module/modules_config.h"

#if (!WINX)
pid_t				process_parent;
pid_t				process_pid;
pid_t				process_new_binary;
#else 
int					process_parent;
int					process_pid;
int					process_new_binary;
#endif

int					process_param_argc;
char			  **process_param_argv;
char			  **process_param_os_argv;
char			  **process_param_os_environ;

unsigned int		process_option_help;
unsigned int		process_option_version;
unsigned int		process_option_show_configure;
unsigned int		process_option_test_config;
unsigned int		process_option_dump_config;
unsigned int		process_option_quiet_node;

unsigned int		process_option_command_line;
std::string			process_option_command_str;

unsigned int		process_type;
unsigned int		process_worker;
unsigned int		process_inherited;
unsigned int		process_daemonized;

std::string 		process_signal;
std::string			process_prefix;
std::string 		process_prefix_conf;

std::string			process_conf_logs;
std::string 		process_conf_params;
std::string 		process_conf_file;
unsigned int		process_conf_test;

sig_atomic_t		process_reap;
sig_atomic_t		process_sigio;
sig_atomic_t		process_sigalrm;
sig_atomic_t		process_terminate;
sig_atomic_t		process_quit;
sig_atomic_t		process_debug_quit;
sig_atomic_t		process_exiting;
sig_atomic_t		process_reconfigure;
sig_atomic_t		process_reopen;
sig_atomic_t		process_change_binary;

std::atomic<int>	process_no_accept;
unsigned int		process_no_accepting;
unsigned int		process_restart;

/* core_cycle */
core_cycle		   *process_cycle;


/* process_mgr::init */
int process_mgr::init(tag_cycle * cycle)
{
	char   *p;
	int		len;

	if (process_prefix.size() > 0)
	{
		len = (int)process_prefix.size();
		p = (char *)process_prefix.data();

		if (len && !(p[len - 1] == '/')) {

			p = (char *)alloc_system_ptr(len + 1);
			if (p == nullptr) {
				return -1;
			}

			memcpy(p, process_prefix.data(), len);
			p[len++] = '/';
		}

		cycle->prefix_conf = p;
		cycle->prefix = len;
		cycle->prefix = p;
	}
	else {

#ifndef MODULE_PREFIX
		const int maxPath = 0xFFF;
		char *dirc, *basec, *dname;

		p = (char *)alloc_system_ptr(maxPath);
		if (p == nullptr) {
			return RET_ERROR;
		}

		if (getcwd(p, maxPath) == 0) {
			return RET_ERROR;
		}

		dirc = strdup(p);
		basec = strdup(p);
		dname = dirname(dirc);

		cycle->prefix = dname;
		cycle->prefix_conf = dname;
		cycle->error_log = dname;

		free(dirc);
		free(basec);
		alloc_system_free(p);

		cycle->prefix += "/";
		cycle->prefix_conf += "/conf/";
		cycle->prefix_logs += "/logs/";
#else
		cycle->prefix = MODULE_PREFIX;
		cycle->prefix_conf = MODULE_PREFIX_CONF;
		cycle->prefix_logs = MODULE_ERROR_LOG;
#endif
	}

	if (process_conf_file.size() > 0) {
		cycle->conf_file = process_conf_file;
	}
	else {
#ifdef MODULE_PREFIX_CONF
		cycle->conf_file = MODULE_PREFIX_CONF;
		cycle->conf_file += "/server.conf";
#else
		cycle->conf_file = cycle->prefix;
		cycle->conf_file += "/conf/server.conf";
#endif
	}

	if (process_conf_logs.size() > 0) {
		cycle->prefix_logs = process_conf_logs;
	}
	else {

#ifdef MODULE_ERROR_LOG
		cycle->prefix_logs = MODULE_ERROR_LOG;
#else 
		cycle->prefix_logs = cycle->prefix;
		cycle->prefix_logs += "/logs";
#endif
	}

	if (process_conf_params.size() > 0) {
		cycle->conf_param = process_conf_params;
	}

	if (process_conf_test == 1) {
		cycle->log->level = logs::Info;
	}

	// core.file;
	process_mgr::dump(cycle);
	return 0;
}
