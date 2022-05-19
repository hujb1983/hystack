/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once

#include <string>
#include <core.h>
#include <core_cycle.h>

#include <module/modules.h>
#include <module/modules_cycle.h>

#if (WINX)
typedef unsigned long	pid_t;
#endif

/* process */
extern unsigned int	process_type;
extern unsigned int	process_worker;
extern unsigned int	process_inherited;
extern unsigned int	process_daemonized;

/* process atomic */
extern sig_atomic_t	process_reap;
extern sig_atomic_t	process_sigio;
extern sig_atomic_t	process_sigalrm;
extern sig_atomic_t	process_terminate;
extern sig_atomic_t	process_quit;
extern sig_atomic_t	process_debug_quit;
extern sig_atomic_t	process_exiting;
extern sig_atomic_t	process_reconfigure;
extern sig_atomic_t	process_reopen;
extern sig_atomic_t	process_change_binary;

/* process_fork_safe */
double process_fork_safe(const char * cmd, std::string & result);

/* process_option_parse */
int process_option_parse(int argc, char *const *argv);

/* process_option_cmdline_ptr */
typedef void(*process_option_cmdline_ptr)(tag_cycle * cycle);
extern process_option_cmdline_ptr top_process_option_cmdline;

/* process */
class process_mgr
{
public:
	process_mgr();
	virtual ~process_mgr();

public:
	static int init(tag_cycle * cycle);
	static int sleep(unsigned long _milliseconds);
	static int execute(tag_cycle *cycle, tag_core_exec *ctx);

public:
	static int dump(tag_cycle * cycle);
	static int run(tag_cycle *cycle);
	static int set_current_cycle(core_cycle *cycle);
};

