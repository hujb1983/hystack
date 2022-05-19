/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include "modules.h"


/* modules_load */
class modules_load
{
public:
	static tag_cycle * add_to_module(tag_cycle *cycle, tag_module * m, char *name);
	static tag_cycle * add_to_cycle(tag_cycle *cycle);
	static tag_cycle * add_to_conf(tag_cycle * cycle);
};