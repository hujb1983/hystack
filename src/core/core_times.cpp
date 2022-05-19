/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include "core_times.h"
#include <string>
#include <string.h>
#include <chrono>

using namespace std;
using namespace chrono;

#define core_time()				core_cached_time->sec
#define core_time_of_day()		core_cached_time;

int							    slot;
unsigned long long			    core_current_msec;
tag_core_times				   *core_cached_time;
tag_string					    core_cached_errorlog_time;
tag_string					    core_cmdline_errorlog_time;
tag_string					    core_rewrite_errorlog_time;
tag_string					    core_collect_sysinfo_time;

const unsigned int				CORE_TIME_SLOTS = 65L;

static tag_core_times			static_core_cached_time[CORE_TIME_SLOTS];

static unsigned char			static_core_cached_errorlog_time[CORE_TIME_SLOTS]
								[sizeof("1970/09/28 12:00:00")];

static unsigned char			static_core_cmdline_errorlog_time[CORE_TIME_SLOTS]
								[sizeof("1970/09/28 12:00:00.000")];

static unsigned char			static_core_rewrite_errorlog_time[CORE_TIME_SLOTS]
								[sizeof("19700928120000")];

static unsigned char			static_core_collect_sysinfo_time[CORE_TIME_SLOTS]
								[sizeof("1970/09/28 12:00:00")];


core_clock::core_clock() 
{
	update();
	current_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	begin_time = current_time;
}

core_clock::~core_clock() {

}

void core_clock::update()
{
	struct tm	*ptm;
	auto tt = system_clock::to_time_t(system_clock::now());

	ptm = localtime(&tt);
	this->microsecond = 0;
	this->seconds = (unsigned int)ptm->tm_sec;
	this->minutes = (unsigned int)ptm->tm_min;
	this->hour = (unsigned int)ptm->tm_hour;
	this->mday = (unsigned int)ptm->tm_mday;
	this->months = (unsigned int)ptm->tm_mon;
	this->years = (unsigned int)ptm->tm_year + 1900;
	this->wday = (unsigned int)ptm->tm_wday;
	this->yday = (unsigned int)ptm->tm_wday;
	this->isdst = (unsigned int)ptm->tm_wday;
}

unsigned long long core_clock::get_tick_count() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string core_clock::get_system_time()
{
	char	date[60] = { 0 };
	snprintf(date, sizeof(date), "%d-%02d-%02d-%02d.%02d.%02d",
		(int)this->years + 1900, (int)this->months + 1, (int)this->mday,
		(int)this->hour, (int)this->minutes, (int)this->seconds);

	return std::string(date);
}


core_times::core_times() {

}

core_times::~core_times() {

}

void core_times::init(void)
{
	slot = 0;

	core_cached_errorlog_time.len = sizeof("1970/09/28 12:00:00") - 1;
	core_cmdline_errorlog_time.len = sizeof("1970/09/28 12:00:00") - 1;
	core_rewrite_errorlog_time.len = sizeof("19700928120000") - 1;
	core_collect_sysinfo_time.len = sizeof("1970/09/28 12:00:00") - 1;

	core_cached_time = &static_core_cached_time[0];

	update();
}

void core_times::update(void)
{
	unsigned char	*p1, *p2, *p3, *p4;
	tag_core_times	*tp;

	core_clock c;
	core_current_msec = c.get_tick_count();

	tp = &static_core_cached_time[slot];
	tp->sec = core_current_msec / 1000;
	tp->msec = core_current_msec % 1000;

	p1 = static_core_cached_errorlog_time[slot];
	sprintf((char *)p1, "%4d/%02d/%02d %02d:%02d:%02d",
		c.years, c.months + 1,
		c.mday, c.hour,
		c.minutes, c.seconds);
	
	p2 = static_core_cmdline_errorlog_time[slot];
	sprintf((char *)p2, "%4d/%02d/%02d %02d:%02d:%02d.%03d",
		c.years, c.months + 1,
		c.mday, c.hour,
		c.minutes, c.seconds,
		c.microsecond);

	p3 = static_core_rewrite_errorlog_time[slot];
	sprintf((char *)p3, "%4d%02d%02d%02d%02d%02d",
		c.years, c.months + 1,
		c.mday, c.hour,
		c.minutes, c.seconds);

	p4 = static_core_collect_sysinfo_time[slot];
	sprintf((char *)p4, "%4d-%02d-%02d %02d:%02d:%02d",
		c.years, c.months + 1,
		c.mday, c.hour,
		c.minutes, c.seconds);

	core_cached_errorlog_time.data = p1;
	core_cmdline_errorlog_time.data = p2;
	core_rewrite_errorlog_time.data = p3;
	core_collect_sysinfo_time.data = p4;
}
