/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#pragma once
#include <core_config.h>
#include "nodes.h"

 /* nodes_test_commandline */
void nodes_test_commandline(tag_cycle * cycle);

 /* nodes_test_authlogin */
void nodes_test_authlogin(tag_nodes_task * task);

/* nodes_test_heartbeat */
void nodes_test_heartbeat(tag_nodes_task * task);

/* nodes_test_pulltask */
void nodes_test_pulltask(tag_nodes_task * task);

/* nodes_test_logs */
void nodes_test_statlogs(tag_nodes_task * task);

/* nodes_test_stat */
void nodes_test_statload(tag_nodes_task * task);

/* print ./asnode/net; (nodes_stream_trace | nodes_stream_error | nodes_stream_debug) */
// void nodes_test_inetlogs(tag_nodes_task * task);

/* print ./asnode/logs; (-e | -d | -i | -t) */
void nodes_test_nodeslogs(tag_nodes_task * task);

/* print /cmdline/defense; */
void nodes_test_defense(tag_nodes_task * task);

/* nodes_test_stop_asnode */
void nodes_test_stop_asnode(tag_nodes_task * task);