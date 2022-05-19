/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   27, SEP, 2021
 */

#include <string>
#include <core_config.h>

#include <process/process.h>
#include <process/process_mgr.h>

#include "unix_config.h"

#define PROCESS_INVALID_PID		-1

extern std::string 		process_signal;
extern unsigned int		process_inherited;
extern unsigned int		process_daemonized;
extern unsigned int		process_type;

extern pid_t			process_parent;
extern pid_t			process_pid;
extern pid_t			process_new_binary;

int						unix_process_slot;
tag_unix_process		unix_processes[PROCESSES_MAX];
int						unix_channel_fd;
int						unix_process_last;

typedef struct {
	int				signo;
	const char*		signame;
	const char*		name;
	void(*handler)(int signo, siginfo_t *siginfo, void *ucontext);
} tag_unix_process_signal;


static void unix_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);
static void unix_process_get_status(void);


tag_unix_process_signal signals[] =
{
	{ unix_signal_value(SIGNAL_RECONFIGURE),
	  "SIG" process_value(SIGNAL_RECONFIGURE),
	  "reload",
	  unix_signal_handler },

	{ unix_signal_value(SIGNAL_REOPEN),
	  "SIG" process_value(SIGNAL_REOPEN),
	  "reopen",
	  unix_signal_handler },

	{ unix_signal_value(SIGNAL_NOACCEPT),
	  "SIG" process_value(SIGNAL_NOACCEPT),
	  "",
	  unix_signal_handler },

	{ unix_signal_value(SIGNAL_TERMINATE),
	  "SIG" process_value(SIGNAL_TERMINATE),
	  "stop",
	  unix_signal_handler },

	{ unix_signal_value(SIGNAL_SHUTDOWN),
	  "SIG" process_value(SIGNAL_SHUTDOWN),
	  "quit",
	  unix_signal_handler },

	{ unix_signal_value(SIGNAL_CHANGEBIN),
	  "SIG" process_value(SIGNAL_CHANGEBIN),
	  "",
	  unix_signal_handler },

	{ SIGALRM, "SIGALRM", "", unix_signal_handler },

	{ SIGINT, "SIGINT", "", unix_signal_handler },

	{ SIGIO, "SIGIO", "", unix_signal_handler },

	{ SIGCHLD, "SIGCHLD", "", unix_signal_handler },

	{ SIGSYS, "SIGSYS, SIG_IGN", "", nullptr },

	{ SIGPIPE, "SIGPIPE, SIG_IGN", "", nullptr },

	{ 0, nullptr, "", nullptr }
};


pid_t unix_spawn_process(tag_cycle *cycle, spawn_proc_ptr proc, void *data, char *name, int re_spawn)
{
	// u_long		on;
	pid_t			pid;
	int				s;

	if (re_spawn >= 0) {
		s = re_spawn;
	}
	else {

		for (s = 0; s < unix_process_last; s++) {
			if (unix_processes[s].pid == -1) {
				break;
			}
		}

		if (s == PROCESSES_MAX) {
			
			std::string err = "no more than ";
			err += std::to_string(PROCESSES_MAX);
			err += " processes can be spawned";
			return UNIX_RET_F(err.data(), -1);
		}
	}

	if (re_spawn != PROCESS_DETACHED) {

		if (socketpair(AF_UNIX, SOCK_STREAM, 0, unix_processes[s].channel) == -1)
		{
			std::string err = std::to_string(errno);
			err += " socketpair() failed while spawning \"";
			err += name;
			err += "\"";
			return UNIX_RET_F(err.data(), -1);
		}

		unix_channel_fd = unix_processes[s].channel[1];
	}
	else {
		unix_processes[s].channel[0] = -1;
		unix_processes[s].channel[1] = -1;
	}

	unix_process_slot = s;

	pid = fork();

	switch (pid) 
	{
	case -1:	
	{
		unix_channel_close(unix_processes[s].channel);
		std::string err = u8R"(fork() failed while spawning \)";
		err += std::to_string(errno);
		err += name;
		err += "\"";
		return UNIX_RET_F(err.data(), -1);
	}
	case 0:
	{
		process_parent = process_pid;
		process_pid = getpid();
		proc(cycle, data);
		break;
	}

	default:
		break;
	}

	LOG_NOTICE(nullptr) << 0 << "start " << name << pid;

	unix_processes[s].pid = pid;
	unix_processes[s].exited = 0;

	if (re_spawn >= 0) {
		return pid;
	}

	unix_processes[s].proc = proc;
	unix_processes[s].data = data;
	unix_processes[s].name = name;
	unix_processes[s].exiting = 0;

	switch (re_spawn) {

	case PROCESS_NORESPAWN:
		unix_processes[s].re_spawn = 0;
		unix_processes[s].just_spawn = 0;
		unix_processes[s].detached = 0;
		break;

	case PROCESS_JUST_SPAWN:
		unix_processes[s].re_spawn = 0;
		unix_processes[s].just_spawn = 1;
		unix_processes[s].detached = 0;
		break;

	case PROCESS_RESPAWN:
		unix_processes[s].re_spawn = 1;
		unix_processes[s].just_spawn = 0;
		unix_processes[s].detached = 0;
		break;

	case PROCESS_JUST_RESPAWN:
		unix_processes[s].re_spawn = 1;
		unix_processes[s].just_spawn = 1;
		unix_processes[s].detached = 0;
		break;

	case PROCESS_DETACHED:
		unix_processes[s].re_spawn = 0;
		unix_processes[s].just_spawn = 0;
		unix_processes[s].detached = 1;
		break;
	}

	if (s == unix_process_last) {
		unix_process_last++;
	}

	return pid;
}

unsigned int unix_init_signals()
{
	tag_unix_process_signal	*sig;
	struct sigaction		 sa;

	for (sig = signals; sig->signo != 0; sig++) {

		memset(&sa, 0x0, sizeof(struct sigaction));

		if (sig->handler) {

			sa.sa_sigaction = sig->handler;
			sa.sa_flags = SA_SIGINFO;

		}
		else {
			sa.sa_handler = SIG_IGN;
		}

		sigemptyset(&sa.sa_mask);
		if (sigaction(sig->signo, &sa, nullptr) == -1) {
			
			std::string err = "sigaction(%s) failed ";
			err += sig->signame;
			err += std::to_string(errno);
			return UNIX_RET_F(err.data(), -1);
		}
	}

	return UNIX_RET_S(0);
}

static void unix_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{
	unsigned int				ignore;
	unsigned int				err;
	std::string					action;
	tag_unix_process_signal	   *sig;

	err = errno;
	for (sig = signals; sig->signo != 0; sig++) {
		if (sig->signo == signo) {
			break;
		}
	}

	LOG_WARN(nullptr) << "Signal pid(" + to_string(process_pid) + ")";
	LOG_WARN(nullptr) << "Signal signo(" + to_string(signo) + ")";
	LOG_WARN(nullptr) << "Signal process(" + to_string(process_type) + ")";

	switch (process_type)
	{
	case PROCESS_MASTER:
	case PROCESS_SINGLE:

		switch (signo)
		{
		case unix_signal_value(SIGNAL_SHUTDOWN):
			process_quit = 1;
			action += "shutting down";
			break;

		case unix_signal_value(SIGNAL_TERMINATE):
		case SIGINT:
			process_terminate = 1;
			action += "exiting";
			break;

		case unix_signal_value(SIGNAL_NOACCEPT):
			if (process_daemonized) {
				process_no_accept = 1;
				action += "stop accepting connections";
			}
			break;

		case unix_signal_value(SIGNAL_RECONFIGURE):
			process_reconfigure = 1;
			action += "reconfiguring";
			break;

		case unix_signal_value(SIGNAL_REOPEN):
			process_reopen = 1;
			action += "reopening logs";
			break;

		case unix_signal_value(SIGNAL_CHANGEBIN):
			if (getppid() == process_parent || process_new_binary > 0) {
				action += "ignoring";
				ignore = 1;
				break;
			}

			process_change_binary = 1;
			action += "changing binary";
			break;

		case SIGALRM:
			process_sigalrm = 1;
			break;

		case SIGIO:
			process_sigio = 1;
			break;

		case SIGCHLD:
			process_reap = 1;
			break;
		}
		break;

	case PROCESS_WORKER:
	case PROCESS_HELPER:

		switch (signo)
		{
		case unix_signal_value(SIGNAL_NOACCEPT):
			if (!process_daemonized) {
				break;
			}
			process_debug_quit = 1;

		case unix_signal_value(SIGNAL_SHUTDOWN):
			process_quit = 1;
			action += "shutting down";
			break;

		case unix_signal_value(SIGNAL_TERMINATE):
		case SIGINT:
			process_terminate = 1;
			action += "exiting";
			break;

		case unix_signal_value(SIGNAL_REOPEN):
			process_reopen = 1;
			action += "reopening logs";
			break;

		case unix_signal_value(SIGNAL_RECONFIGURE):
		case unix_signal_value(SIGNAL_CHANGEBIN):
		case SIGIO:
			action += "ignoring";
			break;
		}

		break;
	}

	LOG_WARN(nullptr) << "Signal received from" << siginfo->si_pid;
	LOG_WARN(nullptr) << "Signal action" << action;

	if (ignore) {
		LOG_WARN(nullptr) << "the changing binary signal is ignored : ";
		LOG_WARN(nullptr) << "you should shutdown or terminate ";
		LOG_WARN(nullptr) << "before either old or new binary's process";
	}

	if (signo == SIGCHLD) {
		unix_process_get_status();
	}

	errno = err;
}

static void unix_process_get_status(void)
{
	int				status;
	pid_t			pid;
	int				err;
	int				i;
	unsigned int	one;
	char		   *process;

	for (;; ) {

		pid = waitpid(-1, &status, WNOHANG);

		if (pid == 0) {
			return;
		}

		if (pid == -1) {
			err = errno;

			if (err == EINTR) {
				continue;
			}

			if (err == ECHILD && one) {
				return;
			}

			if (err == ECHILD) {
				return;
			}

			return;
		}

		one = 1;
		process = (char *)"unknown process";

		for (i = 0; i < unix_process_last; i++) {

			if (unix_processes[i].pid == pid) {
				unix_processes[i].status = status;
				unix_processes[i].exited = 1;
				process = unix_processes[i].name;
				break;
			}
		}

		if (WTERMSIG(status)) {
			UNIX_RET_F("exited on signal ", pid);
		}
		else {
			UNIX_RET_F("exited on signal ", pid);
		}

		if (WEXITSTATUS(status) == 2 && unix_processes[i].re_spawn) {
			unix_processes[i].re_spawn = 0;
		}
	}

	UNIX_TRACK(process, pid);
}

int unix_signal_process(tag_cycle *cycle, char *name, pid_t pid)
{
	tag_unix_process_signal  *sig;

	for (sig = signals; sig->signo != 0; sig++) {

		if (strcmp(name, sig->name) == 0) {

			if (kill(pid, sig->signo) != -1) {
				return 0;
			}

			return UNIX_RET_F("kill() failed", pid);
		}
	}
	return 1;
}

static void unix_execute_proc(tag_cycle *_cycle, void *_data)
{
	tag_unix_exec  *_ctx = (tag_unix_exec *)_data;

	if (execve(_ctx->path, _ctx->argv, _ctx->envp) == -1) {
		UNIX_RET_F("execve() failed while executing",-1);
	}

	exit(1);
}

pid_t unix_execute(tag_cycle *cycle, tag_unix_exec *ctx)
{
	return unix_spawn_process(cycle, unix_execute_proc, 
		ctx, ctx->name, PROCESS_DETACHED);
}

