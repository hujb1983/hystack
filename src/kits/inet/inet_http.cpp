/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#include <random>
#include <chrono>
#include <string>

#include <core_config.h>
#include "inet_http.h"
#include "inet_socket.h"


#if (WINX)
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif


 /* http_request */
class inet_http
{
public:
	inet_http();
	virtual ~inet_http();

public:
	int init(tag_http_page * parse);
	int finish(tag_http_page * parse);
	int parse(tag_http_page * parse);
	int release(tag_http_page * parse);

public:
	int parse_head(tag_http_page * parse);
	int parse_body(tag_http_page * parse);
	int parse_chunk(tag_http_page * parse);

public:
	int try_connect(tag_http_page * parse);
	int try_recv(tag_http_page * parse);
	int try_close(tag_http_page * parse);
	int try_redirect(tag_http_page * parse);
};


/* inet_http_create_request */
int inet_http_init(tag_http_page * page)
{
	/* tag_http_option */
	tag_http_option *option = &page->option;
	
	option->mode.clear();
	option->version.clear();

	option->head = 0;
	option->body = 0;

	option->length = 0;
	option->chunked = 0;
	option->ends = 0;

	option->loaded = 0;
	option->overtime = 0;

	option->format = 0;
	option->keeplive = 0;

	/* tag_http_connection */
	tag_http_connection *conn = &page->connection;

	conn->fd = -1;
	conn->uri = "";

	conn->origin = "";
	conn->redirect = 0;

	page->domain.clear();
	page->api.clear();

	page->page.clear();
	page->body.clear();

	page->head.clear();
	page->head_list.clear();

	page->buff = nullptr;
	page->buff_size = 0;

	page->code.clear();
	page->path.clear();
	page->version.clear();

	return HTTP_OK;
}

/* inet_http_create_request */
int inet_http_close(tag_http_page * page)
{

	return HTTP_OK;
}


/* inet_http_create_request */
int inet_http_request_parse(tag_http_page * page, const std::string &data)
{

	return HTTP_OK;
}

/* inet_http_request_body */
int inet_http_request_body(tag_http_page * page, std::string &body)
{

	return HTTP_OK;
}

/* inet_http_response_parse */
int inet_http_response_parse(tag_http_page * page, const std::string &data)
{
	int			ret = 0;

	if (page->option.head == 0) {
		page->page = data;
	}
	else {
		page->page += data;
		if (page->option.chunked == 0) {
			page->body += data;
		}
	}

	/* page - [head : body] */
	ret = [](tag_http_page * page)
	{
		char * ptr = (char *)page->page.data();
		char * end = ptr + page->page.size();

		char * ptr_body = strstr(ptr, "\r\n\r\n");
		if (ptr_body == nullptr) {
			return -1;
		}

		page->head.clear();
		page->head.append(ptr, ptr_body - ptr);
		ptr_body = ptr_body + 4;

		page->body.clear();
		page->body.append(ptr_body, end - ptr_body);
		return 0;

	}(page);
	
	/* ret */
	if (ret < 0) {
		return ret;
	}

	/* page - [http verstion] */
	ret = [](tag_http_page * page)
	{
		char	*ptr = (char *)page->head.data();
		char	 number[4];
		char    *http_ver = (char *)strstr(ptr, "HTTP/");

		if (http_ver != nullptr) {
			http_ver = http_ver + 5;
			while (*http_ver == ' ' && http_ver++);
			number[0] = *(http_ver++);
			number[1] = *(http_ver++);
			number[2] = *(http_ver++);
			number[3] = '\0';
			page->version = number;
		}
		else {
			return -1;
		}

		char * http_code = (char *)strstr(ptr, "HTTP/");
		http_code = http_code + 9;

		while (*http_code == ' ' && http_code++);

		number[0] = *(http_code++);
		number[1] = *(http_code++);
		number[2] = *(http_code++);
		number[3] = *(http_code++);
		page->code = atoi(number);
		return 0;

	}(page);
	
	/* ret */
	if (ret < 0) {
		return ret;
	}

	/* page - head */
	ret = [](tag_http_page * page)
	{
		char * ptr = (char *)page->head.data();
		char * end = ptr + page->head.size();

		page->head_list.clear();

		while (ptr != end)
		{
			char * last = (char *)strstr(ptr, "\r\n");
			if (last == nullptr) {
				break;
			}

			std::string line;
			line.append(ptr, last - ptr);

			ptr = last + 2;
			if (strstr((char *)line.data(), "HTTP") != nullptr) {
				continue;
			}

			char * start = (char *)line.data();
			last = start + line.size();
			start = (char *)strstr(start, ":");
			if (start == nullptr) {
				break;
			}

			start++;

			std::string val;
			val.append(start, last - start);

			start = (char *)line.data();

			char * move = start;
			while (move != last) {
				if (*move >= 'A' && *move <= 'Z') {
					*move += 32;
				}
				++move;
			}

			if (strstr(start, "host:") != nullptr) {
				page->domain = val;
				page->head_list["host"] = val;
			}
			else if (strstr(start, "location:") != nullptr) {
				page->head_list["location"] = val;
				page->option.location = 1;
			}
			else if (strstr(start, "content-length:") != nullptr) {
				page->head_list["content-length"] = val;
			}
			else if (strstr(start, "content-type:") != nullptr) {
				page->head_list["content-type"] = val;
			}
			else if (strstr(start, "transfer-encoding:") != nullptr ) {
				page->head_list["transfer-encoding"] = val;
				if (strstr(start, "chunked") != nullptr) {
					page->option.chunked = 1;
				}
			}
			else if (strstr(start, "sec-websocket-accept:") != nullptr) {
				page->head_list["sec-websocket-accept"] = val;
			}
		}

		return 0;

	}(page);

	/* http [chunked] */
	if (page->option.location == 1) {
		return HTTP_DIRECT;
	}

	/* http [parse] */
	inet_http http;
	if (http.finish(page) == HTTP_OK) 
	{
		http.parse(page);
		return HTTP_DONE;
	}

	return HTTP_AGAIN;
}

/* inet_http_response_parse */
int inet_http_response_body(tag_http_page * page, const std::string &data)
{
	return HTTP_OK;
}

/* inet_http_request_body */
int inet_http_response_redirect(tag_http_page * page, std::string &location)
{
	inet_http http;
	http.parse(page);

	std::string status = page->code;
	if (status.size() > 0) 
	{
		if ((strcmp(status.data(), "301") == 0) || (strcmp(status.data(), "302") == 0)) 
		{
			page->api = "";
			page->api = page->head_list["location"];
			return RET_AGAIN;
		}
	}
	else {
		return RET_ERROR;
	}

	return HTTP_OK;
}

 /* inet_http::init_parse */
inet_http::inet_http() {

}

/* inet_http::init_parse */
inet_http::~inet_http() {

}

/* inet_http::init_parse */
int inet_http::init(tag_http_page * page)
{
	page->buff = nullptr;
	page->buff_size = 0;
	page->page.clear();
	return HTTP_OK;
}

/* inet_http::init_parse */
int inet_http::finish(tag_http_page * page)
{
	int			 len = 0;
	const char * last = nullptr;
	const char * ptr = page->page.data();
	
	ptr = page->head_list["transfer-encoding"].data();
	last = strstr(ptr, "chunked");
	if (last != nullptr) {
		page->option.chunked = 1;
	}

	page->option.complete = 0;
	if (page->body.size() == 0) {
		len = atoi(page->head_list["content-length"].data());
		if (len != 0) {
			return HTTP_ERROR;
		}
		return HTTP_OK;
	}
	
	if (page->option.chunked == 0)
	{
		len = atoi(page->head_list["content-length"].data());
		if (len > (int)page->body.size()) {
			return HTTP_ERROR;
		}

		page->option.complete = 1;
	}

	return HTTP_OK;
}

/* inet_http::init_parse */
int inet_http::parse(tag_http_page * page)
{
	if (page->option.chunked == 1) {
		this->parse_chunk(page);
		return HTTP_OK;
	}

	this->parse_body(page);
	return HTTP_OK;
}

/* inet_http::init_parse */
int inet_http::release(tag_http_page * page)
{
	this->try_close(page);

	return HTTP_OK;
}

/* inet_http::read_head */
int inet_http::parse_head(tag_http_page * page)
{
	return HTTP_OK;
}


/* inet_http::read_body */
int inet_http::parse_body(tag_http_page * page)
{
	return HTTP_OK;
}

/* inet_http::read_chunk */
int inet_http::parse_chunk(tag_http_page * page)
{
	unsigned int	len = 0;
	std::string		text;

	if (page->body.size() == 0) {
		return HTTP_ERROR;
	}

	const char * ptr = nullptr;
	const char * last = nullptr;
	ptr = page->body.c_str();

	std::vector<std::string>	chunks;

	/* for(;;) */
	for (;;)
	{
		last = strstr(ptr, "\r\n");
		if (last == nullptr) {
			break;
		}

		len = last - ptr;
		if (len == 0) {
			break;
		}

		len = hex_toi(ptr, 0, last - ptr);
		if (len == 0) {
			break;
		}

		ptr = last + 2;
		text.append(ptr, len);
		ptr = ptr + len;

		if (text.size() > 0) {
			chunks.push_back(text);
			text = "";
		}
	}

	page->body = "";
	for (auto ch : chunks) {
		page->body += ch;
	}

	return HTTP_OK;
}


/* inet_http::try_connect */
int inet_http::try_connect(tag_http_page * page)
{
	/* inet_socket_new */
	inet_handler * hd = inet_socket_new(INET_SOCKET_TCP);
	if (hd == nullptr) {
		return -1;
	}

	page->connection.hd = hd;

	return inet_socket_connect(hd, page->api.data());
}


/* inet_http::try_recv */
int inet_http::try_recv(tag_http_page * parse)
{
	int ret = 0;
	int fd = parse->connection.fd;

	fd_set read_set_;
	FD_ZERO(&read_set_);
	FD_SET(fd, &read_set_);

	struct timeval tv;

#if (WINX)
	tv.tv_sec = 3;
	tv.tv_usec = 0;
#else
	tv.tv_sec = 3;
	tv.tv_usec = 0;
#endif

	int ready = select(fd + 1, &read_set_, nullptr, nullptr, &tv);
	if (ready < 0) {
		return HTTP_ERROR;
	}

	if (ready == 0) {
		return ready;
	}

	ret = recv(fd, (char *)parse->buff, parse->buff_size, 0);
	if (ret < 0)
	{
		if (errno == EAGAIN) {
			return HTTP_OK;
		}
		else {
			return HTTP_ERROR;
		}
	}

	return ret;
}


/* inet_http::try_close */
int inet_http::try_close(tag_http_page * page)
{
	if (page->connection.hd == nullptr) {
		return HTTP_OK;
	}

	inet_socket_delete(page->connection.hd);
	return HTTP_OK;
}


/* inet_http::try_redirect */
int inet_http::try_redirect(tag_http_page * parse)
{
	if (parse->code.size() == 0) {
		return HTTP_ERROR;
	}

	unsigned int code = 0;
	code = atoi(parse->code.data());

	if (code == 301 || code == 302) {
		parse->connection.origin = parse->connection.uri;
		parse->connection.uri = parse->head_list["location"];
		return HTTP_AGAIN;
	}
	else {
		return HTTP_ERROR;
	}

	return HTTP_OK;
}