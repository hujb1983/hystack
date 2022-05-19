/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <core_config.h>
#include "unix_config.h"

#include <event/event.h>
#include <event/event_connection.h>
#include <module/modules_event.h>


int unix_channel_write(int s, tag_channel *ch, unsigned int size)
{
	int				n;
	int				err;
	struct iovec	iov[1];
	struct msghdr	msg;

#if (HAVE_MSGHDR_MSG_CONTROL)


#else
	if (ch->fd == -1) {
		msg.msg_accrights = nullptr;
		msg.msg_accrightslen = 0;

	}
	else {
		msg.msg_accrights = (caddr_t)&ch->fd;
		msg.msg_accrightslen = sizeof(int);
	}
#endif

	iov[0].iov_base = (char *)ch;
	iov[0].iov_len = size;

	msg.msg_name = nullptr;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	n = sendmsg(s, &msg, 0);
	if (n == -1)
	{
		err = errno;
		if (err == EAGAIN) {
			return RET_AGAIN;
		}

		LOG_ALERT(nullptr) << err << "sendmsg() failed";
		return -1;
	}

	return 0;
}

int unix_channel_read(int s, tag_channel *ch, unsigned int size)
{
	int				n;
	int				err;
	struct iovec	iov[1];
	struct msghdr	msg;

#if (HAVE_MSGHDR_MSG_CONTROL)
	union {
		struct cmsghdr  cm;
		char            space[CMSG_SPACE(sizeof(int))];
	} cmsg;
#else
	int                 fd;
#endif


	iov[0].iov_base = (char *)ch;
	iov[0].iov_len = size;

	msg.msg_name = nullptr;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

#if (HAVE_MSGHDR_MSG_CONTROL)
	msg.msg_control = (caddr_t)&cmsg;
	msg.msg_controllen = sizeof(cmsg);
#else 
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof(int);
#endif

	n = recvmsg(s, &msg, 0);
	if (n == -1) {
		err = errno;
		if (err == EAGAIN) {
			return RET_AGAIN;
		}
		
		LOG_ERROR(nullptr) << err << "recvmsg() failed";
		return -1;
	}

	if (n == 0) {
		LOG_DEBUG(nullptr) << "recvmsg() returned zero";
		return -1;
	}

	if ((size_t)n < sizeof(tag_channel)) {
		LOG_DEBUG(nullptr) << "recvmsg() returned not enough data:" << n;
		return -1;
	}

#if (HAVE_MSGHDR_MSG_CONTROL)


#else
	if (ch->command == CMD_OPEN_CHANNEL) {
		if (msg.msg_accrightslen != sizeof(int)) {
			LOG_ALERT(nullptr) << " recvmsg() returned no ancillary data" << err;
			return -1;
		}
		ch->fd = fd;
	}
#endif

	return n;
}

int unix_channel_add_event(tag_cycle *cycle, int fd, int event, events_handler_ptr handler)
{
	tag_event       *ev, *rev, *wev;
	tag_connection  *c;

	c = event_connection::get(fd);

	if (c == nullptr) {
		return -1;
	}

	c->pool = cycle->pool;

	rev = c->read;
	wev = c->write;

	rev->channel = 1;
	wev->channel = 1;

	ev = (event == EVENT_READ) ? rev : wev;
	ev->ptr_handler = handler;

	if (proc_add_conn && (proc_event_flags & EVENT_USE_EPOLL) == 0) {
		if (proc_add_conn(c) == -1) {
			event_connection::free(c);
			return -1;
		}
	}
	else {
		if (proc_add_event(ev, event, 0) == -1) {
			event_connection::free(c);
			return -1;
		}
	}

	return 0;
}

int unix_channel_close(int * fd) 
{
	if (::close(fd[0]) == -1) {
		LOG_ALERT(nullptr) << "close() channel failed" << errno;
	}

	if (::close(fd[1]) == -1) {
		LOG_ALERT(nullptr) << "close() channel failed" << errno;
	}
	return 0;
}

