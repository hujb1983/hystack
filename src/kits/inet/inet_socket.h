/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once

#include <string>
#include <vector>

 // winx
#if (WINX)
	#include <windows.h> 
	#pragma comment(lib,"ws2_32.lib") 
#else
	#define INVALID_SOCKET	(unsigned int)(~0)
	#define SOCKET_ERROR				  (-1)
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

#define INET_SOCKADDR_STRLEN   (sizeof("255.255.255.255") - 1)
#define INET_TCPCONN_STRLEN    (INET_SOCKADDR_STRLEN + sizeof(":65535") - 1)

typedef union {
	struct sockaddr           sockaddr;
	struct sockaddr_in        sockaddr_in;

#if (INET6)
	struct sockaddr_in6       sockaddr_in6;
#endif
#if (UNIX_DOMAIN)
	struct sockaddr_un        sockaddr_un;
#endif

} tag_inet_sockaddr;


 /* inet_socket_type */
#define INET_SOCKET_TCP	0x01
#define INET_SOCKET_UDP	0x02
#define INET_SOCKET_SSL	0x04
#define INET_SOCKET_UNX	0x08

 /* inet_handler */
typedef void* inet_handler;

/* inet_socket_new */
inet_handler* inet_socket_new(int type);

/* inet_socket_delete */
void inet_socket_delete(inet_handler *hc);

/* inet_socket_new */
int inet_socket_connect(inet_handler *hc, const char * addr);

/* inet_socket_new */
int inet_socket_select(inet_handler *hc, unsigned int timeout = 1000L);

/* inet_socket_new */
int inet_socket_recv(inet_handler *hc, unsigned char * data, unsigned short size);

/* inet_socket_new */
int inet_socket_send(inet_handler *hc, unsigned char * data, unsigned short size);
