/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   30, Feb, 2021
 */

#include <atomic>
#include <string>
#include <vector>
#include <string.h>
#include "files_tools.h"


 /* files_lock */
class files_lock
{
public:
	std::string		file_path;

public:
	files_lock() = delete;
	files_lock(const char * fp);
	virtual ~files_lock();
};


#if (!WINX)
#include <sys/file.h>
#endif


files_lock::files_lock(const char * fp)
{
#if (!WINX)
	file_path = fp;
	
	/*
	if (0 == flock(fileno(fp), LOCK_EX)) {
	
	} */
#endif
}


files_lock::~files_lock()
{
#if (!WINX)
	/*
	if (0 == flock(fileno(file_path.data()), LOCK_UN)) {
	
	} */
#endif
}