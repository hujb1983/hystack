/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include <core_config.h>
#include <core_times.h>

#include <event/event.h>
#include <event/event_connection.h>

#include <module/modules.h>
#include <module/modules_conf.h>
#include <module/modules_cycle.h>
#include <module/modules_event.h>
#include <module/modules_config.h>

#include <conv/conv.h>
#include <alloc/alloc_pool.h>

#define EP_HAVE_EPOLL 1

#if (EP_HAVE_EPOLL) 
#define EP_EVENT_READ		(EPOLLIN|EPOLLRDHUP)
#define EP_EVENT_WRITE		 EPOLLOUT
#define EP_EVENT_CLOSE		 EVENT_CLOSE
#endif

extern tag_module		event_module;
extern tag_module		event_core_module;

typedef struct {
	unsigned int		events;
	unsigned int		aioRequests;
} tag_epoll_conf;


static int					ep = -1;
static struct epoll_event  *event_list;
static unsigned int			nEvents;

static int epoll_add_event(tag_event *ev, int event, unsigned int flags);
static int epoll_del_event(tag_event *ev, int event, unsigned int flags);
static int epoll_add_connection(tag_connection *c);
static int epoll_del_connection(tag_connection *c, unsigned int flags);
static int epoll_process_events(tag_cycle *cycle, unsigned long long timer, unsigned int flags);
static int epoll_init(tag_cycle *cycle, unsigned long long timer);

static void		epoll_done(tag_cycle *cycle);

static void *	epoll_create_conf(tag_cycle *cycle);
static char *	epoll_init_conf(tag_cycle *cycle, void *conf);

static tag_command  epoll_commands[] = {

	{ init_string("epoll_events"),
	  EVENT_CONF | CONF_TAKE1,
	  modules_parse::number,
	  0,
	  0,
	  nullptr },

	{ init_string("worker_aio_requests"),
	  EVENT_CONF | CONF_TAKE1,
	  modules_parse::number,
	  0,
	  0,
	  nullptr },

	init_command_null
};


tag_event_module epoll_module_ctx = {

	init_string("epoll"),
	epoll_create_conf,                /* create configuration */
	epoll_init_conf,                  /* init configuration */

	{
		epoll_add_event,
		epoll_del_event,
		epoll_add_event,
		epoll_del_event,
		epoll_add_connection,
		epoll_del_connection,
		nullptr,
		epoll_process_events,
		epoll_init,
		epoll_done,
	}
};


tag_module epoll_module = {

	MODULE_V1,
	&epoll_module_ctx,					   /* module context */
	epoll_commands,                        /* module directives */
	EVENT_MODULE,						   /* module type */
	nullptr,                               /* init master */
	nullptr,                               /* init module */
	nullptr,                               /* init process */
	nullptr,                               /* init thread */
	nullptr,                               /* exit thread */
	nullptr,                               /* exit process */
	nullptr,                               /* exit master */
	MODULE_V1_PADDING

};

int epoll_add_event(tag_event *ev, int event, unsigned int flags)
{
	int                  op;
	unsigned int         events, prev;
	tag_event			*e;
	tag_connection		*c;
	struct epoll_event   ee;

	c = (tag_connection *)ev->data;

	events = (unsigned int)event;

	if (event == EP_EVENT_READ) {
		e = c->write;
		prev = EPOLLOUT;
		events = EPOLLIN | EPOLLRDHUP;
	}
	else {
		e = c->read;
		prev = EPOLLIN | EPOLLRDHUP;
		events = EPOLLOUT;
	}

	if (e->active) {
		op = EPOLL_CTL_MOD;
		events |= prev;
	}
	else {
		op = EPOLL_CTL_ADD;
	}

	ee.events = events | (uint32_t)flags;
	ee.data.ptr = (void *)((uintptr_t)c | ev->instance);

	if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
		return -1;
	}

	ev->active = 1;
	return 0;
}


int epoll_del_event(tag_event *ev, int event, unsigned int flags)
{
	int                  op;
	uint32_t             prev;
	tag_event            *e;
	tag_connection       *c;
	struct epoll_event   ee;

	if (flags & EP_EVENT_CLOSE) {
		ev->active = 0;
		return 0;
	}

	c = (tag_connection *)ev->data;

	if (event == EP_EVENT_READ) {
		e = c->write;
		prev = EPOLLOUT;

	}
	else {
		e = c->read;
		prev = EPOLLIN | EPOLLRDHUP;
	}

	if (e->active) {
		op = EPOLL_CTL_MOD;
		ee.events = prev | (uint32_t)flags;
		ee.data.ptr = (void *)((uintptr_t)c | ev->instance);

	}
	else {
		op = EPOLL_CTL_DEL;
		ee.events = 0;
		ee.data.ptr = nullptr;
	}

	if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
		return -1;
	}

	ev->active = 0;
	return 0;
}

int epoll_add_connection(tag_connection *c)
{
	struct epoll_event  ee;

	ee.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
	ee.data.ptr = (void *)((uintptr_t)c | c->read->instance);

	if (epoll_ctl(ep, EPOLL_CTL_ADD, c->fd, &ee) == -1) {
		return -1;
	}

	c->read->active = 1;
	c->write->active = 1;
	return 0;
}

int epoll_del_connection(tag_connection *c, unsigned int flags)
{
	int                 op;
	struct epoll_event  ee;

	if (flags & EP_EVENT_CLOSE) {
		c->read->active = 0;
		c->write->active = 0;
		return 0;
	}

	op = EPOLL_CTL_DEL;
	ee.events = 0;
	ee.data.ptr = nullptr;

	if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
		return -1;
	}

	c->read->active = 0;
	c->write->active = 0;
	return 0;
}

int epoll_process_events(tag_cycle *cycle, unsigned long long timer, unsigned int flags)
{
	int					events;
	uint32_t			revents;
	int					instance, i;
	int   				err;
	tag_queue		   *queue;
	tag_event           *rev, *wev;
	tag_connection      *c;

	events = epoll_wait(ep, event_list, (int)nEvents, timer);
	err = (events == -1) ? errno : 0;

	if (flags & TIMES_UPDATE || event_timer_alarm) {
		core_times::update();
	}

	if (err) {

		if (err == EINTR) {

			if (event_timer_alarm) {
				event_timer_alarm = 0;
				return RET_OK;
			}
		}

		LOG_ERROR(nullptr) << "epoll error: " << err;
		return RET_ERROR;
	}

	if (events == 0) {
		if (timer != TIMER_INFINITE) {
			return 0;
		}
		return RET_ERROR;
	}

	for (i = 0; i < events; i++) {

		c = (tag_connection *)event_list[i].data.ptr;
		instance = (uintptr_t)c & 1;
		c = (tag_connection *)((uintptr_t)c & (uintptr_t)~1);

		rev = c->read;
		if (c->fd == -1 || rev->instance != instance) {

			continue;
		}

		revents = event_list[i].events;
		if (revents & (EPOLLERR | EPOLLHUP)) {

			revents |= EPOLLIN | EPOLLOUT;
		}

		if ((revents & EPOLLIN) && rev->active) {

			rev->ready = 1;
			rev->available = -1;

			queue = rev->accept ? &posted_accept_events :
				&posted_events;
			post_event(rev, queue);
		}

		wev = c->write;
		if ((revents & EPOLLOUT) && wev->active) {

			if (c->fd == -1 || wev->instance != instance) {
				continue;
			}

			wev->ready = 1;
			wev->complete = 1;

			post_event(wev, &posted_events);
		}
	}

	return RET_OK;
}

int epoll_init(tag_cycle *cycle, unsigned long long timer)
{
	tag_epoll_conf * epcf;
	epcf = (tag_epoll_conf *)events_get_conf(cycle->conf_ctx, epoll_module);

	if (ep == -1) {
		ep = epoll_create(cycle->conn_size / 2);
		if (ep == -1) {
			return -1;
		}
	}

	if (nEvents < epcf->events) {
		if (event_list) {
			free(event_list);
		}

		event_list = (struct epoll_event*) alloc_system_ptr(
								sizeof(struct epoll_event) * epcf->events);

		if (event_list == nullptr) {
			return RET_ERROR;
		}
	}

	nEvents = epcf->events;

	proc_event_actions = epoll_module_ctx.actions;
	proc_event_flags = EVENT_USE_LEVEL | EVENT_USE_GREEDY | EVENT_USE_EPOLL;
	return RET_OK;
}


void epoll_done(tag_cycle *cycle)
{
	if (::close(ep) == -1) {
	}

	ep = -1;
	alloc_system_free(event_list);

	event_list = nullptr;
	nEvents = 0;
}


static void *
epoll_create_conf(tag_cycle *cycle)
{
	tag_pool		*pool;
	tag_epoll_conf	*epcf;

	pool = cycle->pool;

	epcf = (tag_epoll_conf *)alloc_pool_ptr(pool, sizeof(tag_epoll_conf));
	if (epcf == nullptr) {
		return nullptr;
	}

	epcf->events = CMD_CONF_UNSET;
	epcf->aioRequests = CMD_CONF_UNSET;
	return epcf;
}

static char *
epoll_init_conf(tag_cycle *cycle, void *conf)
{
	tag_epoll_conf  *epcf;

	epcf = (tag_epoll_conf *)conf;

	if (epcf->events == CMD_CONF_UNSET) {
		epcf->events = 512;
	}

	if (epcf->aioRequests == CMD_CONF_UNSET) {
		epcf->events = 32;
	}

	return (char *)epcf;
}
