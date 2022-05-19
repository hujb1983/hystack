/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <core_config.h>
#include "nodes.h"

/* nodes_process_popen */
int	nodes_process_popen(const std::string & cmd, std::string & output);

/* nodes_process_test_nginx */
int	nodes_process_test_nginx(tag_nodes_task * task);

/* nodes_process_check_nginx */
int	nodes_process_check_nginx(tag_nodes_task * task);

/* nodes_process_start_nginx */
int	nodes_process_start_nginx(tag_nodes_task * task);

/* nodes_process_stop_nginx */
int	nodes_process_stop_nginx(tag_nodes_task * task);

/* nodes_process_reload_nginx */
int	nodes_process_reload_nginx(tag_nodes_task * task);

/* nodes_nginx_unix_socket */
int nodes_nginx_unix_socket(tag_nodes_task * task);

/* nodes_nginx_current_path */
int nodes_nginx_current_path(const char *p, char *s, int len);

/* nodes_nginx_backup_file */
char * nodes_nginx_backup_file(tag_nodes_task * task);

/* nodes_nginx_update_file */
int nodes_nginx_update_file(tag_nodes_task * task);

/* nodes_nginx_remove_file */
int nodes_nginx_remove_file(tag_nodes_task * task);

/* nodes_nginx_delete_file */
int nodes_nginx_delete_file(tag_nodes_task * task);

/* nodes_nginx_delete_dir */
int nodes_nginx_delete_dir(const char * path);

/* nodes_nginx_remove_domain */
int nodes_nginx_remove_domain(tag_nodes_task * task);

/* nodes_nginx_update_domain */
int nodes_nginx_update_domain(tag_nodes_task * task);

/* nodes_cmdline_remove_nodes */
int nodes_cmdline_remove_nodes(tag_nodes_task * task);
