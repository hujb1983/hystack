/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <core.h>
#include <core_config.h>
#include <core_times.h>

#if (WINX)
#include <conio.h>
#endif

#include "nodes.h"
#include "nodes_cycle.h"
#include "nodes_tasks.h"
#include "nodes_handler.h"
#include "nodes_cmdline.h"
#include "nodes_exception.h"

#include <logs/logs.h>
#include <event/event.h>
#include <event/event_timer.h>
#include <event/event_connection.h>

#include <module/modules.h>
#include <module/modules_load.h>
#include <module/modules_conf.h>
#include <module/modules_core.h>
#include <module/modules_event.h>
#include <module/modules_cycle.h>
#include <module/modules_config.h>
#include <module/modules_nodes.h>

extern tag_cycle			   *root_cycle;
extern vector<tag_cycle*>	   *old_cycle;
extern tag_module				core_module;
extern tag_module				nodes_core_module;

extern sig_atomic_t				process_reap;
extern sig_atomic_t				process_sigio;
extern sig_atomic_t				process_sigalrm;
extern sig_atomic_t				process_terminate;
extern sig_atomic_t				process_quit;
extern sig_atomic_t				process_debug_quit;
extern sig_atomic_t				process_exiting;
extern sig_atomic_t				process_reconfigure;
extern sig_atomic_t				process_reopen;
extern sig_atomic_t				process_change_binary;

extern u_int					process_type;
extern u_int					process_worker;
extern u_int					process_inherited;
extern u_int					process_daemonized;

#if (!WINX)
extern int						unix_process_slot;
extern int						unix_channel_fd;
extern tag_unix_process			unix_processes[PROCESSES_MAX];
#endif

extern int						process_param_argc;
extern char					  **process_param_argv;

static char		core_master_process[] = "master process";

/* nodes_channel_handler */
static void		nodes_channel_handler(tag_event *ev);

/* nodes_channel_handler */
tag_nodes_task					var_nodes_task;
tag_nodes_cmdline				var_nodes_cmds;
tag_nodes_nginx_param			var_nodes_param;
tag_nodes_return_packet			var_nodes_packet;
tag_nodes_auths					var_nodes_auths;
tag_nodes_pulls					var_nodes_pulls;
tag_nodes_upload				var_nodes_statlogs;
tag_nodes_upload				var_nodes_statload;


nodes_cycle::nodes_cycle() {

}

nodes_cycle::~nodes_cycle() {

}

int nodes_cycle::run(tag_cycle *cycle)
{
#if (WINX)

	nodes_single_process(this, cycle);

#else

	/*	nodes_single_process */
	LOG(nullptr, logs::Info) << "start nodes_single_process";
	if (nodes_worker_param_init(this, cycle) == -1) {
		::exit(2);
	}

	master_process(cycle);
#endif

	return 0;
}

int nodes_cycle::signal_process(tag_cycle *cycle, char * signo)
{
#if (!WINX)
	int					 n;
	pid_t				 pid;
	tag_core_conf		*ccf;
	unsigned char		 buf[TYPE_INT64_LEN + 2];

	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	int fd = open((char *)ccf->pid.data(), O_RDWR | O_CREAT, 0644);
	if (fd == -1) {
		return -1;
	}

	memset(buf, 0x0, sizeof(buf));
	n = (int)read(fd, buf, TYPE_INT64_LEN);
	close(fd);

	if (n == -1) {
		return 1;
	}

	while (n-- && (buf[n] == CR || buf[n] == LF)) { /* void */ }

	pid = atoi((char*)buf);
	if (pid == -1) {
		LOG_ERROR(nullptr) << "invalid PID number " << n << " in " << (char *)buf << ".";
		return 1;
	}

	return unix_signal_process(cycle, signo, pid);
#endif
	return 0;
}

int	nodes_cycle::single_process(tag_cycle *cycle)
{
	for (auto m : cycle->modules)
	{
		if (m->init_process != nullptr)
		{
			if (m->init_process(cycle) == -1) {
				::exit(2);
			}
		}
	}

	for (;;)
	{
		// process_events_and_timers(cycle);

		if (process_terminate || process_quit)
		{
			for (auto m : cycle->modules) {
				m->exit_process(cycle);
			}

			master_process_exit(cycle);
		}

		if (process_reconfigure) {
			process_reconfigure = 0;

			cycle = modules_conf::copy(cycle);
			if (cycle == nullptr) {
				cycle = (tag_cycle *)root_cycle;
				continue;
			}
		}

		if (process_reopen) {
			process_reopen = 0;
			reopen_files(cycle, -1);
		}
	}

	return this->master_process_exit(cycle);
}

int	nodes_cycle::master_process(tag_cycle *cycle)
{
#if (!WINX)
	sigset_t	set;

	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, SIGALRM);
	sigaddset(&set, SIGIO);
	sigaddset(&set, SIGINT);
	sigaddset(&set, unix_signal_value(SIGNAL_RECONFIGURE));
	sigaddset(&set, unix_signal_value(SIGNAL_REOPEN));
	sigaddset(&set, unix_signal_value(SIGNAL_NOACCEPT));
	sigaddset(&set, unix_signal_value(SIGNAL_TERMINATE));
	sigaddset(&set, unix_signal_value(SIGNAL_SHUTDOWN));
	sigaddset(&set, unix_signal_value(SIGNAL_CHANGEBIN));

	if (sigprocmask(SIG_BLOCK, &set, nullptr) == -1) {
		LOG_ERROR(nullptr) << errno << "sigprocmask() failed";
	}

	sigemptyset(&set);
	unix_set_proctitle(core_master_process);

	tag_core_conf * ccf;
	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	process_new_binary = 0;
	nodes_start_worker_process(cycle, ccf->worker_processes, PROCESS_RESPAWN);

	unsigned int         sigio;
	unsigned int		 live;
	unsigned long long	 delay;
	struct itimerval	 itv;

	delay = 0;
	sigio = 0;
	live = 1;

	for (;; )
	{
		if (delay)
		{
			if (process_sigalrm)
			{
				sigio = 0;
				delay *= 2;
				process_sigalrm = 0;
			}

			itv.it_interval.tv_sec = 0;
			itv.it_interval.tv_usec = 0;
			itv.it_value.tv_sec = delay / 1000;
			itv.it_value.tv_usec = (delay % 1000) * 1000;

			if (setitimer(ITIMER_REAL, &itv, nullptr) == -1) {
				LOG_ERROR(nullptr) << errno << " setitimer() failed";
			}
		}

		sigsuspend(&set);
		core_times::init();

		if (process_reap) {
			process_reap = 0;
			live = unix_reap_children(cycle);
		}

		if (!live && (process_terminate || process_quit)) {
			master_process_exit(cycle);
		}

		if (process_terminate) {

			if (delay == 0) {
				delay = 50;
			}

			if (sigio) {
				sigio--;
				continue;
			}

			sigio = ccf->worker_processes + 2;

			if (delay > 1000) {
				signal_worker_process(cycle, SIGKILL);
			}
			else {
				signal_worker_process(cycle, unix_signal_value(SIGNAL_TERMINATE));
			}
		}

		if (process_quit) {
			signal_worker_process(cycle, unix_signal_value(SIGNAL_SHUTDOWN));
			event_listening::close(cycle);
			continue;
		}

		if (process_reconfigure) {
			process_reconfigure = 0;

			if (process_new_binary) {
				nodes_start_worker_process(cycle, ccf->worker_processes, PROCESS_RESPAWN);
				process_no_accepting = 0;
				continue;
			}

			cycle = modules_load::add_to_conf(cycle);
			if (cycle == nullptr) {
				cycle = (tag_cycle *)root_cycle;
				continue;
			}

			root_cycle = cycle;
			ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);
			nodes_start_worker_process(cycle, ccf->worker_processes, PROCESS_JUST_RESPAWN);

			/* sleep(1000) */
			this_thread::sleep_for(chrono::milliseconds(1000));

			live = 1;
			signal_worker_process(cycle, unix_signal_value(SIGNAL_SHUTDOWN));
		}

		if (process_restart) {
			process_restart = 0;
			nodes_start_worker_process(cycle, ccf->worker_processes, PROCESS_RESPAWN);
			live = 1;
		}

		if (process_reopen) {
			process_reopen = 0;
			LOG_DEBUG(nullptr) << 0 << " reopening logs.";
			reopen_files(cycle, ccf->user);
			signal_worker_process(cycle, unix_signal_value(SIGNAL_REOPEN));
		}

		if (process_change_binary) {
			process_change_binary = 0;
			LOG_DEBUG(nullptr) << 0 << "changing binary";
			process_new_binary = core::exec_new_binary(cycle, process_param_argv);
		}

		if (process_no_accept) {
			process_no_accept = 0;
			process_no_accepting = 1;
			signal_worker_process(cycle, unix_signal_value(SIGNAL_SHUTDOWN));
		}
	}

#endif
	return 0;
}

int	nodes_cycle::master_process_exit(tag_cycle *cycle)
{
	tag_pool * pool = cycle->pool;
	alloc_pool_destory(cycle->pool);

	if (pool != cycle->pool_temp) {
		alloc_pool_destory(cycle->pool_temp);
	}

	cycle->pool = nullptr;
	cycle->pool_temp = nullptr;

	for (auto m : cycle->modules) {
		if (m->exit_master != nullptr) {
			m->exit_master(cycle);
		}
	}
	
	core::delete_pid(cycle);
	::exit(0);

	return 0;
}

/* nodes_cycle::nodes_worker_process_init */
int nodes_cycle::nodes_worker_param_init(nodes_cycle *core, tag_cycle *cycle)
{
	var_nodes_task.cycle = cycle;
	var_nodes_task.cmds = &var_nodes_cmds;
	
	var_nodes_task.param = &var_nodes_param;
	var_nodes_task.packet = &var_nodes_packet;

	var_nodes_task.auths = &var_nodes_auths;
	var_nodes_task.pulls = &var_nodes_pulls;
	var_nodes_task.statlogs = &var_nodes_statlogs;
	var_nodes_task.statload = &var_nodes_statload;

	tag_nodes_core	*ncf;
	ncf = (tag_nodes_core *)nodes_get_conf(cycle->conf_ctx, nodes_core_module);
	var_nodes_param.nginx.prefix = ncf->prefix;
	var_nodes_param.nginx.conf = ncf->conf;
	var_nodes_param.nginx.sbin = ncf->sbin;
	var_nodes_param.nginx.cache = ncf->cache;
	var_nodes_param.nginx.cert = ncf->cert;
	var_nodes_param.nginx.html = ncf->html;
	var_nodes_param.nginx.logs = ncf->logs;
	var_nodes_param.nginx.lua = ncf->lua;

	/* process */
	map_nodes_task_process & proc = var_nodes_task.proc;
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

	if (nodes_handler_authed_check(&var_nodes_task) == -1) {
		LOG_STDERR << "Authorized failed.";
		return -1;
	}
	
	return 0;
}

int nodes_cycle::nodes_single_process(nodes_cycle *core, tag_cycle *cycle)
{
	for (auto m : cycle->modules)
	{
		if (m->init_process != nullptr)
		{
			if (m->init_process(cycle) == -1) {
				::exit(2);
			}
		}
	}

	/*	nodes_single_process */
	LOG(nullptr, logs::Info) << "start nodes_single_process";
	if (nodes_worker_param_init(core, cycle) == -1) {
		::exit(2);
	}

	// session_pulls->on_init(cycle);

	for (;; )
	{
		/* sync process */
		// session_pulls->on_process(cycle);

		if (process_terminate || process_quit)
		{
			for (auto m : cycle->modules) {
				m->exit_process(cycle);
			}

			core->master_process_exit(cycle);
		}

		if (process_reconfigure) {
			process_reconfigure = 0;

			cycle = modules_conf::copy(cycle);
			if (cycle == nullptr) {
				cycle = (tag_cycle *)root_cycle;
				continue;
			}
		}

		if (process_reopen) {
			process_reopen = 0;
			core->reopen_files(cycle, -1);
		}
	}

	return this->master_process_exit(cycle);
}


void nodes_cycle::nodes_worker_process_exit(tag_cycle *cycle)
{
	for (auto m : cycle->modules) {
		if (m->exit_process != nullptr) {
			m->exit_process(cycle);
		}
	}

	event_listening::close(cycle);
	alloc_pool_destory(cycle->pool);
	alloc_pool_destory(cycle->pool_temp);

	::exit(0);
}

void nodes_cycle::nodes_worker_process_init(tag_cycle *cycle, int worker)
{
#if (!WINX)

	sigset_t		 set;
	int				 n;
	tag_core_conf	*ccf;

	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	if (worker >= 0 && ccf->priority != 0)
	{
		if (setpriority(PRIO_PROCESS, 0, ccf->priority) == -1) 
		{
			LOG_ERROR(nullptr) << errno << "setpriority " << ccf->priority << " failed";
		}
	}

	sigemptyset(&set);
	if (sigprocmask(SIG_SETMASK, &set, nullptr) == -1) {
		LOG_ERROR(nullptr) << errno << "sigprocmask() failed";
	}

	for (auto m : cycle->modules) {
		if (m->init_process != nullptr) {
			if (m->init_process(cycle) == -1) {
				::exit(2);
			}
		}
	}

	for (n = 0; n < unix_process_last; n++) {

		if (unix_processes[n].pid == -1) {
			continue;
		}

		if (n == unix_process_slot) {
			continue;
		}

		if (unix_processes[n].channel[1] == -1) {
			continue;
		}

		if (close(unix_processes[n].channel[1]) == -1) {
			LOG_ALERT(nullptr) << errno << "close() channel failed";
		}
	}

	if (close(unix_processes[unix_process_slot].channel[0]) == -1) {
		LOG_ALERT(nullptr) << errno << "close() channel failed";
	}

	if (unix_channel_add_event(cycle, unix_channel_fd, EVENT_READ, nodes_channel_handler) == -1) {
		::exit(2); /* fatal */
	}
#endif
}

void nodes_cycle::nodes_pulls_process_cycle(tag_cycle *cycle, void *data)
{
	int worker = (intptr_t)data;
	process_type = PROCESS_WORKER;
	process_worker = worker;

	nodes_worker_process_init(cycle, worker);

#if (!WINX)
	unix_set_proctitle("sync-pulls process");
#endif

	/* heartbeat timespan */
	u_int delay_time = 6;

	for (;; ) {

		nodes_handler_pulls_process(&var_nodes_task);

		/* heartbeat timespan 6s. */
		std::this_thread::sleep_for(std::chrono::seconds(delay_time));
		core_times::update();

		if (process_exiting) {

			if (event_timer::no_left() == 0) {
				LOG_NOTICE(nullptr) << "exiting";
				LOG_TRACE(nullptr) << "gracefully shutting down" << 1;
				nodes_worker_process_exit(cycle);
			}
		}

		if (process_terminate || process_quit) {
			nodes_worker_process_exit(cycle);
		}

		if (process_quit) {
			process_quit = 0;

#if (!WINX)
			NODES_TRACE("gracefully shutting down", 1);
			unix_set_proctitle("worker process is shutting down");
#endif

			if (!process_exiting) {
				process_exiting = 1;
				nodes_cycle::set_shutdown_timer(cycle);
				event_listening::close(cycle);
				event_listening::close_idle(cycle);
			}
		}

		if (process_reopen) {
			process_reopen = 0;
			nodes_cycle::reopen_files(cycle, -1);
		}
	}
}


void nodes_cycle::nodes_stat_process_cycle(tag_cycle *cycle, void *data)
{
	int worker = (intptr_t)data;
	process_type = PROCESS_WORKER;
	process_worker = worker;

	nodes_worker_process_init(cycle, worker);

#if (!WINX)
	unix_set_proctitle("stat-load process");
#endif

	/* heartbeat timespan 300s. */
	int delay_time = 300;

	for (;; ) {

		/* stat load */
		LOG_DEBUG(nullptr) << "start to stat-load";
		nodes_handler_statload_process(&var_nodes_task);

		/* heartbeat timespan 300s. */
		std::this_thread::sleep_for(std::chrono::seconds(delay_time));
		core_times::update();

		if (process_exiting) {

			if (event_timer::no_left() == 0) {
				LOG_NOTICE(nullptr) << "exiting";
				LOG_TRACE(nullptr) << "gracefully shutting down" << 1;
				nodes_worker_process_exit(cycle);
			}
		}

		if (process_terminate || process_quit) {
			nodes_worker_process_exit(cycle);
		}

		if (process_quit) {
			process_quit = 0;

#if (!WINX)
			NODES_TRACE("gracefully shutting down", 1);
			unix_set_proctitle("worker process is shutting down");
#endif

			if (!process_exiting) {
				process_exiting = 1;
				nodes_cycle::set_shutdown_timer(cycle);
				event_listening::close(cycle);
				event_listening::close_idle(cycle);
			}
		}

		if (process_reopen) {
			process_reopen = 0;
			nodes_cycle::reopen_files(cycle, -1);
		}
	}
}

void nodes_cycle::nodes_logs_process_cycle(tag_cycle *cycle, void *data)
{
	int worker = (intptr_t)data;
	process_type = PROCESS_WORKER;
	process_worker = worker;

	nodes_worker_process_init(cycle, worker);

#if (!WINX)
	unix_set_proctitle("stat-logs process");
#endif

	/* heartbeat timespan 3s. */
	int delay_time = 3;
	int stat_count = 0;

	for (;; ) {

		/* stat logs count.*/
		stat_count += 1;
		if ((stat_count % 100) == 0) {
			stat_count = 0;
			LOG_DEBUG(nullptr) << "start to stat-logs next 100/times";
		}

		nodes_handler_statlogs_process(&var_nodes_task);

		/* heartbeat timespan 3s. */
		std::this_thread::sleep_for(std::chrono::seconds(delay_time));
		core_times::update();

		if (process_exiting) {
			if (event_timer::no_left() == 0) {
				LOG_NOTICE(nullptr) << "exiting";
				LOG_TRACE(nullptr) << "gracefully shutting down" << 1;
				nodes_worker_process_exit(cycle);
			}
		}

		if (process_terminate || process_quit) {
			nodes_worker_process_exit(cycle);
		}

		if (process_quit) {
			process_quit = 0;

#if (!WINX)
			NODES_TRACE("gracefully shutting down", 1);
			unix_set_proctitle("worker process is shutting down");
#endif

			if (!process_exiting) {
				process_exiting = 1;
				nodes_cycle::set_shutdown_timer(cycle);
				event_listening::close(cycle);
				event_listening::close_idle(cycle);
			}
		}

		if (process_reopen) {
			process_reopen = 0;
			nodes_cycle::reopen_files(cycle, -1);
		}
	}

}


void nodes_cycle::nodes_start_worker_process(tag_cycle *cycle, int n, int type)
{
#if (!WINX)

	{
		unix_spawn_process(cycle, 
			nodes_cycle::nodes_pulls_process_cycle,
			(void *)(intptr_t)0,
			(char *)"pulls process", type);
		unix_pass_open_channel(cycle);
	}

	{
		unix_spawn_process(cycle, 
			nodes_cycle::nodes_logs_process_cycle, 
			(void *)(intptr_t)2,
			(char *)"logs process", type);
		unix_pass_open_channel(cycle);
	}

	{
		unix_spawn_process(cycle,
			nodes_cycle::nodes_stat_process_cycle,
			(void *)(intptr_t)1,
			(char *)"stat process", type);
		unix_pass_open_channel(cycle);
	}

#endif
}

/* nodes_channel_handler */
static void nodes_channel_handler(tag_event *ev)
{
#if (!WINX)
	int				 n;
	tag_connection  *c;
	tag_channel		 ch;

	if (ev->timed_out) {
		ev->timed_out = 0;
		return;
	}

	c = (tag_connection *)ev->data;

	for (;; )
	{
		n = unix_channel_read(c->fd, &ch, sizeof(tag_channel));

		LOG_DEBUG(nullptr) << "channel: " << n;

		if (n == -1) {

			if (proc_event_flags & EVENT_USE_EPOLL) {
				proc_del_conn(c, 0);
			}

			event_connection::close(c);
			return;
		}

		if (proc_event_flags & EVENT_USE_EVENTPORT) {
			if (proc_add_event(ev, EVENT_READ, 0) == -1) {
				return;
			}
		}

		if (n == RET_AGAIN) {
			return;
		}

		LOG_DEBUG(nullptr) << "channel command: " << ch.command;

		switch (ch.command) {

		case CMD_QUIT:
			process_quit = 1;
			break;

		case CMD_TERMINATE:
			process_terminate = 1;
			break;

		case CMD_REOPEN:
			process_reopen = 1;
			break;

		case CMD_OPEN_CHANNEL:

			LOG_DEBUG(nullptr) << "get channel"
				<< "s: " << ch.slot
				<< "pid: " << ch.pid
				<< "fd: " << ch.fd;

			unix_processes[ch.slot].pid = ch.pid;
			unix_processes[ch.slot].channel[0] = ch.fd;
			break;

		case CMD_CLOSE_CHANNEL:

			LOG_DEBUG(nullptr) << "get channel"
				<< "s: " << ch.slot << "pid: " << ch.pid
				<< "out: " << unix_processes[ch.slot].pid
				<< "fd: " << unix_processes[ch.slot].channel[0];

			if (close(unix_processes[ch.slot].channel[0]) == -1) {
				LOG_DEBUG(nullptr) << errno << "close() channel failed";
			}

			unix_processes[ch.slot].channel[0] = -1;
			break;
		}
	}

#endif
}

/* nodes_cycle::nodes_sleep_process_cycle */
void nodes_cycle::nodes_sleep_process_cycle(unsigned int times)
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

	/* int			kb = 0l;
	fd_set		readfd;

	kb = open("/dev/tty", O_RDONLY | O_NONBLOCK);
	FD_ZERO(&readfd);
	FD_SET(kb, &readfd);

	struct timeval tp;
	tp.tv_sec = 1;
	tp.tv_usec = 0;
	int ret = select(kb + 1, &readfd, NULL, NULL, &tp);
	if (ret > 0) {
		if (FD_ISSET(kb, &readfd)) {
			char c[1025] = { 0 }
			read(kb, c, 1024);
		}
	}
	else {

	} */
#endif
}