/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */
#pragma once

 /* modules_cycle */
#include <module/modules_cycle.h>

 /* typedef struct */
typedef struct ST_EVENT				tag_event;
typedef struct ST_QUEUE				tag_queue;
typedef struct ST_CONNECTION		tag_connection;
typedef struct ST_LISTENING			tag_listening;
typedef struct ST_EVENT_ACTIONS		tag_event_actions;

/* events_handler_ptr && connection_handler_ptr */
typedef void(*events_handler_ptr)(tag_event *ev);
typedef void(*connection_handler_ptr)(tag_connection *c);

/* alloc/alloc_pool */
#include <alloc/alloc_pool.h>

#include "event_io.h"
#include "event_posted.h"
#include "event_rbtree.h"

/* tag_event */
struct ST_EVENT
{
	void			   *data;

	tag_pool		   *pool;

	unsigned char		write : 1;
	unsigned char		accept : 1;

	unsigned short		instance : 1;

	unsigned char		active : 1;
	unsigned char		disabled : 1;

	unsigned char		ready : 1;
	unsigned char		one_shot : 1;

	unsigned int		index;

	unsigned char		timed_out : 1;
	unsigned char		timer_set : 1;

	unsigned char		delayed : 1;
	unsigned char		deferred_accept : 1;

	int					available;
	events_handler_ptr	ptr_handler;

	unsigned char		posted : 1;
	unsigned char		closed : 1;
	unsigned int		complete : 1;

	unsigned char       eof : 1;
	unsigned char       error : 1;

	/* to test on worker exit */
	unsigned char		channel : 1;
	unsigned char		resolver : 1;

	tag_queue			queue;
	tag_rbtree_node		key_timer;

	unsigned char		cancelable : 1;
};

#if (WINX)
extern unsigned int				event_timer_alarm;
#else
extern sig_atomic_t				event_timer_alarm;
#endif

extern tag_event_actions		proc_event_actions;
extern unsigned int				proc_event_flags;

extern unsigned long long		core_current_msec;
extern unsigned int				core_process;
extern unsigned int				core_worker;

#define proc_events				proc_event_actions.process_events
#define proc_done_event			proc_event_actions.done

#define proc_add_event			proc_event_actions.add
#define proc_del_event			proc_event_actions.del
#define proc_add_conn			proc_event_actions.add_conn
#define proc_del_conn			proc_event_actions.del_conn

/* events_accept */
void event_accept(tag_event * ev);

/* events_accept_enable */
int event_accept_enable(tag_cycle *cycle);

/* tag_events_io */
extern  tag_event_io		event_io;

#define events_recv			event_io.ptr_recv
#define events_recv_udp		event_io.ptr_recv_udp
#define events_send			event_io.ptr_send
#define events_send_udp		event_io.ptr_send_udp

/* process_event_and_timers */
void process_event_and_timers(tag_cycle * cycle);

#define events_get_conf(conf_ctx, module)								\
			(*(modules_get_conf(conf_ctx, event_module))) [module.ctx_index]

/* used in ngx_log_debugX() */
#define events_ident(p)		((tag_connection *) (p))->fd