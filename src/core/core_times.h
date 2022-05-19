/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#pragma once
#include <time.h>
#include <string>

#include "core_config.h"
#include "core_string.h"

 /* TIME_LEN */
#if (WINX)

#define PTR_SIZE				8
#define SIZE_LEN				(sizeof("-9223372036854775808") - 1)
#define MAX_SIZE_VALUE			9223372036854775807
#define TIME_LEN				(sizeof("-9223372036854775808") - 1)
#define TIME_SIZE				8
#define MAX_TIME_VALUE			9223372036854775807

#else

#define PTR_SIZE				4
#define SIZE_LEN				(sizeof("-2147483648") - 1)
#define MAX_SIZE_VALUE			2147483647
#define TIME_LEN				(sizeof("-2147483648") - 1)
#define TIME_SIZE				4
#define MAX_TIME_VALUE			2147483647

#endif


/* tag_core_times */
typedef struct {
	time_t			sec;
	unsigned int	msec;
	int				gmtoff;
} tag_core_times;

/* tag_msec */
typedef unsigned long long tag_msec;

/* core_clock */
class core_clock
{
public:
	unsigned long long  begin_time;
	unsigned long long  current_time;
	unsigned int		microsecond;
	unsigned int		seconds;		// seconds after the minute - [0, 60] including leap second
	unsigned int		minutes;		// minutes after the hour - [0, 59]
	unsigned int		hour;			// hours since midnight - [0, 23]
	unsigned int		mday;			// day of the month - [1, 31]
	unsigned int		months;			// months since January - [0, 11]
	unsigned int		years;			// years since 1900
	unsigned int		wday;			// days since Sunday - [0, 6]
	unsigned int		yday;			// days since January 1 - [0, 365]
	unsigned int		isdst;			// daylight savings time flag

public:
	core_clock();
	~core_clock();

public:
	void update();

public:
	unsigned long long	get_tick_count();
	std::string			get_system_time();
};


/* core_times */
class core_times 
{
public:
	core_times();
	~core_times();

public:
	static void init(void);
	static void update(void);
};

extern unsigned long long		core_current_msec;
extern tag_core_times		   *core_cached_time;
extern tag_string				core_cached_http_time;
extern tag_string				core_cached_errorlog_time;
extern tag_string				core_cmdline_errorlog_time;
extern tag_string				core_rewrite_errorlog_time;
extern tag_string				core_collect_sysinfo_time;

#define core_time()				core_cached_time->sec
#define core_time_of_day()		core_cached_time;
