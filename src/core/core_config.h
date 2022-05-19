/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#define RET_OK					 0
#define RET_ERROR				-1
#define RET_AGAIN				-2
#define RET_BUSY				-3
#define RET_DONE				-4
#define RET_DECLINED			-5
#define RET_ABORT				-6
#define RET_PING				-7
#define RET_PONG				-8

#if (WINX)
typedef unsigned long long		sig_atomic_t;
typedef int						socklen_t;
#else
typedef int						sig_atomic_t;
#endif

typedef struct ST_LOG			tag_log;
typedef struct ST_STRING		tag_string;
typedef struct ST_CYCLE			tag_cycle;
typedef struct ST_MODULE		tag_module;
typedef struct ST_COMMAND		tag_command;

typedef struct ST_QUEUE			tag_queue;
typedef struct ST_EVENT			tag_event;
typedef struct ST_CONNECTION	tag_connection;
typedef struct ST_LISTENING		tag_listening;


/* events_handler_ptr */
typedef void(*events_handler_ptr)(tag_event *ev);

/* connection_handler_ptr */
typedef void(*connection_handler_ptr)(tag_connection *c);

#if (WINX)
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unix_config.h>
#endif


#if (!WINX)

extern pid_t					process_parent;
extern pid_t					process_pid;
extern pid_t					process_new_binary;
#else 
extern int						process_parent;
extern int						process_pid;
extern int						process_new_binary;
#endif

/* include header */
#include "core_cycle.h"
#include "core_string.h"

/* process accept */
#include <atomic>
extern std::atomic<int>			process_no_accept;
extern unsigned int				process_no_accepting;
extern unsigned int				process_restart;
