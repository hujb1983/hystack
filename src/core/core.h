/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once
#include "core_config.h"

#include <string>
#include <vector>
using namespace std;

#define AS_USER		"root"
#define AS_GROUP	"root"

typedef struct {
	std::string   path;
	std::string   name;
	char *const  *argv;
	char *const  *envp;
} tag_core_exec;

/* core */
class core
{
public:
	core();
	~core();

public:
	static int create_pid(const std::string & name);
	static int delete_pid(tag_cycle *cycle);

public:
	static int		exec_new_binary(tag_cycle *cycle, char *const *argv);
	static char**	set_environment(tag_cycle * cycle, unsigned int *last);
};


extern unsigned int	core_process;
extern unsigned int	core_worker;
