/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "modules.h"
#include "modules_config.h"
#include "modules_cycle.h"


/* modules_conf::modules_conf */
modules_conf::modules_conf()
{

}

/* modules_conf::~modules_conf */
modules_conf::~modules_conf()
{

}

/* modules_conf::copy */
tag_cycle* modules_conf::copy(tag_cycle *cycle)
{
	return cycle;
}

/* modules_conf::init */
int modules_conf::init(tag_cycle *cycle)
{
	for (auto m : cycle->modules)
	{
		if (m->init_module(cycle) != 0)
		{
			fprintf(stderr, "init module index(%d, %d) failed.\r\n", m->ctx_index, m->index);
			return -1;
		}
	}

	return 0;
}

/* modules_conf::count */
int modules_conf::count(tag_cycle *cycle, unsigned int type)
{
	unsigned int  next, max;

	next = 0;
	max = 0;

	for (auto m : cycle->modules)
	{
		if (m->type != type) {
			continue;
		}

		if (m->ctx_index != MODULE_UNSET_INDEX) {

			if (m->ctx_index > max) {
				max = m->ctx_index;
			}
			if (m->ctx_index == next) {
				next++;
			}
			continue;
		}

		m->ctx_index = ctx_index(cycle, type, next);
		if (m->ctx_index > max) {
			max = m->ctx_index;
		}

		next = m->ctx_index + 1;
	}

	if (cycle->old_cycle && cycle->old_cycle->modules.size()) {

		for (auto m : cycle->old_cycle->modules) {

			if (m->type != type) {
				continue;
			}

			if (m->ctx_index > max) {
				max = m->ctx_index;
			}
		}
	}

	cycle->modules_used = 1;

	return max + 1;
}

/* modules_conf::ctx_index */
int	modules_conf::ctx_index(tag_cycle * cycle, unsigned int type, unsigned int index)
{
again:

	for (auto m : cycle->modules)
	{
		if (m->type != type) {
			continue;
		}

		if (m->ctx_index == index) {
			index++;
			goto again;
		}
	}

	if (cycle->old_cycle && cycle->old_cycle->modules.size()) {

		for (auto m : cycle->modules) {

			if (m->type != type) {
				continue;
			}

			if (m->ctx_index == index) {
				index++;
				goto again;
			}
		}
	}

	return index;
}

