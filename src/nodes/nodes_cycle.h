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


/* nodes_cycle */
class nodes_cycle : public core_cycle
{
public:
	nodes_cycle();
	~nodes_cycle();

public:
	virtual int	run(tag_cycle *cycle);

public:
	virtual int	signal_process(tag_cycle *cycle, char * signo);
	virtual int	single_process(tag_cycle *cycle);
	virtual int	master_process(tag_cycle *cycle);
	virtual int	master_process_exit(tag_cycle *cycle);

private:
	int			nodes_worker_param_init(nodes_cycle *core, tag_cycle *cycle);
	void		nodes_start_worker_process(tag_cycle *cycle, int n, int type);
	int			nodes_single_process(nodes_cycle *core, tag_cycle *cycle);

private:
	static void	nodes_worker_process_exit(tag_cycle *cycle);
	static void	nodes_worker_process_init(tag_cycle *cycle, int worker);
	static void nodes_pulls_process_cycle(tag_cycle *cycle, void *data);
	static void	nodes_stat_process_cycle(tag_cycle *cycle, void *data);
	static void	nodes_logs_process_cycle(tag_cycle *cycle, void *data);
	static void	nodes_sleep_process_cycle(unsigned int times);
};
