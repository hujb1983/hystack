/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   30, Feb, 2021
 */

#include <atomic>
#include <string>
#include <vector>
#include <string.h>

 /* path_string */
typedef struct ST_FILES_TABLE tag_files_table;


 /* path_string */
using VEC_FILES = std::vector<tag_files_table>;


/* path_string */
struct ST_FILES_TABLE
{
	std::string		file_path;
	std::string		file_name;
	std::string		file_type;
};


 /* files_path_check */
class files_table
{
public:
public:
	files_table();
	virtual ~files_table();

private:
	unsigned char	check(tag_files_table *);
	unsigned char	create(tag_files_table *);
};


/* files_table::files_table */
files_table::files_table()
{

}

/* files_table::files_table */
files_table::~files_table()
{

}