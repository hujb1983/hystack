/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#if (WINX)
#include <windows.h>
#include <sys\types.h>
#endif

typedef int(*recv_chain_ptr)(tag_connection *c, tag_chain *in, off_t limit);
typedef tag_chain *(*send_chain_ptr)(tag_connection *c, tag_chain *in, off_t limit);

typedef int(*recv_ptr)(tag_connection *c, u_char *buf, u_int size);
typedef int(*send_ptr)(tag_connection *c, u_char *buf, u_int size);

/* tag_events_io */
typedef struct {

	recv_ptr		ptr_recv;
	recv_ptr		ptr_recv_udp;
	send_ptr		ptr_send;
	send_ptr		ptr_send_udp;
	unsigned int	flags;

} tag_event_io;
