/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <time.h>
#include "process_mgr.h"


 /* start process */
class process_popen
{
public:
	FILE	*fp;

public:
	process_popen() = delete;
	process_popen(const std::string & cmd, std::string & result);
	virtual ~process_popen();
};


/* process_popen::process_popen */
process_popen::process_popen(const std::string & cmd, std::string & result)
{
	fp = nullptr;

#if (!WINX)
	char buff[1024] = { 0 };
	result.clear();

	if ((fp = ::popen(cmd.data(), "r")) != nullptr)
	{
		memset(buff, 0, 1024);

		while (fgets(buff, 1024, fp) != nullptr) {
			result += buff;
		}
	}
#endif
}


/* process_popen::~process_popen */
process_popen::~process_popen()
{
#if (!WINX)
	if (fp != nullptr) {
		::pclose(fp);
	}
#endif
}

/* process_fork_safe*/
double process_fork_safe(const char * cmd, std::string & result)
{
	time_t c_start;
	time_t c_end;

	/* process_popen */

	c_start = clock();
	process_popen popen(cmd, result);
	c_end = clock();

	return difftime(c_end, c_start);
}