/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <vector>

/* tag_core_conf */
typedef struct {

	unsigned int				daemon;
	unsigned int				master;

	unsigned long long			timer_resolution;
	unsigned long long			shutdown_timeout;

	unsigned int				worker_processes;
	std::string					working_directory;

	std::string					pid;
	std::string					old_pid;
	int							priority;

	std::string					username;

#if (!WINX)
	uid_t						user;
	gid_t						group;
#else 
	unsigned long				user;
	unsigned long				group;
#endif

	std::vector<std::string>	env;
	char					  **environment;
	std::string					prefix_path;

} tag_core_conf;

