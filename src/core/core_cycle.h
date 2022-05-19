/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once
#include "core_config.h"

#include <string>
#include <vector>
#include <queue>

#include <logs/logs.h>
#include <alloc/alloc_pool.h>
#include <module/modules.h>


#define CMD_OPEN_CHANNEL		1
#define CMD_CLOSE_CHANNEL		2
#define CMD_QUIT				3
#define CMD_TERMINATE			4
#define CMD_REOPEN				5


/* core_cycly */
class core_cycle 
{
public:
					core_cycle();
	virtual			~core_cycle();

public:
	virtual int		run(tag_cycle *cycle);
	void			destory(tag_cycle * cycle);

public:
	virtual int		signal_process(tag_cycle *cycle, char * signo);
	virtual int		single_process(tag_cycle *cycle);
	virtual int		master_process(tag_cycle *cycle);
	virtual int		master_process_exit(tag_cycle *cycle);

public:
	virtual void	signal_worker_process(tag_cycle *cycle, int signo);

public:
	static void		set_shutdown_timer(tag_cycle * cycle);
	static void		reopen_files(tag_cycle *cycle, unsigned long user);

public:
	static void		unix_pass_open_channel(tag_cycle *cycle);
	static int		unix_reap_children(tag_cycle *cycle);
};
