/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <core_config.h>

/*	write stream. */
class logs_stream
{
public:
	logs_stream();
	logs_stream(unsigned char * pos, unsigned char * last);
	logs_stream(const std::string & data, unsigned int level);
	~logs_stream();

	unsigned int write(const char * path);
	unsigned int write(std::string & path);

	const char  *data();
	unsigned int size();

	void print();

protected:
	std::string _stream;
};

