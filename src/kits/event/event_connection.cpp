/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */

#include <thread>
#include <chrono>
#include <random>
#include <functional>
using namespace std;
using namespace chrono;

#if (WINX)
#include <windows.h> 
#pragma comment(lib,"ws2_32.lib") 
#endif

#include <logs/logs.h>
#include <inet/inet_parse.h>
#include <inet/inet_socket.h>

#include <module/modules.h>
#include <module/modules_event.h>
#include <module/modules_cycle.h>

#include "event_connection.h"
#include "event_posted.h"
#include "event_rbtree.h"
#include "event_timer.h"
#include "event.h"

extern tag_cycle*		root_cycle;
extern u_int			process_conf_test;

#if (WINX)
#include <windows.h>
#endif

/* event_connection::get */
tag_connection * event_connection::get(int s)
{
	unsigned short		 instance;
	tag_event			*rev, *wev;
	tag_connection		*c;

	c = root_cycle->conn_free_queue.front();
	if (c == nullptr) {

		// LOG_ERROR(log) << root_cycle->conn_size << " worker_connections are not enough";
		return nullptr;
	}

	root_cycle->conn_free_queue.pop();

	rev = c->read;
	wev = c->write;

	memset(c, 0x0, sizeof(tag_connection));

	c->fd = s;

	instance = rev->instance;

	memset(rev, 0x0, sizeof(tag_event));
	memset(wev, 0x0, sizeof(tag_event));

	c->read = rev;
	c->write = wev;

	rev->instance = !instance;
	wev->instance = !instance;

	rev->index = INVALID_INDEX;
	wev->index = INVALID_INDEX;

	rev->accept = 0;

	rev->data = c;
	wev->data = c;
	wev->write = 1;
	return c;
}


/* event_connection::close */
void event_connection::close(tag_connection *c)
{
	int					err;
	unsigned int		level;
	int					fd;

	if (c->fd == (int)-1) {
		LOG_ALERT(nullptr) << "connection already closed";
		return;
	}

	if (c->read->timer_set) {
		event_timer::del_timer(c->read);
	}

	if (c->write->timer_set) {
		event_timer::del_timer(c->write);
	}

	if (!c->shared) {

		if (proc_del_conn) {
			proc_del_conn(c, EVENT_CLOSE);

		}
		else {
			if (c->read->active || c->read->disabled) {
				proc_del_event(c->read, EVENT_READ, EVENT_CLOSE);
			}

			if (c->write->active || c->write->disabled) {
				proc_del_event(c->write, EVENT_WRITE, EVENT_CLOSE);
			}
		}
	}

	if (c->read->posted) {
		posted_event(c->read);
	}

	if (c->write->posted) {
		posted_event(c->write);
	}

	c->read->closed = 1;
	c->write->closed = 1;

	root_cycle->conn_free_queue.push(c);

	fd = c->fd;
	c->fd = (int)-1;

	if (c->shared) {
		return;
	}

#if (WINX)
	if (::closesocket(fd) == -1) {
#else
	if (::close(fd) == -1) {
#endif
		err = errno;

		if (err == ECONNRESET || err == ENOTCONN) {
			level = logs::Error;
		}
		else {
			level = logs::Crit;
		}

		LOG_ERROR(nullptr) << "error:" << level;
		LOG_ERROR(nullptr) << err << "socket closed " << fd << " failed";
	}

	if (c->pool != nullptr) {
		alloc_pool_reset(c->pool);
	}
}

/* event_connection::free */
void event_connection::free(tag_connection *c) {
	event_connection::close(c);
}

/* event_connection::tcp_no_delay */
int event_connection::tcp_no_delay(tag_connection * c)
{
	int  tcpNodelay;

	if (c->tcp_no_delay != TCP_NODELAY_UNSET) {
		return 0;
	}

	tcpNodelay = 1;

	if (setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&tcpNodelay, sizeof(int)) == -1) {
		return errno;
	}

	c->tcp_no_delay = TCP_NODELAY_SET;
	return 0;
}

/* event_connection::tcp_close */
int event_connection::tcp_close(int s)
{
	int err = 0;

#if (WINX)
	err = ::closesocket(s);
#else
	err = ::close(s);
#endif

	return err;
}

/* event_connection::tcp_local_sockaddr */
int event_connection::tcp_local_sockaddr(tag_connection *c, tag_string *s, u_int port)
{
	socklen_t				len;
	long long				addr;
	tag_inet_sockaddr		sa;

#if (WINX)
	struct sockaddr_in     *sin;
#endif

	addr = 0;

	if (c->local_socklen) {
		switch (c->local_sockaddr->sa_family) {

#if (INET6)
		case AF_INET6:
			sin6 = (struct sockaddr_in6 *) c->local_sockaddr;

			for (i = 0; addr == 0 && i < 16; i++) {
				addr |= sin6->sin6_addr.s6_addr[i];
			}

			break;
#endif

#if (UNIX_DOMAIN)
		case AF_UNIX:
			addr = 1;
			break;
#endif

		default: /* AF_INET */
#if (WINX)
			sin = (struct sockaddr_in *) c->local_sockaddr;
			addr = sin->sin_addr.S_un.S_addr;
#endif
			break;
		}
	}
	if (addr == 0) {

		len = sizeof(tag_inet_sockaddr);

		if (getsockname(c->fd, &sa.sockaddr, &len) == -1) {
			return -1;
		}

		c->local_sockaddr = (struct sockaddr *)alloc_pool_ptr_null(c->pool, len);
		if (c->local_sockaddr == NULL) {
			return -1;
		}

		memcpy(c->local_sockaddr, &sa, len);
		c->local_socklen = len;
	}

	if (s == nullptr) {
		return 0;
	}

	inet_parse_sockntop sockntop(c->local_sockaddr, c->local_socklen);
	s->len = sockntop.sock_len;
	return 0;
}

/* event_connection::tcp_nonblocking */
int event_connection::tcp_nonblocking(int s)
{
#if (WINX)
	return 0;
#else
	int  nb = 1;
	return ioctl(s, FIONBIO, &nb);
#endif 

}

/* event_connection::tcp_blocking */
int event_connection::tcp_blocking(int s)
{
#if (WINX)
	return 0;
#else
	int  nb = 0;
	return ioctl(s, FIONBIO, &nb);
#endif 
}

#if (!WINX)
/* event_connection::tcp_blocking */
int event_connection::tcp_no_push(int s)
{
	int  cork = 1;
	return setsockopt(s, IPPROTO_TCP, TCP_CORK, (const void *)&cork, sizeof(int));
}

/* event_connection::tcp_blocking */
int event_connection::tcp_push(int s)
{
	int  cork = 0;
	return setsockopt(s, IPPROTO_TCP, TCP_CORK, (const void *)&cork, sizeof(int));
}
#endif

/* event_listening::open */
int event_listening::open(tag_cycle * cycle)
{
	int				s;
	int				err;
	int				reuse_addr;
	u_int			failed;

	failed = 0;

	for (auto ls : cycle->listening)
	{
		new(&ls->addr_text)(string);

		if (ls->sock != (int)-1) {
			continue;
		}

		ls->sock = socket(ls->sock_addr.sin_family, ls->type, IPPROTO_TCP);
		if (ls->sock == (int)INVALID_SOCKET) {
			LOG_ERROR(nullptr) << "invalid socket.";
			return -1;
		}

		s = ls->sock;

		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_addr, sizeof(int)) == -1)
		{
			event_connection::tcp_close(s);
			LOG_ERROR(nullptr) << errno << " setsockopt(SO_REUSEADDR) "
				<< s << " failed";
		}

		if (event_connection::tcp_blocking(s) == -1) {
			LOG_ERROR(nullptr) << errno << " Nonblocking failed " << s;

			if (event_connection::tcp_close(s) == -1) {
				LOG_ERROR(nullptr) << errno << " AsCloseSocket failed " << s;
			}
		}

		ls->sock_len = sizeof(struct sockaddr_in);
		if (::bind(s, (struct sockaddr *)&ls->sock_addr, ls->sock_len) == -1) {

			err = errno;

			if (event_connection::tcp_close(s) == -1) {
				LOG_ERROR(nullptr) << errno << " AsCloseSocket(" << s << ")failed ";
			}

			if (err != EADDRINUSE) {
				LOG_ERROR(nullptr) << errno << " bind("
					<< s << ")  failed!";
			}

			if (!process_conf_test) {
				failed = 1;
			}

			continue;
		}

		if (::listen(s, ls->back_log) == -1) {
			err = errno;

			if (event_connection::tcp_close(s) == -1) {
				LOG_ERROR(nullptr) << errno << " tcp_close(" << s << ")failed ";
			}

			if (err != EADDRINUSE) {
				LOG_ERROR(nullptr) << errno << " listen(" << s << ")  failed!";
			}

			if (!process_conf_test) {
				failed = 1;
			}

			continue;

			if (!failed) {
				break;
			}

			LOG_NOTICE(nullptr) << "try again to bind() after 500ms";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		ls->listen = 1;
	}

	if (failed) {
		LOG_ERROR(nullptr) << "still could not bind()";
		return RET_ERROR;
	}

	return RET_OK;
}

/* event_listening::close */
void event_listening::close(tag_cycle * cycle)
{
	tag_connection		*c;

	if (proc_event_flags & EVENT_USE_IOCP) {
		return;
	}

	// process_accept_mutex_held = 0;
	// process_use_accept_mutex = 0;

	for (auto ls : cycle->listening)
	{
		c = ls->connection;

		if (c) {
			if (c->read->active) {
				if (proc_event_flags & EVENT_USE_EPOLL) {

					proc_del_event(c->read, EVENT_READ, 0);
				}
				else {
					proc_del_event(c->read, EVENT_READ, EVENT_CLOSE);
				}
			}

			event_connection::free(c);
			c->fd = (int)-1;
		}

		ls->sock = (int)-1;
	}

	cycle->listening.clear();
}

/* event_listening::close_idle */
void event_listening::close_idle(tag_cycle *cycle)
{

}
