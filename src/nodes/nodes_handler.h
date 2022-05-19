/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#pragma once
#include "nodes.h"

 /* nodes_handler_authed_check */
int nodes_handler_authed_check(tag_nodes_task * task);

/* nodes_handler_heartbeat */
int nodes_handler_heartbeat(tag_nodes_task * task);

/* nodes_handler_heartbeat */
int nodes_handler_pull_task(tag_nodes_task * task);

 /* nodes_handler_pulls_process */
int nodes_handler_pulls_process(tag_nodes_task * task);

 /* nodes_handler_logs_process */
int nodes_handler_statlogs_process(tag_nodes_task * task);

/* nodes_handler_stat_process */
int nodes_handler_statload_process(tag_nodes_task * task);
