/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <core_config.h>

 /*	logs_record. */
class logs_record
{
public:
	logs_record(void * log, int level);
	virtual ~logs_record();

	logs_record &operator<<(char arg);
	logs_record &operator<<(short arg);
	logs_record &operator<<(unsigned short arg);
	logs_record &operator<<(int arg);
	logs_record &operator<<(unsigned int arg);
	logs_record &operator<<(long long arg);
	logs_record &operator<<(unsigned long long arg);
	logs_record &operator<<(double arg);
	logs_record &operator<<(const char *arg);
	logs_record &operator<<(const std::string &arg);

private:
	void append(const char *data, unsigned int n);
	void append(const char *data);

protected:
	void		   *log_;
	unsigned int	level_;
	unsigned int	count_;
	std::string		line_;
};
