/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <stdio.h>
#include <stdlib.h>

#if (WINX)
#include <conio.h>
#endif

#include <thread>
#include <chrono>
using namespace std;
using namespace chrono;

#include <stdio.h>
#include <iostream>
#include <core_times.h>
#include <core_config.h>

#include <module/modules_cycle.h>
#include <module/modules_config.h>
#include <module/modules_nodes.h>

#include <dev/dev_stat.h>
#include <dev/dev_nginx.h>

#include "nodes.h"
#include "nodes_tasks.h"
#include "nodes_stream.h"
#include "nodes_cmdline.h"
#include "nodes_handler.h"
#include "nodes_process.h"
#include "nodes_exception.h"

extern tag_module	nodes_core_module;
extern std::string	process_option_command_line;
extern std::string	process_option_command_str;
extern unsigned int	process_daemonized;

/* nodes_test_init */
static void nodes_test_init(tag_nodes_task * task);
static int nodes_test_command_parse(tag_nodes_cmdline * cmdline, char * cmd, int len);
static int nodes_test_command_select(unsigned int times);

 /* nodes_test_commandline */
void nodes_test_commandline(tag_cycle * cycle)
{
	char	cmd[255] = {0};

	tag_nodes_task			task;
	tag_nodes_cmdline		cmdline;

	tag_nodes_auths			auths;
	tag_nodes_pulls			pulls;
	tag_nodes_upload		logs;
	tag_nodes_upload		stat;

	tag_nodes_nginx_param	param;
	tag_nodes_return_packet	packet;

	task.cycle = cycle;
	task.cmds = &cmdline;
	
	task.auths = &auths;
	task.pulls = &pulls;
	task.statlogs = &logs;
	task.statload = &stat;

	task.param = &param;
	task.packet = &packet;

	// start command line.
	nodes_test_init(&task);

	if (process_option_command_str.size() != 0) {
		std::string &cmd_str = process_option_command_str;
		memcpy(cmd, cmd_str.data(), cmd_str.size());
	}
	else {
		fprintf(stderr, "The version is %s\n", MODULE_VER);
		fprintf(stderr, "Please input your command.\n");
	}

	// start input your command.
	// for
	for (;;)
	{
		if (strlen(cmd) == 0) {
			goto next_cmdline;
		}

		/* test command */
		if (nodes_test_command_parse(task.cmds, cmd, 255) == -1) {
			continue;
		}

		if (strcmp(cmd, "defense") == 0) {
			nodes_test_defense(&task);
			goto next_cmdline;
		}
		else if (strcmp(cmd, "nodeslogs") == 0) {
			nodes_test_nodeslogs(&task);
			nodes_error_output(&task, "", 1);
			goto next_cmdline;
		}
		else if (strcmp(cmd, "stopnodes") == 0) {
			nodes_test_stop_asnode(&task);
			nodes_error_output(&task, "", 1);
			goto next_cmdline;
		}

		do
		{
			if (strcmp(cmd, "authlogin") == 0) {
				nodes_test_authlogin(&task);
				goto finish_cmdline;
			}
			else if (strcmp(cmd, "heartbeat") == 0) {
				nodes_test_heartbeat(&task);
				goto finish_cmdline;
			}
			else if (strcmp(cmd, "pulltask") == 0) {
				nodes_test_pulltask(&task);
				goto finish_cmdline;
			}
			else if (strcmp(cmd, "statlogs") == 0) {
				nodes_test_statlogs(&task);
				goto finish_cmdline;
			}
			else if (strcmp(cmd, "statload") == 0) {
				nodes_test_statload(&task);
				goto finish_cmdline;
			}
			else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
				return;
			}

			fprintf(stderr, "unknow command.\n");
			break;
		
		/* continue and break */
		finish_cmdline:

			core_times::update(); 
			nodes_error_output(&task, (char *)core_cmdline_errorlog_time.data, 1);

			if (nodes_test_command_select(task.cmds->reprocess_times) == 0) {
				continue;
			}
		
		} while (task.cmds->reprocess == 1);
	
	next_cmdline:

		if (process_option_command_str.size() > 0) {
			process_option_command_str.clear();
			break;
		}

		memset(cmd, 0x0, sizeof(cmd));
		fprintf(stderr, "%s>> ", MODULE_VER);
		cin.getline(cmd, sizeof(cmd), '\n');
		continue;
	}

	return;
}

/* nodes_test_commandline */
std::shared_ptr<nodes_stream>	nodes_cmdline_debug;
std::shared_ptr<nodes_stream>	nodes_cmdline_error;
std::shared_ptr<nodes_stream>	nodes_cmdline_track;

/* nodes_test_authlogin */
void nodes_test_init(tag_nodes_task * task)
{
	tag_cycle	* cycle = task->cycle;
	tag_nodes_nginx_param * param = task->param;

	tag_nodes_core	*ncf;
	ncf = (tag_nodes_core *)nodes_get_conf(cycle->conf_ctx, nodes_core_module);
	param->nginx.prefix = ncf->prefix;
	param->nginx.conf = ncf->conf;
	param->nginx.sbin = ncf->sbin;
	param->nginx.cache = ncf->cache;
	param->nginx.cert = ncf->cert;
	param->nginx.html = ncf->html;
	param->nginx.logs = ncf->logs;
	param->nginx.lua = ncf->lua;

	/* process */
	map_nodes_task_process & proc = task->proc;
	proc.clear();

	proc[199] = nodes_task_step_process;
	proc[101] = nodes_task_nginx_process;
	proc[102] = nodes_task_ssl_process;
	proc[105] = nodes_task_redis_process;
	proc[129] = nodes_task_errpage_process;

	/* lua */
	proc[126] = nodes_task_lua_process;
	proc[127] = nodes_task_lua_process;
	proc[128] = nodes_task_lua_process;
	proc[130] = nodes_task_lua_process;
	proc[131] = nodes_task_lua_process;
	proc[132] = nodes_task_lua_process;

	proc[133] = nodes_task_defense_process;
	proc[135] = nodes_task_cache_process;

	/* read - stream */
	std::string prefix = param->nginx.prefix;
	nodes_cmdline_debug = std::make_shared<nodes_stream>(prefix);
	nodes_cmdline_error = std::make_shared<nodes_stream>(prefix);
	nodes_cmdline_track = std::make_shared<nodes_stream>(prefix);
	nodes_cmdline_debug->init("/net/", u8R"(nodes_stream_debug)");
	nodes_cmdline_error->init("/net/", u8R"(nodes_stream_error)");
	nodes_cmdline_track->init("/net/", u8R"(nodes_stream_track)");

	nodes_cmdline_debug->redirect();
	nodes_cmdline_error->redirect();
	nodes_cmdline_track->redirect();
}

/* nodes_test_command_parse */
int nodes_test_command_parse(tag_nodes_cmdline * cmdline, char * cmd, int len)
{
	std::vector<std::string> option;
	option.clear();

	char * temp = strtok((char *)cmd, " ");
	while(temp != nullptr)
	{
		option.push_back(temp);
		temp = strtok(nullptr, " ");
	};

	/* option.size() */
	if (option.size() == 0) {
		return -1;
	}

	/* init */
	tag_nodes_cmdline *cmds = cmdline;
	cmds->output = 1;
	cmds->output_buff.clear();

	cmds->output_file = 1;
	cmds->output_error = 1;
	cmds->output_stderr = 1;
	cmds->output_sent = 1;
	cmds->output_recv = 1;
	cmds->reprocess = 0;
	cmds->reprocess_times = 0;
	cmds->print_nodeslogs = 0;
	cmds->print_nginxlogs = 0;

	memset(cmd, 0x0, len);
	memcpy(cmd, option[0].data(), option[0].size());

	char	*p = nullptr;
	for (int i = 1; i < (int)option.size(); i++) 
	{
		p = (char *)option[i].data();

		if (*p++ != '-') {
			return -1;
		}

		while (*p) {

			char *s = p;

			switch (*p++) {
			
			case 't': {

				if (strcmp(s, "trace") == 0) {
					cmds->print_nodeslogs = 0x0F00;
					cmds->print_nginxlogs = 0x0F00;
					goto next;
				}

				if (strcmp(s, "t") == 0) 
				{
					if ((int)option.size() > (i+1)) {
						cmdline->reprocess_times = atoi(option[++i].data());
						if (cmdline->reprocess_times < 6) {
							cmdline->reprocess = 0;
							fprintf(stderr, "The timespan shall not be less than 6 seconds.");
							return -1;
						}
						cmdline->reprocess = 1;
					}
				}

				goto next;
			}
			
			case 'd': {
				if (strcmp(s, "debug") == 0) {
					cmds->print_nodeslogs = 0x000F;
					cmds->print_nginxlogs = 0x000F;
					goto next;
				}
			}

			case 'e': {
				if (strcmp(s, "error") == 0) {
					cmds->print_nodeslogs = 0x00F0;
					cmds->print_nginxlogs = 0x00F0;
					goto next;
				}
			}

			default:
				fprintf(stderr, "invalid option: \"%c\".\n", *(p - 1));
				return RET_ERROR;
			};
		}
	next:
		continue;
	}

	return 0;
}

/* nodes_test_command_parse */
int nodes_test_command_select(unsigned int times)
{
#if (WINX)
	unsigned int i = times;

	while (i--) {
		this_thread::sleep_for(chrono::seconds(1));
		if (!_kbhit()) {
			break;
		}
	}
#else 
	this_thread::sleep_for(chrono::seconds(times));
#endif
	return 0;
}

/* nodes_error_output */
void nodes_error_output(tag_nodes_task * task, const std::string & errmsg, int type)
{
	if (errmsg.size() == 0) {
		return;
	}

	tag_nodes_cmdline * cmds = task->cmds;

	if (type == 0) {
		cmds->output_buff += errmsg;
		cmds->output_buff += '\n';
		return;
	}

	if (cmds->output == 1)
	{
		if (cmds->output_error == 1)
		{
			if (cmds->output_buff.size() > 0) {
				NODES_TRACE(cmds->output_buff, 0);
			}

			if (errmsg.size() > 0) {
				NODES_TRACE(errmsg, -1);
			}
		}

		if (cmds->output_stderr == 1)
		{
			if (cmds->output_buff.size() > 0) {
				fprintf(stderr, "%s", cmds->output_buff.data());
				fprintf(stderr, "\n");
			}

			if (errmsg.size() > 0) {
				fprintf(stderr, "%s", errmsg.data());
				fprintf(stderr, "\n");
			}

			cmds->output_buff.clear();
		}
	}
}


/* nodes_test_authlogin */
void nodes_test_authlogin(tag_nodes_task * task)
{
	tag_nodes_cmdline * cmds = task->cmds;
	cmds->output_buff.clear();

	if (nodes_handler_authed_check(task) == -1) {
		nodes_error_output(task, "Authorized checking failed.");
		return;
	}
}

/* nodes_test_heartbeat */
void nodes_test_heartbeat(tag_nodes_task * task)
{
	tag_nodes_auths * auth = task->auths;
	tag_nodes_cmdline * cmds = task->cmds;
	cmds->output_buff.clear();

	if (auth->token.size() == 0) 
	{
		if (nodes_handler_authed_check(task) == -1) {
			nodes_error_output(task, "Authorized login failed.");
			return;
		}
	}

	nodes_handler_heartbeat(task);
}

/* nodes_test_pulltask */
void nodes_test_pulltask(tag_nodes_task * task)
{
	tag_nodes_auths * auth = task->auths;
	tag_nodes_pulls * pull = task->pulls;
	tag_nodes_cmdline * cmds = task->cmds;
	cmds->output_buff.clear();

	if (auth->token.size() == 0)
	{
		if (nodes_handler_authed_check(task) == -1) {
			nodes_error_output(task, "Authorized login failed.");
			return;
		}
	}

	if (pull->requests.size() == 0) 
	{
		if (nodes_handler_heartbeat(task) == -1) {
			nodes_error_output(task, "heartbeat failed.");
			return;
		}
	}

	if (pull->requests.size() == 0) {
		nodes_error_output(task, "request task 0");
		return;
	}


	NODES_DEBUG("\nstart process task ", 0);
	std::map<std::string, int>	map_temp;

	while (true) {

		map_temp.clear();
		for (auto r : pull->requests)
		{
			pull->task_id = r.first;
			int ret = nodes_handler_pull_task(task);
			if (ret == -1) {
				if (r.second > 6) {
					continue;
				}
				map_temp[r.first] = r.second + 1;
			}
		}

		if (map_temp.size() == 0) {
			break;
		}

		pull->requests.clear();
		pull->requests.insert(std::begin(map_temp), std::end(map_temp));
	}

	pull->requests.clear();
	NODES_DEBUG("end process task ", 0);
}

/* nodes_test_logs */
void nodes_test_statlogs(tag_nodes_task * task)
{
	tag_nodes_auths * auth = task->auths;
	tag_nodes_cmdline * cmds = task->cmds;
	cmds->output_buff.clear();

	if (auth->token.size() == 0)
	{
		if (nodes_handler_authed_check(task) == -1) {
			nodes_error_output(task, "Authorized login failed.");
			return;
		}
	}

	/* stat-logs */
	nodes_handler_statlogs_process(task);
}

/* nodes_test_stat */
void nodes_test_statload(tag_nodes_task * task)
{
	tag_nodes_auths * auth = task->auths;
	tag_nodes_cmdline * cmds = task->cmds;
	cmds->output_buff.clear();

	if (auth->token.size() == 0)
	{
		if (nodes_handler_authed_check(task) == -1) {
			nodes_error_output(task, "Authorized login failed.");
			return;
		}
	}

	/* stat-load */
	nodes_handler_statload_process(task);
}

/* print /cmdline/defense; */
void nodes_test_nodeslogs(tag_nodes_task * task)
{
	u_char	recv_data[4000] = { 0 };
	u_int	recv_size = 3999L;

	tag_nodes_cmdline * cmds = task->cmds;

	if (cmds->print_nodeslogs == 0x000F) {
		nodes_cmdline_debug->read(recv_data, recv_size);
		nodes_error_output(task, (char *)recv_data);
	}

	if (cmds->print_nodeslogs == 0x00F0) {
		nodes_cmdline_error->read(recv_data, recv_size);
		nodes_error_output(task, (char *)recv_data);
	}

	if (cmds->print_nodeslogs == 0x0F00) {
		nodes_cmdline_track->read(recv_data, recv_size);
		nodes_error_output(task, (char *)recv_data);
	}
}


/* print /cmdline/defense; */
void nodes_test_defense(tag_nodes_task * task)
{

}


/* nodes stop */
void nodes_test_stop_asnode(tag_nodes_task * task)
{
#if (!WINX)
	if (unix_daemon() != 0) {
		process_daemonized = 1;
	}
#endif 

	std::string prefix = MODULE_PREFIX;
	std::string cmdline = prefix + "/sbin/Asnode -s stop";
	std::string error;
	
	/* nodes_process_popen */
	nodes_process_popen(cmdline.data(), error);
	if (error.size() == 0) {
		this_thread::sleep_for(chrono::seconds(1));
	}
}
