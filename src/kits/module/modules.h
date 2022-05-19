/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once
#include <core_config.h>
#include <core_string.h>
#include <logs/logs.h>

 /* windows */
#if (WINX)
#include <windows.h>
#endif

 /* tag_module */
struct ST_MODULE
{
public:
	unsigned int		ctx_index;
	unsigned int		index;

	char			   *name;

	unsigned int		spare0;
	unsigned int		spare1;

	unsigned int		version;
	const char		   *signature;

	void 			   *context;
	tag_command		   *commands;
	unsigned int		type;

	int(*init_master)(tag_cycle *log);
	int(*init_module)(tag_cycle *cycle);
	int(*init_process)(tag_cycle *cycle);
	int(*init_thread)(tag_cycle *cycle);

	void(*exit_thread)(tag_cycle *cycle);
	void(*exit_process)(tag_cycle *cycle);
	void(*exit_master)(tag_cycle *cycle);

	uintptr_t			spare_hook0;
	uintptr_t			spare_hook1;
	uintptr_t			spare_hook2;
	uintptr_t			spare_hook3;
	uintptr_t			spare_hook4;
	uintptr_t			spare_hook5;
	uintptr_t			spare_hook6;
	uintptr_t			spare_hook7;
};


/* tag_core_module */
struct tag_core_module
{
	tag_string		   name;
	void			*(*create_conf)(tag_cycle *cycle);
	void			*(*init_conf)(tag_cycle *cycle, void *conf);
};


/* modules_conf */
class modules_conf
{
public:
	modules_conf();
	~modules_conf();

public:
	static tag_cycle * copy(tag_cycle *cycle);

public:
	static int init(tag_cycle *cycle);
	static int count(tag_cycle *cycle, unsigned int type);
	static int ctx_index(tag_cycle * cycle, unsigned int type, unsigned int index);
};

