/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include <thread>
#include <chrono>
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
#include <process/process.h>

extern tag_module			event_module;
extern tag_module			event_core_module;

static fd_set				master_read_fd_set;
static fd_set				master_write_fd_set;
static fd_set				work_read_fd_set;
static fd_set				work_write_fd_set;

static int					select_max_fd;
static unsigned int			select_events;
static tag_event		  **event_indexs;


static int select_proc_add_event(tag_event *ev, int event, unsigned int flags);
static int select_del_event(tag_event *ev, int event, unsigned int flags);
static int select_add_connection(tag_connection *c);
static int select_del_connection(tag_connection *c, unsigned int flags);
static int select_process_events(tag_cycle *cycle, unsigned long long timer, unsigned int flags);
static int select_init(tag_cycle *cycle, unsigned long long timer);

static void  select_done(tag_cycle *cycle);
static char *select_init_conf(tag_cycle *cycle, void *conf);


tag_event_module select_ctx = {

	init_string("select"),
	nullptr,						/* create configuration */
	select_init_conf,				/* init configuration */

	{
		select_proc_add_event,
		select_del_event,
		select_proc_add_event,
		select_del_event,
		select_add_connection,
		select_del_connection,
		nullptr,
		select_process_events,
		select_init,
		select_done,
	}
};

tag_module  select_module = {
	MODULE_V1,
	&select_ctx,								/* module context */
	nullptr,									/* module directives */
	EVENT_MODULE,								/* module type */
	nullptr,									/* init master */
	nullptr,									/* init module */
	nullptr,									/* init process */
	nullptr,									/* init thread */
	nullptr,									/* exit thread */
	nullptr,									/* exit process */
	nullptr,									/* exit master */
	MODULE_V1_PADDING
};

/* select_init_conf */
char * select_init_conf(tag_cycle *cycle, void *conf)
{
	tag_event_conf  *ecf;

	ecf = (tag_event_conf *)events_get_conf(cycle->conf_ctx, event_core_module);
	if (ecf->use != select_module.ctx_index) {
		return (char *)ecf;
	}

	if (cycle->conn_size > FD_SETSIZE) {
		LOG_EMERG(nullptr) << "The maximum number of files supported by select() is " << FD_SETSIZE;
		return nullptr;
	}

	return (char *)ecf;
}

/* select_proc_add_event */
int	select_proc_add_event(tag_event *ev, int event, unsigned int flags)
{
	tag_connection  *c;

	c = (tag_connection *)ev->data;

	if (ev->index != INVALID_INDEX) {
		return 0;
	}

	if ((event == EVENT_READ && ev->write) ||
		(event == EVENT_WRITE && !ev->write)) {
		return -1;
	}

	if (event == EVENT_READ) {
		FD_SET(c->fd, &master_read_fd_set);
	}

	else if (event == EVENT_WRITE) {
		FD_SET(c->fd, &master_write_fd_set);
	}

	if (select_max_fd != -1 && select_max_fd < c->fd) {
		select_max_fd = c->fd;
	}

	ev->active = 1;

	event_indexs[select_events] = ev;
	ev->index = select_events;

	select_events++;
	return 0;
}

/* select_del_event */
int	select_del_event(tag_event *ev, int event, unsigned int flags)
{
	tag_event       *e;
	tag_connection  *c;

	c = (tag_connection *)ev->data;

	ev->active = 0;

	if (ev->index == INVALID_INDEX) {
		return 0;
	}

	if (event == EVENT_READ) {
		FD_CLR(c->fd, &master_read_fd_set);

	}
	else if (event == EVENT_WRITE) {
		FD_CLR(c->fd, &master_write_fd_set);
	}

	if (select_max_fd == c->fd) {
		select_max_fd = -1;
	}

	if (ev->index < --select_events) {
		e = event_indexs[select_events];
		event_indexs[ev->index] = e;
		e->index = ev->index;
	}

	ev->index = INVALID_INDEX;
	return 0;
}

/* select_add_connection */
int	select_add_connection(tag_connection *c)
{
	tag_event	*rev;
	rev = c->read;
	proc_add_event(rev, EVENT_READ, 0);
	return 0;
}

/* select_del_connection */
int	select_del_connection(tag_connection *c, unsigned int flags)
{
	tag_event	*rev;
	rev = c->read;
	proc_add_event(rev, EVENT_READ, 0);
	return 0;
}

/* select_repair_fd_sets */
static void select_repair_fd_sets(tag_cycle *cycle)
{
#if (WINX)
	int				len;
#else
	socklen_t		len;
#endif

	char			n;
	int				err;
	int				s;

	if (select_max_fd == -1) {
		return;
	}

	select_max_fd = select_events;
	for (s = 0; s < select_max_fd; s++)
	{
		if (FD_ISSET(s, &master_read_fd_set) == 0) {
			continue;
		}

		len = sizeof(int);
		if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) {

			err = errno;

			LOG_ERROR(nullptr) 
				<< "select repair"
				<< err << "invalid descriptor #" 
				<< s << "in read fd_set";

			FD_CLR(s, &master_read_fd_set);
		}
	}

	for (s = 0; s < select_max_fd; s++) {

		if (FD_ISSET(s, &master_write_fd_set) == 0) {
			continue;
		}

		len = sizeof(int);

		if (getsockopt(s, SOL_SOCKET, SO_TYPE, &n, &len) == -1) {
			err = errno;

			LOG_ERROR(nullptr) 
				<< "Select repair"
				<< err << "invalid descriptor #" 
				<< s << "in write fd_set";

			FD_CLR(s, &master_write_fd_set);
		}
	}

	select_max_fd = -1;
}

/* event_select */
int	select_process_events(tag_cycle *cycle, unsigned long long timer, unsigned int flags)
{
	int					ready, nready;
	int					err;
	unsigned int		i, found;
	tag_event		   *ev;
	tag_queue		   *queue;
	struct timeval      tv, *tp;
	tag_connection      *c;

	if (select_max_fd == -1)
	{
		for (i = 0; i < select_events; i++)
		{
			c = (tag_connection *)event_indexs[i]->data;
			if (select_max_fd < (int)c->fd)
			{
				select_max_fd = c->fd;
			}
		}
	}

	if (timer == TIMER_INFINITE) {
		tp = nullptr;
	}
	else {
		tv.tv_sec = (long)(timer / 1000);
		tv.tv_usec = (long)(timer % 1000 * 1000);
		tp = &tv;
	}

	work_read_fd_set = master_read_fd_set;
	work_write_fd_set = master_write_fd_set;

	ready = select(select_max_fd + 1, &work_read_fd_set, &work_write_fd_set, nullptr, tp);
	err = (ready == -1) ? errno : 0;

	if (flags & TIMES_UPDATE || event_timer_alarm) {
		core_times::update();
	}

	if (err) {

		unsigned int level = 0;

		if (err == EINTR) {

			if (event_timer_alarm) {
				event_timer_alarm = 0;
				return 0;
			}

			level = logs::Info;
		}
		else {
			level = logs::Alert;
		}

		if (err == EBADF) {
			select_repair_fd_sets(cycle);
		}

		LOG(nullptr, level) << "Select error" << err;
		return -1;
	}

	if (ready == 0) {
		if (timer != TIMER_INFINITE) {
			return 0;
		}

		return -1;
	}

	nready = 0;
	for (i = 0; i < select_events; i++) {

		ev = event_indexs[i];
		c = (tag_connection *)ev->data;
		found = 0;

		if (ev->write)
		{
			if (FD_ISSET(c->fd, &work_write_fd_set)) {
				found = 1;
			}
		}
		else {
			if (FD_ISSET(c->fd, &work_read_fd_set)) {
				found = 1;
			}
		}

		if (found) {

			ev->ready = 1;
			ev->available = -1;

			queue = ev->accept ? &posted_accept_events : &posted_events;
			post_event(ev, queue);

			nready++;
		}
	}

	if (ready != nready) {

		LOG_ERROR(nullptr) << "Select ready error: " << errno;

		this_thread::sleep_for(chrono::milliseconds(1200L));
		select_repair_fd_sets(cycle);
	}

	return 0;
}

/* select_done */
void select_done(tag_cycle *cycle)
{
	alloc_system_free(event_indexs);
	event_indexs = nullptr;
}

/* select_init */
int	select_init(tag_cycle *cycle, unsigned long long timer)
{
	tag_event	**index;
	if (event_indexs == nullptr)
	{
		FD_ZERO(&master_read_fd_set);
		FD_ZERO(&master_write_fd_set);
		select_events = 0;
	}

	if (core_process >= PROCESS_WORKER 
		|| cycle->old_cycle == nullptr 
		|| cycle->old_cycle->conn_size < cycle->conn_size)
	{
		index = (tag_event **)alloc_system_ptr(sizeof(tag_event *) * 2 * cycle->conn_size);
		if (index == nullptr) {
			return -1;
		}

		if (event_indexs) {
			memcpy(index, event_indexs, sizeof(tag_event *) * select_events);
			alloc_system_free(event_indexs);
		}

		event_indexs = index;
	}

	proc_event_actions = select_ctx.actions;
	proc_event_flags = EVENT_USE_LEVEL;

	select_max_fd = -1;
	return 0;
}
