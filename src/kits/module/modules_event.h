/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */
#pragma once

#include "modules.h"
#include "modules_config.h"

 /* ST_EVENT_MODULE */
typedef struct ST_EVENT_ACTIONS	tag_event_actions;
typedef struct ST_EVENT_MODULE	tag_event_module;
typedef struct ST_EVENT			tag_event;

/* define */
#define EVENT_CLOSE				1
#define EVENT_POST				2

#define INVALID_INDEX			0xd0d0d0d0

 /* "EVNT" */
#define EVENT_MODULE			0x544E5645 
#define EVENT_CONF				0x02000000

#define EVENT_READ				0
#define EVENT_WRITE				1
#define EVENT_DISABLE			2

#define EVENT_LEVEL				0
#define EVENT_ONESHOT			1

#define EVENT_LOWAT				0
#define EVENT_CLEAR				0

#define TIMES_UPDATE			1

 /* tag_event_actions */
struct ST_EVENT_ACTIONS {

	int(*add)(tag_event *ev, int event, unsigned int flags);
	int(*del)(tag_event *ev, int event, unsigned int flags);

	int(*enable)(tag_event *ev, int event, unsigned int flags);
	int(*disable)(tag_event *ev, int event, unsigned int flags);

	int(*add_conn)(tag_connection *c);
	int(*del_conn)(tag_connection *c, unsigned int flags);

	int(*notify)(events_handler_ptr handler);
	int(*process_events)(tag_cycle *cycle,unsigned long long timer, unsigned int flags);

	int(*init)(tag_cycle *cycle, unsigned long long timer);
	void(*done)(tag_cycle *cycle);

};

/* tag_event_module */
struct ST_EVENT_MODULE
{
	tag_string			 name;

	void				*(*create_conf)(tag_cycle *cycle);
	char				*(*init_conf)(tag_cycle *cycle, void *conf);

	tag_event_actions	actions;
};

 /* define events */
#define EVENT_USE_LEVEL			0x00000001
#define EVENT_USE_ONESHOT		0x00000002
#define EVENT_USE_CLEAR			0x00000004
#define EVENT_USE_KQUEUE		0x00000008
#define EVENT_USE_LOWAT			0x00000010
#define EVENT_USE_GREEDY		0x00000020
#define EVENT_USE_EPOLL			0x00000040
#define EVENT_USE_RTSIG			0x00000080
#define EVENT_USE_AIO			0x00000100
#define EVENT_USE_IOCP			0x00000200
#define EVENT_USE_FD			0x00000400
#define EVENT_USE_TIMER			0x00000800
#define EVENT_USE_EVENTPORT		0x00001000
#define EVENT_USE_VNODE			0x00002000

/* tag_events_conf */
typedef struct {
	unsigned int		 connections;
	unsigned int		 use;
	int					 accept_multi;
	int					 accept_mutex;
	unsigned long long	 accept_mutex_delay;
	unsigned char		*name;
} tag_event_conf;


/* event_handler_ptr */
typedef void(*event_handler_ptr)(tag_event *ev);

/* connection_handler_ptr */
typedef void(*connection_handler_ptr)(tag_connection *c);
