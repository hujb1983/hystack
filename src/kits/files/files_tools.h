/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   30, Feb, 2021
 */

#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <string.h>


/* tag_file_path */
typedef struct ST_FILE_PATH	tag_file_path;


/* ST_FILE_PATH */
struct ST_FILE_PATH
{
#if (WINX)
	FILE		   *file;
#else
	int				file;
#endif

	std::string		path;
};


