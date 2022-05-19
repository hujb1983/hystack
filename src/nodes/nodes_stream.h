/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#pragma once

#include "nodes.h"
#include <files/files_stream.h>


/* nodes files stream */
class nodes_stream : public files_stream
{
public:
	nodes_stream(const std::string &prefix);
	virtual ~nodes_stream();

public:
	std::string	file_scene;
	virtual int init(const std::string &file_path, const std::string &file_name);

public:
	virtual int redirect();
	virtual int serial(unsigned char save);
};
