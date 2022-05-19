/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once
#include <mutex>
#include <core_config.h>

 /* const_logs_page_size */
const unsigned int const_logs_page_size = (1024 * 8L);

/* logs_handler_ptr */
typedef int(*logs_handler_ptr) (tag_log *log, int level, u_char * data, u_short size);

/* logs_writer_ptr */
typedef int(*logs_writer_ptr) (tag_log *log, int level, u_char * buff, u_short size);

/* ST_LOG */
struct ST_LOG
{
	int					fd;

	int					level;
	std::string			nickname;

	long long			file_size;
	std::string			file_name;
	std::string			file_path;

	logs_handler_ptr	ptr_handler;
	void               *buffer;			// pointer a core-module;	
	unsigned int        buffered;		// buffered;		

	logs_writer_ptr		ptr_writer;
	void			   *data;			// pointer a buffer;

	std::string			action;
	std::mutex			mutex;

	tag_log            *ptr_next;
};
