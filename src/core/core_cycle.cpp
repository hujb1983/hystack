/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "core_config.h"
#include "core_cycle.h"
#include "core_times.h"

#include <thread>
#include <chrono>
#include <functional>
#include <string>

#include <logs/logs.h>
#include <event/event.h>
#include <event/event_connection.h>

#include <module/modules_core.h>
#include <module/modules_conf.h>
#include <module/modules_config.h>

#include <process/process_mgr.h>
#include <process/process.h>

using namespace std;
using namespace chrono;

tag_cycle			*root_cycle;
vector<tag_cycle*>  *old_cycle;

extern tag_module	 core_module;
static tag_event     shutdown_event;

unsigned int		 core_process;
unsigned int		 core_worker;

extern int			 process_param_argc;
extern char		   **process_param_argv;


core_cycle::core_cycle() {

}

core_cycle::~core_cycle() {

}

int core_cycle::run(tag_cycle *cycle)
{
	return 0;
}

int core_cycle::signal_process(tag_cycle *cycle, char * signo)
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
		LOG_ERROR(nullptr) << "invalid pid number " << n << " in " << (char *)buf << ".";
		return 1;
	}

	return unix_signal_process(cycle, signo, pid);
#else
	return 0;
#endif

}

int core_cycle::single_process(tag_cycle *cycle)
{
	return 0;
}

int core_cycle::master_process(tag_cycle *cycle)
{
	return 0;
}

int core_cycle::master_process_exit(tag_cycle *cycle)
{
	return 0;
}

void core_cycle::destory(tag_cycle * cycle) {

}

static void core_shutdown_timer_handler(tag_event *ev)
{
	unsigned int	 i;
	tag_connection	*c;
	tag_cycle		*cycle;

	cycle = (tag_cycle *)ev->data;

	c = cycle->conn_ptr;
	for (i = 0; i < cycle->conn_size; i++) {

		if (c[i].fd == -1
			|| c[i].read == nullptr
			|| c[i].read->accept
			|| c[i].read->channel
			|| c[i].read->resolver) {

			continue;
		}

		LOG_DEBUG(nullptr) << c[i].number << "shutdown timeout";

		c[i].close = 1;
		c[i].error = 1;
		if (c[i].read->ptr_handler != nullptr) {
			c[i].read->ptr_handler(c[i].read);
		}
	}
}

void core_cycle::set_shutdown_timer(tag_cycle * cycle)
{
	tag_core_conf  *ccf;

	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	if (ccf->shutdown_timeout) {
		shutdown_event.ptr_handler = core_shutdown_timer_handler;
		shutdown_event.data = cycle;
		shutdown_event.cancelable = 1;
		// addTimer(&shutdown_event, ccf->shutdown_timeout);
	}
}

void core_cycle::reopen_files(tag_cycle *cycle, unsigned long user) {

}


void core_cycle::signal_worker_process(tag_cycle *cycle, int signo)
{
#if (!WINX)

	int				i;
	int				err;
	tag_channel		ch;

	switch (signo) {

	case unix_signal_value(SIGNAL_SHUTDOWN):
		ch.command = CMD_QUIT;
		break;

	case unix_signal_value(SIGNAL_TERMINATE):
		ch.command = CMD_TERMINATE;
		break;

	case unix_signal_value(SIGNAL_REOPEN):
		ch.command = CMD_REOPEN;
		break;

	default:
		ch.command = 0;
	}

	ch.fd = -1;

	for (i = 0; i < unix_process_last; i++) {

		if (unix_processes[i].detached || unix_processes[i].pid == -1) {
			continue;
		}

		if (unix_processes[i].just_spawn) {
			unix_processes[i].just_spawn = 0;
			continue;
		}

		if (unix_processes[i].exiting && signo == unix_signal_value(SIGNAL_SHUTDOWN)) {
			continue;
		}

		if (ch.command) {
			if (unix_channel_write( unix_processes[i].channel[0], &ch, sizeof(tag_channel)) == 0)
			{
				if (signo != unix_signal_value(SIGNAL_REOPEN)) {
					unix_processes[i].exiting = 1;
				}
				continue;
			}
		}

		if (kill(unix_processes[i].pid, signo) == -1) {

			err = errno;

			if (err == ESRCH) {
				unix_processes[i].exited = 1;
				unix_processes[i].exiting = 0;
				process_reap = 1;
			}
			continue;
		}

		if (signo != unix_signal_value(SIGNAL_REOPEN)) {
			unix_processes[i].exiting = 1;
		}
	}

#endif
}


void core_cycle::unix_pass_open_channel(tag_cycle *cycle)
{
#if (!WINX)

	int				i;
	tag_channel		ch;

	memset(&ch, 0x0, sizeof(tag_channel));

	ch.command = CMD_OPEN_CHANNEL;
	ch.pid = unix_processes[unix_process_slot].pid;
	ch.slot = unix_process_slot;
	ch.fd = unix_processes[unix_process_slot].channel[0];

	for (i = 0; i < unix_process_last; i++) 
	{
		if (i == unix_process_slot || 
			unix_processes[i].pid == -1 || 
			unix_processes[i].channel[0] == -1)
		{
			continue;
		}
		
		LOG_DEBUG(nullptr) << "pass channel s:" << ch.slot
			<< " pid:" << ch.pid
			<< " fd:" << ch.fd
			<< " s:" << i
			<< " pid:" << unix_processes[i].pid
			<< " fd:" << unix_processes[i].channel[0];

		unix_channel_write(unix_processes[i].channel[0], &ch, sizeof(tag_channel));
	}
#endif
}

int core_cycle::unix_reap_children(tag_cycle *cycle)
{
#if (!WINX)
	int					i, n;
	unsigned int		live;
	tag_channel			ch;
	tag_core_conf	   *ccf;

	memset(&ch, 0x0, sizeof(tag_channel));

	ch.command = CMD_CLOSE_CHANNEL;
	ch.fd = -1;

	live = 0;
	for (i = 0; i < unix_process_last; i++) {

		if (unix_processes[i].pid == -1) {
			continue;
		}

		if (unix_processes[i].exited) {

			if (!unix_processes[i].detached) 
			{
				unix_channel_close(unix_processes[i].channel);

				unix_processes[i].channel[0] = -1;
				unix_processes[i].channel[1] = -1;

				ch.pid = unix_processes[i].pid;
				ch.slot = i;

				for (n = 0; n < unix_process_last; n++) 
				{
					if (unix_processes[n].exited || unix_processes[n].pid == -1 || unix_processes[n].channel[0] == -1) {
						continue;
					}

					unix_channel_write(unix_processes[n].channel[0], &ch, sizeof(tag_channel));
				}
			}

			if (unix_processes[i].re_spawn && !unix_processes[i].exiting && !process_terminate && !process_quit) {

				if (unix_spawn_process(cycle, unix_processes[i].proc, 
					unix_processes[i].data, unix_processes[i].name, i) == UNIX_INVALID_PID) {

					/*
					LOG_ALERT(cycle->log) << "could not respawn %s"
						<< unix_processes[i].name; */

					continue;
				}

				unix_pass_open_channel(cycle);

				live = 1;
				continue;
			}

			if (unix_processes[i].pid == process_new_binary) {
				ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);
				if (rename((char *)ccf->old_pid.data(), (char *)ccf->pid.data())) {

					/*
					LOG_ALERT(cycle->log) << errno
						<< "rename_n " << ccf->oldpid
						<< " back to " << ccf->pid << "failed "
						<< " after the new binary process " << AsArgv[0] << " exited"; */
				}

				process_new_binary = 0;
				if (process_no_accepting) {
					process_restart = 1;
					process_no_accepting = 0;
				}
			}

			if (i == unix_process_last - 1) {
				unix_process_last--;
			}
			else {
				unix_processes[i].pid = -1;
			}
		}
		else if (unix_processes[i].exiting || !unix_processes[i].detached) {
			live = 1;
		}
	}

	return live;
#else
	return 0;
#endif
}