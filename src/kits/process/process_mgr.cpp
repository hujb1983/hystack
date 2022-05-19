/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <core.h>
#include <core_config.h>

#include <core.h>
#include <thread>
#include <chrono>
using namespace std;

#include "process.h"
#include "process_mgr.h"

/* process_cycle */
extern core_cycle	*process_cycle;

/* process_mgr::process_mgr */
process_mgr::process_mgr() {

}

/* process_mgr::~process_mgr */
process_mgr::~process_mgr() {

}

/* core_solution.cpp */
int process_mgr::run(tag_cycle *cycle)
{
	if (process_cycle == nullptr) {
		return -1;
	}

	return process_cycle->run(cycle);
}

/* process_mgr::set_current_cycle */
int process_mgr::set_current_cycle(core_cycle *cycle) {
	process_cycle = cycle;
	return 0;
}

/* process_mgr::sleep */
int process_mgr::sleep(unsigned long _milliseconds) {
	this_thread::sleep_for(chrono::milliseconds(_milliseconds));
	return 0;
}

/* process_mgr::execute */
int process_mgr::execute(tag_cycle *cycle, tag_core_exec *ctx)
{
	return 0;
}

/* process_mgr::dump */
int process_mgr::dump(tag_cycle * cycle)
{
#if (!WINX)
	struct rlimit rlmt;
	if (getrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1;
	}

	const int core_size = 1024 * 1024 * 500;
	rlmt.rlim_cur = (rlim_t)core_size;
	rlmt.rlim_max = (rlim_t)core_size;

	if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1;
	}
#endif
	return 0;
}
