/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */
#pragma once

#include "event.h"
#include "event_io.h"

#include <atomic>
#include <string>

#if (WINX)
#include <windows.h>
#endif

 /* buffered */
#define LOWLEVEL_BUFFERED		0x0f
#define SSL_BUFFERED			0x01
#define HTTP_V2_BUFFERED		0x02

/* tag_listening */
struct ST_LISTENING
{
	int						 sock;
	u_int					 type;
	u_int					 listen;

	u_int					 pool_size;

	struct sockaddr_in		 sock_addr;
	int						 sock_len;
	std::string				 addr_text;

	void					*server;

	connection_handler_ptr	 ptr_handler;

	int						 back_log;
	int						 recv_buffer;
	int						 send_buffer;

	tag_listening			*previous;
	tag_connection			*connection;

	u_int					 worker;
};


/* connection_tcp_nodelay */
typedef enum {
	TCP_NODELAY_UNSET = 0,
	TCP_NODELAY_SET,
	TCP_NODELAY_DISABLED
} connection_tcp_nodelay;


/* tag_connection */
struct ST_CONNECTION
{
	void					*data;
	tag_event				*read;
	tag_event				*write;

	int						 fd;

	recv_ptr				 ptr_recv;
	send_ptr				 ptr_send;
	recv_chain_ptr			 ptr_recv_chain;
	send_chain_ptr			 ptr_send_chain;

	off_t					 sent;
	tag_listening			*listening;

	tag_pool				*pool;
	int						 type;

	struct sockaddr			*sock_addr;
	int						 sock_len;
	tag_string				 addr_text;

	struct sockaddr			*local_sockaddr;
	int						 local_socklen;

	unsigned int			 send_file : 1;
	unsigned int			 send_lowat : 1;

	tag_buffer				*ptr_buffer;
	tag_chain				*ptr_chain;
	std::atomic<int>		 number;

	unsigned long long		 start_time;
	unsigned int			 requests;

	unsigned int			 buffered : 8;

	unsigned int			 timed_out : 1;
	unsigned int			 error : 1;

	unsigned int			 close : 1;
	unsigned int			 shared : 1;

	unsigned int			 tcp_no_delay : 2;
	unsigned int			 tcp_no_push : 2;

	unsigned int             need_last_buff : 1;
};


/* events_connection */
class event_connection
{
public:
	static tag_connection *get(int s);

public:
	static void close(tag_connection * c);
	static void free(tag_connection * c);

public:
	static int tcp_no_delay(tag_connection * c);
	static int tcp_close(int s);
	static int tcp_local_sockaddr(tag_connection *c, tag_string *s, u_int port);

public:
	static int tcp_nonblocking(int s);
	static int tcp_blocking(int s);

public:
	static int tcp_no_push(int s);
	static int tcp_push(int s);
};


/* event_listening */
class event_listening
{
public:
	static int	open(tag_cycle * cycle);
	static void close(tag_cycle * cycle);
	static void close_idle(tag_cycle *cycle);
};
