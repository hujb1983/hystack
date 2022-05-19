/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */

#include "event.h"
#include "event_io.h"
#include "event_connection.h"

#include <module/modules.h>
#include <module/modules_event.h>
#include <module/modules_cycle.h>

extern tag_cycle	*root_cycle;
extern tag_module	 event_core_module;
extern tag_module	 event_module;

/* events_accept_connection_close */
static void event_accept_connection_close(tag_connection *c)
{
	event_connection::free(c);

	c->fd = -1;

	if (c->pool) {
		alloc_pool_destory(c->pool);
	}
}

/* events_accept */
void event_accept(tag_event * ev)
{
	socklen_t			 sock_len;
	int					 err;
	int					 s;
	tag_listening		*ls;
	tag_connection		*c, *lc;
	tag_event			*rev, *wev;
	struct sockaddr_in	 sa;


	if (ev->timed_out)
	{
		if (event_accept_enable((tag_cycle *)root_cycle) != RET_OK) {
			return;
		}
		ev->timed_out = 0;
	}

	// tag_event_conf		*ecf;
	// ecf = (tag_event_conf *) events_get_conf(root_cycle->conf_ctx, events_core_module);

	lc = (tag_connection *)ev->data;
	ls = lc->listening;
	ev->ready = 0;

	LOG_ERROR(nullptr) << "accept on " << ls->addr_text << ", ready: " << ev->available;

	do
	{
		sock_len = sizeof(struct sockaddr_in);

		s = accept(lc->fd, (struct sockaddr *)&sa, &sock_len);
		if (s == -1) {
			err = errno;

			if (err == EAGAIN) {
				LOG_ERROR(nullptr) << err << " accept() not ready";
				return;
			}
		}

		c = event_connection::get(s);
		if (c == nullptr)
		{
			if (event_connection::tcp_close(s) == -1) {
				LOG_ERROR(nullptr) << errno << " failed";
			}
			return;
		}

		c->type = SOCK_STREAM;

		if (c->pool == nullptr)
		{
			c->pool = (tag_pool *)alloc_pool_create(sizeof(tag_pool));
			if (c->pool == nullptr) {
				event_accept_connection_close(c);
				return;
			}
		}

		if (sock_len > (int) sizeof(struct sockaddr)) {
			sock_len = sizeof(struct sockaddr_in);
		}

		c->sock_addr = (struct sockaddr*)alloc_pool_ptr_null(c->pool, sock_len);
		if (c->sock_addr == nullptr) {
			event_accept_connection_close(c);
			return;
		}

		memcpy(c->sock_addr, &sa, sock_len);

		c->ptr_recv = events_recv;
		c->ptr_send = events_send;

		c->sock_len = sock_len;
		c->listening = ls;

		c->local_sockaddr = (struct sockaddr *)&ls->sock_addr;
		c->local_socklen = ls->sock_len;

		rev = c->read;
		wev = c->write;

		rev->write = 1;
		wev->ready = 1;

		c->start_time = core_current_msec;

		if (proc_add_conn != nullptr) {
			if (proc_add_conn(c) == RET_ERROR) {
				event_accept_connection_close(c);
				return;
			}
		}

		ls->ptr_handler(c);

	} while (ev->available > 0);
}

/* event_accept_enable */
int event_accept_enable(tag_cycle *cycle)
{
	tag_connection	*c;

	for (auto ls : cycle->listening) {

		c = ls->connection;

		if (c == nullptr || c->read->active) {
			continue;
		}

		if (proc_add_event(c->read, EVENT_READ, 0) == RET_ERROR) {
			return RET_ERROR;
		}
	}

	return RET_OK;
}


/* events_accept_disable */
int events_accept_disable(tag_cycle *cycle)
{
	tag_connection		*c;

	for (auto ls : cycle->listening) {

		c = ls->connection;

		if (c == nullptr || !c->read->active) {
			continue;
		}

		if (proc_del_event(c->read, EVENT_READ, EVENT_DISABLE) == RET_ERROR) {
			return RET_ERROR;
		}
	}

	return RET_OK;
}
