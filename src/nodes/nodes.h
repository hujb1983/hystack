/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#pragma once

#include <map>
#include <string>
#include <string.h>
#include <thread>
#include <chrono>
#include <functional>
using namespace std;
using namespace chrono;
 
#include <core_config.h>
#include <inet/inet_socket.h>
#include <conv/conv_json.h>
#include <alloc/alloc_pool.h>
#include <process/process.h>


/* tag_nodes_authed  */
typedef struct ST_NODES_TASK				tag_nodes_task;
typedef struct ST_NODES_ACCT				tag_nodes_acct;
typedef struct ST_NODES_AUTHS				tag_nodes_auths;
typedef struct ST_NODES_PULLS				tag_nodes_pulls;
typedef struct ST_NODES_UPLOAD				tag_nodes_upload;
typedef struct ST_NODES_NGINX				tag_nodes_nginx;
typedef struct ST_NODES_CMDLINE				tag_nodes_cmdline;
typedef struct ST_NODES_NGINX_PARAM			tag_nodes_nginx_param;
typedef struct ST_NODES_RETURNPACKET		tag_nodes_return_packet;

/* task process */
typedef int(*nodes_task_process_ptr)(tag_nodes_task * task);

/* ST_NODES_ACCT  */
struct ST_NODES_ACCT
{
	ST_NODES_ACCT() {
		auth_api = "";
		stat_api = "";
		logs_api = "";
		sync_api = "";
		task_api = "";
		pull_api = "";
		back_api = "";
		accounts = "";
		password = "";
		secret	 = "";
	}

	std::string			  auth_api;
	std::string			  stat_api;
	std::string			  logs_api;
	std::string			  sync_api;
	std::string			  task_api;
	std::string			  pull_api;
	std::string			  back_api;
	std::string			  accounts;
	std::string			  password;
	std::string			  secret;
};

/* ST_NODE_AUTHED  */
struct ST_NODES_AUTHS
{
	tag_nodes_acct		  acct;
	std::string			  token;
								
	std::string			  api;
	std::string			  old_api;

	inet_handler		 *http;
	std::string			  data;
};

/* nodes_authed_read_acct  */
int nodes_authed_read_acct(const std::string & node_json, tag_nodes_acct & acct);

/* nodes_authed_read_acct  */
using MAP_REQUEST = std::map<std::string, int>;

/* ST_NODES_PULLS */
struct ST_NODES_PULLS
{
	u_char				 pulled;
	u_char				 result;

	std::string			 api;
	std::string			 token;

	std::string			 task_id;
	MAP_REQUEST			 requests;

	inet_handler	    *sync;
	std::string			 sent;

	inet_handler	    *http;
	std::string			 data;
};


/* ST_NODES_PULLS */
struct ST_NODES_UPLOAD
{
	std::string		 	 api;
	std::string		 	 token;
					 
	inet_handler	    *http;
	std::string		 	 data;
};


/* ST_NODES_RETURNPACKET */
struct ST_NODES_RETURNPACKET 
{
	std::string			 buff;
	cJSON				*data;
	
	int					 error_id;
	std::string			 request_id;

	std::string			 message;
};


/* tag_nodes_return_packet */
int nodes_return_packet(tag_nodes_task * packet);


/* the struct is about the nginx-path */
struct ST_NODES_NGINX
{
	std::string			 prefix;
	std::string			 conf;
	std::string			 sbin;
	std::string			 cache;
	std::string			 cert;
	std::string			 html;
	std::string			 logs;
	std::string			 lua;
};


/* the struct is about the nginx-param */
struct ST_NODES_NGINX_PARAM
{
	tag_nodes_nginx	     nginx;
	std::string			 err;
						 
	std::string			 name;
	std::string			 type;
	std::string			 path;
	std::string			 opt;
	std::string			 content;
	std::string			 ext;
	std::string			 result;
};

/* the struct is about the nginx-param */
struct ST_NODES_CMDLINE
{
	unsigned int		 output;
	std::string			 output_buff;
	std::string			 output_file_path;

	unsigned int		 output_file;
	unsigned int		 output_error;
	unsigned int		 output_stderr;
	unsigned int		 output_sent;
	unsigned int		 output_recv;

	unsigned int		 reprocess;
	unsigned int		 reprocess_times;

	unsigned int		 print_nodeslogs;
	unsigned int		 print_nginxlogs;
};

/* nodes_exception_init */
int nodes_exception_init(const std::string &prefix);

/* output error message. */
void nodes_error_output(tag_nodes_task * task, const std::string & errmsg, int type = 0);

/* map_nodes_task_process */
using map_nodes_task_process = std::map<u_int, nodes_task_process_ptr>;

/* the struct is about the nodes-task */
struct ST_NODES_TASK
{
	tag_nodes_cmdline			*cmds;

	std::string					 buff;
	std::string					 data;
	std::string					 request_id;

	tag_cycle					*cycle;

	tag_nodes_auths				*auths;
	tag_nodes_pulls				*pulls;

	map_nodes_task_process		 proc;

	tag_nodes_nginx_param		*param;
	tag_nodes_return_packet		*packet;

	tag_nodes_upload		    *statlogs;
	tag_nodes_upload		    *statload;

	unsigned int				 next_times;
};
