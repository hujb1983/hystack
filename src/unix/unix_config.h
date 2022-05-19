/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   27, SEP, 2021
 */
#pragma once

 /* unix_config.h */
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>  
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <glob.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <libgen.h>

#include <core_config.h>
#include <event/event.h>
#include <process/process.h>
#include "unix_exception.h"

#if (WINX)
typedef unsigned int pid_t;
#endif

#define UNIX_INVALID_PID	-1

/* doemom for proc */
unsigned int unix_daemon();
unsigned int unix_init_signals();

/* tag_unix_process */
typedef void(*spawn_proc_ptr) (tag_cycle * cycle, void *data);

typedef struct {
	pid_t				pid;
	int                 status;
	int					channel[2];

	spawn_proc_ptr		proc;
	void               *data;
	char               *name;

	unsigned            re_spawn : 1;
	unsigned            just_spawn : 1;
	unsigned            detached : 1;
	unsigned            exiting : 1;
	unsigned            exited : 1;

} tag_unix_process;


typedef struct {
	char			   *path;
	char			   *name;
	char *const		   *argv;
	char *const		   *envp;
} tag_unix_exec;


/* unix channel */
struct tag_channel
{
	unsigned int	    command;
	pid_t			    pid;
	int				    slot;
	int				    fd;
};


/* unix_channel_write */
int unix_channel_write(int s, tag_channel *ch, unsigned int size);

/* unix_channel_read */
int unix_channel_read(int s, tag_channel *ch, unsigned int size);

/* unix_channel_event */
int unix_channel_add_event(tag_cycle *cycle, int fd, int event, events_handler_ptr handler);

/* unix_channel_event */
int unix_channel_close(int * fd);

/* unix_spawn_process */
int unix_signal_process(tag_cycle *cycle, char *name, pid_t pid);

/* unix_spawn_process */
pid_t unix_spawn_process(tag_cycle *cycle, spawn_proc_ptr proc, void *data, char *name, int respawn);

/* unix_proctitle_init */
int unix_proctitle_init();
int unix_set_proctitle(const char * title);

/* unix_channel or unix_process */
extern int				unix_channel_fd;
extern int				unix_process_slot;
extern int				unix_process_last;
extern tag_unix_process	unix_processes[PROCESSES_MAX];

#define HAVE_MSGHDR_MSG_CONTROL 1
