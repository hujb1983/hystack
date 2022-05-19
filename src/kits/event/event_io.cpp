/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#include "event.h"
#include "event_io.h"
#include "event_connection.h"
#include <module/modules_event.h>

static int event_posix_recv(tag_connection *c, u_char *buf, u_int size);
static int event_posix_send(tag_connection *c, u_char *buf, u_int size);

/* tag_events_io events_io */
tag_event_io event_io = {
	event_posix_recv,
	nullptr,
	event_posix_send,
	nullptr,
	0
};


/* events_posix_recv */
static int event_posix_recv(tag_connection *c, u_char *buf, u_int size)
{
	int				 n;
	int				 err;
	tag_event		*rev;

	rev = c->read;

	do {

		n = recv(c->fd, (char*)buf, size, 0);

		std::string bug = "recv fd(";
		bug += std::to_string(c->fd);
		bug += ")";
		bug += std::to_string(n);
		bug += std::to_string(size);
		LOG_DEBUG(nullptr) << bug;

		if (n == 0) {

			rev->ready = 0;
			rev->eof = 1;
			return 0;
		}

		if (n > 0) {

			if ((size_t)n < size && !(proc_event_flags & EVENT_USE_GREEDY)) {
				rev->ready = 0;
			}

			rev->available = 0;

			return n;
		}

		err = errno;

		if (err == EAGAIN || err == EINTR) {
			LOG_DEBUG(nullptr) << err << "recv() not ready";
			n = RET_AGAIN;
		}
		else {
			n = RET_ERROR;
			LOG_ERROR(nullptr) << err << "recv() failed";
			break;
		}


	} while (err == EINTR);

	if (n < -1) {
		rev->error = 1;
	}

	return RET_OK;
}


/* events_posix_send */
static int event_posix_send(tag_connection *c, u_char *buf, u_int size)
{
	u_int			 n;
	int				 err;
	tag_event		*wev;

	wev = c->write;

	for (;; ) {

		n = send(c->fd, (char *)buf, size, 0);

		LOG_DEBUG(nullptr) << "recv fd("
			<< c->fd
			<< ") "
			<< n
			<< size;

		if (n > 0) {

			if (n < (u_int)size) {
				wev->ready = 0;
			}

			c->sent += n;

			return n;
		}

		err = errno;

		if (n == 0) {

			LOG_ERROR(nullptr) << err
				<< "send() returned zero";

			wev->ready = 0;
			return n;
		}

		if (err == EAGAIN || err == EINTR) {

			wev->ready = 0;

			LOG_ERROR(nullptr) << err << "send() not ready";
			if (err == EAGAIN) {
				return RET_AGAIN;
			}
		}
		else {

			wev->error = 1;

			LOG_ERROR(nullptr) << err << "recv() failed";
			return RET_ERROR;
		}
	}

	return -1;
}