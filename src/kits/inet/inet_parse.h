/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once
#include <string>
#include <vector>

#include <core_config.h>

#if (WINX)
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif


/* tag_inet_addr */
typedef struct {
	struct sockaddr_in	 sock_addr;
	int					 sock_len;
	std::string			 name;
} tag_inet_addr;


/* tag_inet_url */
typedef struct {

	std::string						 url;
	std::string						 host;
	std::string						 portText;
	std::string						 uri;
									 
	int								 port;
	int								 portDefault;
	int								 portLast;
	int								 family;
									 
	int								 listen : 1;
	int								 uri_part : 1;
	int								 no_resolve : 1;
									 
	unsigned						 no_port : 1;
	unsigned						 wild_card : 1;
									 
	int								 sock_len;
	struct sockaddr_in				 sock_addr;
	std::vector<tag_inet_addr>		 addrs;

	char							*err;

} tag_inet_uri;


/* inet_domain */
class inet_parse_domain
{
public:
	inet_parse_domain();
	~inet_parse_domain();
};


/* inet_uri */
class inet_parse_uri
{
public:
	std::string		uri;
	std::string		path;
	std::string		file;
	std::string		host;
	std::string		protocol;
	unsigned short  port;

public:
	inet_parse_uri(const char * uri);
	~inet_parse_uri();
};


/* inet_host */
class inet_parse_host
{
public:
	std::vector<tag_inet_addr>  addr_list;

public:
	inet_parse_host(const char * hostname, u_short port);
	~inet_parse_host();
};


/* inet_host */
class inet_parse_port
{
public:
	u_short		nport;

public:
	inet_parse_port(struct sockaddr * sa);
	~inet_parse_port();
};


/* inet_host */
class inet_parse_sockntop
{
public:
	std::string		sock_text;
	u_int			sock_len;
	u_short			sock_port;

public:
	inet_parse_sockntop(struct sockaddr * sa, int sock_len, u_char s_port = 0 );
	~inet_parse_sockntop();
};
