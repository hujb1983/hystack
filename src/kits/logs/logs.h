/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <core_config.h>

#include "logs_record.h"
#include "logs_config.h"

/*	logs. */
namespace logs
{
	typedef enum {
		Stderr = 0,
		Emerg,
		Alert,
		Crit,
		Error,
		Warn,
		Notice,
		Info,
		Trace,
		Debug
	} level;
}

/* logs_handler_console */
int logs_handler_console(tag_log *log, int level, u_char * data, u_short size);

/* logs_handler_write_files */
int logs_handler_write_files(tag_log *log, int level, u_char * data, u_short size);

/* logs_handler_write_buffer */
int logs_handler_write_buffer(tag_log *log, int level, u_char * data, u_short size);

/* print logs */
#define LOG(log, level)		logs_record(log, level)
#define LOG_STDERR			LOG( nullptr, logs::Stderr )
#define LOG_EMERG(log)		LOG( log, logs::Emerg )
#define LOG_ALERT(log)		LOG( log, logs::Alert )
#define LOG_CRIT(log)		LOG( log, logs::Crit )
#define LOG_ERROR(log)		LOG( log, logs::Error )
#define LOG_WARN(log)		LOG( log, logs::Warn )
#define LOG_NOTICE(log)		LOG( log, logs::Notice )
#define LOG_INFO(log)		LOG( log, logs::Info )
#define LOG_TRACE(log)		LOG( log, logs::Trace )
#define LOG_DEBUG(log)		LOG( log, logs::Debug )

/* logs_init */
tag_log * logs_init(unsigned char *prefix, unsigned char * error_log);

/* logs_print */
int logs_print(tag_log * log, int level, const std::string & error);
