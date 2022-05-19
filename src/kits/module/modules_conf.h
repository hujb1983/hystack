/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <vector>
#include <fstream>
using namespace std;

#include <module/modules.h>
#include <alloc/alloc_pool.h>


#define CONF_BLOCK_START	1
#define CONF_BLOCK_DONE		2
#define CONF_FILE_DONE		3

#define MODULE_CORE			0x45524F43  /* "CORE" */
#define MODULE_CONF			0x464E4F43  /* "CONF" */

#define CONF_ARGS_NUMBER	0x000000ff
#define CONF_BLOCK			0x00000100
#define CONF_FLAG			0x00000200
#define CONF_ANY			0x00000400
#define CONF_1MORE			0x00000800
#define CONF_2MORE			0x00001000

#define CONF_NOARGS			0x00000001
#define CONF_TAKE1			0x00000002
#define CONF_TAKE2			0x00000004
#define CONF_TAKE3			0x00000008
#define CONF_TAKE4			0x00000010
#define CONF_TAKE5			0x00000020
#define CONF_TAKE6			0x00000040
#define CONF_TAKE7			0x00000080

#define CONF_MAX_ARGS		8

#define CONF_TAKE12			(CONF_TAKE1| CONF_TAKE2)
#define CONF_TAKE13			(CONF_TAKE1| CONF_TAKE3)

#define CMD_DIRECT_CONF		0x00010000
#define CMD_MAIN_CONF		0x01000000
#define CMD_ANY_CONF		0xFF000000

typedef struct ST_CONF		tag_conf;
typedef struct ST_COMMAND	tag_command;

/* ST_CONF */
struct ST_CONF
{
	char				 name;
	void				*ctx;

	unsigned int		 module_type;
	unsigned int		 cmd_type;

	std::string			 conf_file;
	std::ifstream		 conf_ptr;

	std::vector<string>  args;
	std::string			 line;

	tag_pool			*pool;
	tag_cycle			*cycle;

	void				*context;
};


#define CMD_CONF_UNSET			(0xFFFFFFFF)
#define CMD_CONF_UNSET_UINT		(0xFFFFFFFF)
#define CMD_CONF_UNSET_PTR		((void *)-1)
#define CMD_CONF_UNSET_SIZE		(0xFFFFFFFF)
#define CMD_CONF_UNSET_MSEC		(0xFFFFFFFFFFFFFFFF)


struct ST_COMMAND
{
	tag_string 			name;
	unsigned long		type;
	char               *(*ptr_set)(tag_conf *cf, tag_command *cmd, void *conf);

	unsigned int		conf;
	unsigned int		offset;
	void			   *post;
};

#define init_command_null			{ init_string_null, 0, nullptr, 0, 0, nullptr }


#define MODULES_CONF_UNSET			(0xFFFFFFFF)
#define MODULES_CONF_UNSET_UINT		(0xFFFFFFFF)
#define MODULES_CONF_UNSET_PTR		((void *)-1)
#define MODULES_CONF_UNSET_SIZE		(0xFFFFFFFF)
#define MODULES_CONF_UNSET_MSEC		(0xFFFFFFFFFFFFFFFF)

/* cycle->conf_ctx */
#define modules_get_conf(conf_ctx, modules)		\
		conf_ctx[modules.index]


/* modules_parse */
class modules_parse
{
public:

	enum {
		emParseFile = 0,
		emParseBlock,
		emParseParam
	};

public:
	static char* include_file(tag_conf * cf);
	static char* include_param(tag_conf * cf);

private:
	static int read_token(tag_conf * conf);
	static int handler(tag_conf *cf, int last);

public:
	static char* flag(tag_conf *cf, tag_command *cmd, void *conf);
	static char* msec(tag_conf *cf, tag_command *cmd, void *conf);
	static char* string(tag_conf *cf, tag_command *cmd, void *conf);
	static char* number(tag_conf *cf, tag_command *cmd, void *conf);
};


/* ptr_conf_post_handler */
typedef char *(*ptr_conf_post_handler)(tag_conf *cf,
	void *data, void *conf);


/* tag_conf_post */
typedef struct {
	ptr_conf_post_handler  post_handler;
} tag_conf_post;
