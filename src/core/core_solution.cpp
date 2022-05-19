/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "core.h"
#include "core_times.h"
#include "core_cycle.h"
#include "core_config.h"

#include <module/modules_load.h>
#include <module/modules_core.h>
#include <module/modules_conf.h>
#include <module/modules_config.h>

#include <process/process_mgr.h>
#include <process/process.h>

#include <logs/logs.h>
#include <logs/logs_config.h>


extern tag_module	core_module;
extern unsigned int	core_process;
extern unsigned int	core_worker;

extern unsigned int	process_option_help;
extern unsigned int	process_option_version;
extern unsigned int	process_option_show_configure;
extern unsigned int	process_option_test_config;
extern unsigned int	process_option_command_line;
extern unsigned int	process_option_dump_config;
extern unsigned int	process_option_quiet_node;

extern std::string 	process_signal;
extern unsigned int	process_inherited;
extern unsigned int	process_daemonized;

/* process_cmdline_option */
int process_cmdline_option(tag_cycle * cycle, tag_cycle * init_cycle);

/* process_socket_initialize */
int process_socket_initialize()
{
#if (WINX)
	WORD		wVersionRequested;
	WSADATA		wsaData;
	int			err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);	//load win socket
	if (err != 0) {
		::exit(0);
	}
#endif

	return 0;
};


/* main */
int main(int argc, char *const *argv)
{
	core			c;
	core_times		c_times;
	core_cycle		c_cycle;

	/* socket */
	process_socket_initialize();

	if (process_option_parse(argc, argv) == RET_OK) {
		if (!process_option_test_config) {
			return 0;
		}
	}

	/* time */
	c_times.init();

	tag_log	* log;
	log = logs_init((unsigned char *)MODULE_PREFIX, (unsigned char *)"errorlog");

#if (!WINX)
	process_pid = getpid();
	process_parent = getppid();

	if (unix_proctitle_init() != 0) {
		return 1;
	}
#endif

	tag_cycle init_cycle;
	process_mgr::init(&init_cycle);

	// load modules ..
	if (modules_load::add_to_cycle(&init_cycle) != 0) {
		return 1;
	}

	tag_cycle * cycle = modules_load::add_to_conf(&init_cycle);
	if (nullptr == cycle) {

		if (process_option_test_config) {
			fprintf(stderr, "\nconfiguration file \"%s\" test failed.\n", init_cycle.conf_path.data());
		}

		fprintf(stderr, "\ninit cycle failed.\n");
		return 1;
	}

	cycle->log = log;
	
	/* testing and cmdline */
	if (process_cmdline_option(cycle, &init_cycle) == 0) {
		return 0;
	}

	/*  */
	if (process_signal.size() > 0) {
		return c_cycle.signal_process(cycle, (char *)process_signal.data());
	}

	tag_core_conf * ccf = nullptr;
	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);
	if (ccf->master && core_process == PROCESS_SINGLE) {
		core_process = PROCESS_MASTER;
		process_type = core_process;
	}

#if (!WINX)
	if (unix_init_signals() != 0) {
		return 1;
	}

	if (!process_inherited && ccf->daemon) {

		if (unix_daemon() != 0) {
			process_daemonized = 1;
		}
	}

	if (process_inherited) {
		process_daemonized = 1;
	}
#endif

	/* open process cycle */
	if (c.create_pid(ccf->pid) != 0) {
		return 1;
	}

	return process_mgr::run(cycle);
}


/* process_command_parameter */
int process_cmdline_option(tag_cycle * cycle, tag_cycle * init_cycle)
{
	if (process_option_test_config) {

		if (process_option_command_line == 1) {
			if (top_process_option_cmdline != nullptr) {
				top_process_option_cmdline(cycle);
				return 0;
			}

			fprintf(stderr, "\ncon't found cmdline module.\n");
			return 0;
		}

		fprintf(stderr, "\nconfiguration file \"%s\" test successful.\n", 
			init_cycle->conf_path.data());
		return 0;
	}

	return -1;
}