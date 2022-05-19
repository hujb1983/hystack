/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#include <map>
#include "inet_socket.h"

/* http return */
#define HTTP_OK					 0
#define HTTP_ERROR				-1
#define HTTP_AGAIN				-2
#define HTTP_BUSY				-3
#define HTTP_DONE				-4
#define HTTP_DIRECT				-5


// typedef struct
typedef struct ST_HTTP_CONNECTION	tag_http_connection;
typedef struct ST_HTTP_KEYVALUE		tag_http_keyvalue;
typedef struct ST_HTTP_OPTION		tag_http_option;
typedef struct ST_HTTP_PAGE			tag_http_page;


/* ST_HTTP_OPTION */
struct ST_HTTP_OPTION
{
	std::string					mode;
	std::string					version;
	
	unsigned int				head;
	unsigned int				body;
	unsigned int				complete;

	unsigned int				length;
	unsigned int				chunked;
	unsigned int				ends;

	unsigned int				loaded;
	unsigned int				overtime;

	unsigned int				format;
	unsigned int				keeplive;
	unsigned int				location;
};


/* ST_HTTP_PARSE */
struct ST_HTTP_KEYVALUE
{
	std::string  key;
	std::string  value;
};


// http head type
#define HTTP_HEAD_DEFAULT					0x00FF
#define HTTP_HEAD_CODE						0x0A01
#define HTTP_HEAD_MODE						0x0001
#define HTTP_HEAD_PATH						0x0002
#define HTTP_HEAD_HOST						0x0003
#define HTTP_HEAD_TOKEN						0x0004
#define HTTP_HEAD_ACCEPT					0x0005
#define HTTP_HEAD_ORIGIN					0x0006
#define HTTP_HEAD_AUTHED					0x0007
#define HTTP_HEAD_UPGRADE					0x0008
#define HTTP_HEAD_LOCATION				    0x0009
#define HTTP_HEAD_USERAGENT				    0x000A
#define HTTP_HEAD_CONNECTION				0x000B
#define HTTP_HEAD_CONTENTTYPE				0x000C
#define HTTP_HEAD_CONTENTLENGTH				0x000D
#define HTTP_HEAD_SERWEBSOCKETKEY			0x000E
#define HTTP_HEAD_SERWEBSOCKETVERSION		0x000F
#define HTTP_HEAD_SERWEBSOCKETEXTENSIONS    0x0010


// http body type
#define HTTP_BODY_TYPE						0xA000		
#define HTTP_BODY_TEXT_HTML					0xA101		// x-www-form-urlencoded
#define HTTP_BODY_TEXT_PLAIN				0xA102		// text/plain
#define HTTP_BODY_TEXT_XML					0xA103		// text/xml
#define HTTP_BODY_IMAGE_GIF					0xA201		// image/gif
#define HTTP_BODY_IMAGE_JPEG				0xA202		// image/jpeg
#define HTTP_BODY_IMAGE_PNG					0xA203		// image/png
#define HTTP_BODY_APP_XHTML					0xA301		// application/xhtml
#define HTTP_BODY_APP_XML					0xA302		// application/xml
#define HTTP_BODY_APP_AXML					0xA303		// application/atom+xml
#define HTTP_BODY_APP_JSON					0xA304		// application/json 
#define HTTP_BODY_APP_PDF					0xA305		// application/pdf 
#define HTTP_BODY_APP_MSWROD				0xA306		// application/msword 
#define HTTP_BODY_APP_STREAM				0xA307		// application/octet-stream 
#define HTTP_BODY_APP_XFORMU				0xA308		// application/x-www-form-urlencoded 
#define HTTP_BODY_APP_FDATA					0xA309		// application/form-data;


/* ST_HTTP_PAGE */
using MAP_KEYVALUE = std::map<unsigned int, tag_http_keyvalue>;
using MAP_KSTRING  = std::map<std::string, std::string>;
using PAIR_KSTRING = std::pair<std::string, std::string>;


/* ST_HTTP_PAGE */
struct ST_HTTP_CONNECTION
{
	int							fd;
	inet_handler			   *hd;

	std::string					uri;

	std::string					origin;
	unsigned int				redirect;
};


/* ST_HTTP_PAGE */
struct ST_HTTP_PAGE
{
	tag_http_option			    option;
	tag_http_connection			connection;

	std::string					domain;
	std::string					api;

	std::string					page;
	std::string					body;

	std::string					prefix;
	std::string					file;

	std::string					head;
	MAP_KSTRING					head_list;

	const char *				buff;
	unsigned int				buff_size;

	std::string					code;
	std::string					path;
	std::string					version;
};


/* inet_http_create_request */
int inet_http_init(tag_http_page * page);
int inet_http_close(tag_http_page * page);

/* inet_http_request_parse */
int inet_http_request_parse(tag_http_page * page, const std::string &data);

/* inet_http_request_body */
int inet_http_request_body(tag_http_page * page, std::string &body);

/* inet_http_response_parse */
int inet_http_response_parse(tag_http_page * page, const std::string &data);

/* inet_http_request_body */
int inet_http_response_body(tag_http_page * page, std::string &body);

/* inet_http_request_body */
int inet_http_response_redirect(tag_http_page * page, std::string &location);
