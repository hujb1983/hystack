/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#include "inet_parse.h"

#if (WINX)
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif

 /* inet_parse_domain */
inet_parse_domain::inet_parse_domain() {

} 

inet_parse_domain::~inet_parse_domain() {

}


/* inet_parse_uri */
inet_parse_uri::inet_parse_uri(const char * uri)
{
	char     h[80] = { 0 };
	char	*ptr1, *ptr2;
	int		 len = 0;

	ptr1 = (char *)uri;

	while ((*ptr1 == ' ') && (ptr1++));

	if (strstr(ptr1, "https://") != nullptr) {
		ptr1 += strlen("https://");
		protocol = "https";
		port = 443;
	}
	else if (strstr(ptr1, "http://") != nullptr) {
		ptr1 += strlen("http://");
		protocol = "http";
		port = 80;
	}
	else if (strstr(ptr1, "wss://") != nullptr) {
		ptr1 += strlen("wss://");
		protocol = "wss";
		port = 443;
	}
	else if (strstr(ptr1, "ws://") != nullptr) {
		ptr1 += strlen("ws://");
		protocol = "ws";
		port = 80;
	}
	else {
		port = 80;
		return;
	}

	ptr2 = strchr(ptr1, '/');
	if (ptr2) {

		len = strlen(ptr1) - strlen(ptr2);
		memcpy(h, ptr1, len);
		host = h;

		if (*(ptr2 + 1)) {
			path = ptr2 + 1;
		}
	}
	else {
		host = ptr1;
	}

	ptr2 = const_cast<char *> (host.data());
	ptr1 = strchr(ptr2, ':');

	if (ptr1) {
		*ptr1++ = '\0';
		port = atoi(ptr1);

		memcpy(h, ptr2, ptr1 - ptr2);
		this->host = h;
	}
}

inet_parse_uri::~inet_parse_uri()
{

}


/* inet_parse_host */
inet_parse_host::inet_parse_host(const char * hostname, u_short port)
{
	addr_list.clear();

	struct hostent *he;
	he = gethostbyname(hostname);

	tag_inet_addr	addr;
	std::string		real_ip;
	for (int i = 0; he->h_addr_list[i]; i++) 
	{
		addr.name = hostname;

		addr.sock_addr.sin_family = AF_INET;
		addr.sock_addr.sin_port = htons(port);

		real_ip = inet_ntoa(*(struct in_addr*)he->h_addr_list[i]);
		addr.sock_addr.sin_addr.s_addr = inet_addr(real_ip.c_str());

		addr.sock_len = sizeof(struct sockaddr_in);
		addr_list.push_back(addr);
	}
}

inet_parse_host::~inet_parse_host()
{

}


/* inet_parse_host */
inet_parse_port::inet_parse_port(struct sockaddr * sa)
{
	struct sockaddr_in * sin;

	switch (sa->sa_family) {

	case AF_INET:

	default: /* AF_INET */

		sin = (struct sockaddr_in *) sa;
		nport = ntohs(sin->sin_port);
	}
}

inet_parse_port::~inet_parse_port()
{

}


/* inet_parse_host */
inet_parse_sockntop::inet_parse_sockntop(struct sockaddr * sa, 
	int sock_len, u_char s_port)
{
	unsigned char			*p;
	struct sockaddr_in		*sin;
	
	char txt[32] = { 0 };

	switch (sa->sa_family) {

	case AF_INET:

		sin = (struct sockaddr_in *) sa;

		p = (unsigned char *)&sin->sin_addr;

		sock_port = ntohs(sin->sin_port);

		if (sock_port != 80) {
			snprintf((char*)txt, sizeof(32), "%ud.%ud.%ud.%ud:%d",
				p[0], p[1], p[2], p[3], sock_port);
		}
		else {
			snprintf((char*)txt, sizeof(32), "%ud.%ud.%ud.%ud",
				p[0], p[1], p[2], p[3]);
		}

		sock_text = txt;
		sock_len = strlen(txt);

	default:
		return;
	}
}

inet_parse_sockntop::~inet_parse_sockntop()
{

}