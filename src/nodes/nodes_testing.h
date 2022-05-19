 /*
  * Copyright (C) As Cloud, Inc.
  * Data     :   01, Feb, 2021
  */

#pragma once
#include <core_config.h>

/* nodes_testing_nginx_process */
int nodes_testing_nginx_process(tag_nodes_task * task);

/* nodes_testing_cache_process */
int nodes_testing_cache_process(tag_nodes_task * task);

/* nodes_testing_cache_process */
int nodes_testing_redis_process(tag_nodes_task * task);

/* nodes_testing_ssl_process */
int nodes_testing_ssl_process(tag_nodes_task * task);

/* nodes_testing_lua_process */
int nodes_testing_lua_process(tag_nodes_task * task);

/* nodes_testing_step_process */
int nodes_testing_step_process(tag_nodes_task * task);

/* nodes_testing_errpage_process */
int nodes_testing_errpage_process(tag_nodes_task * task);

/* nodes_testing_http_process */
int nodes_testing_http_process(tag_nodes_task * task);

/* nodes_testing_defense_process */
int nodes_testing_defense_process(tag_nodes_task * task);
