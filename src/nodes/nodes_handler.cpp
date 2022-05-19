/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <random>
#include <chrono>

#include <core.h>
#include <core_config.h>
#include <core_times.h>

#include <conv/conv.h>
#include <conv/conv_json.h>

#include <dev/dev_nginx.h>
#include <dev/dev_stat.h>

#include <inet/inet_http.h>
#include <inet/inet_parse.h>
#include <inet/inet_socket.h>

#include <module/modules.h>
#include <module/modules_conf.h>
#include <module/modules_cycle.h>
#include <module/modules_config.h>

#include "nodes.h"
#include "nodes_tasks.h"
#include "nodes_parse.h"
#include "nodes_exception.h"

/* nodes_authed */
class nodes_authed
{
public:
	nodes_authed() {

	}

	~nodes_authed() {

	}

public:
	/* read_conf */
	int read_conf(const std::string &conf, tag_nodes_auths * auth)
	{
		if (nodes_authed_read_acct(conf, auth->acct) == -1) {
			return -1;
		}

		return 0;
	}

	/* authed_request */
	int authed_request(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_auths * auths = task->auths;

		unsigned char	b64[255] = { 0 };
		conv_base64_encode(b64, sizeof(b64), "product-cdn-node:" + auths->acct.secret);

		std::string		url;
		url = "?account=" + auths->acct.accounts + "&password=" + auths->acct.password;

		std::string token = "Basic ";
		token += (char *)b64;

		inet_parse_uri  uri(auths->api.data());

		std::string pack = "";
		pack += "POST /" + uri.path + url + " HTTP/1.1\r\n";
		pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
		pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
		pack += "Accept: */* \r\n";
		pack += "Connection: keep-alive\r\n";
		pack += "ContentType: application/x-www-form-urlencoded;charset=UTF-8\r\n";
		pack += "Authorization: " + token + "\r\n";
		pack += "\r\n";

		if (cmds->output_sent == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> authlogin request <<\n";
			cmds->output_buff += pack;
			cmds->output_buff += "\n";
		}

		/*	inet_http_request */
		return inet_socket_send(auths->http, (u_char *)pack.data(), (u_short)pack.size());
	}

	/* authed_recv */
	int authed_recv(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;

		u_char			recv_data[4000] = { 0 };
		u_int			recv_size = 3999L;

		int				ret = 0;
		std::string     data;

		tag_http_page	http;
		inet_http_init(&http);

		tag_nodes_auths * auths = nullptr;
		auths = task->auths;

		while(true)
		{
			data.clear();

			for (;;)
			{
				ret = inet_socket_select(auths->http);
				if (ret == 0) {
					break;
				}

				memset(recv_data, 0x0, recv_size);
				ret = inet_socket_recv(auths->http, recv_data, recv_size);
				if (ret < 0) {
					return -1;
				}

				if (ret == 0) {
					break;
				}

				// conv to ascii
				data += (char *)recv_data;
			}
			
			ret = inet_http_response_parse(&http, data);
			if (ret == HTTP_AGAIN) {
				continue;
			}

			if (ret == HTTP_DIRECT) {
				auths->api = http.head_list["location"];
			}
			break;
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += ">> authlogin response <<\n";
			cmds->output_buff += data;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(http.page, 0);
		}

		// body;
		conv_current_charset(auths->data, (u_char *)http.body.data(),
			(int)http.body.size(), conv_charset::emUtf8);

		return ret;
	}

	/* check_body */
	int check_body(tag_nodes_task * task)
	{
		tag_nodes_auths * ptr_auth = task->auths;

		/* repair_json_string */
		std::string js_text;
		nodes_parse::repair_json_string(ptr_auth->data, js_text);

		nodes_parse::tag_packet_data	auth;
		nodes_parse::unpacket			unpack;
		std::string						value;

		cJSON * jr = cJSON_Parse(js_text.data());
		if (jr == nullptr) {
			return -1;
		}

		cJSON * jc = cJSON_GetObjectItem(jr, "code");
		if (jc != nullptr) {

			int err = jc->valueint;
			if (err == 400) {
				return -1;
			}
			else if (err == 500) {
				return -1;
			}
		}

		cJSON * jd = cJSON_GetObjectItem(jr, "data");
		if (jd == nullptr) {
			goto error_result;
		}

		value = cJSON_Print(jd);
		if (unpack.auther_data(value, &auth) == nullptr) {
			goto error_result;
		}

		ptr_auth->token = auth.access_token;

		cJSON_free(jr);
		return 0;

	error_result:
		cJSON_free(jr);
		return 0;
	}
};

/* nodes_handler_authed_check */
int nodes_handler_authed_check(tag_nodes_task * task)
{
	std::shared_ptr<nodes_authed> ptr_auth;
	ptr_auth = std::make_shared<nodes_authed>();

	tag_cycle			*cycle = task->cycle;
	tag_nodes_auths		*auth = task->auths;

	std::string	node_path;
	node_path = cycle->prefix + "/conf/node.json";

	tag_nodes_acct acct;
	if (nodes_authed_read_acct(node_path, acct) == -1) {
		return -1;
	}

	/* acct-auth_api */
	auth->acct = acct;
	auth->api = acct.auth_api;

	/* inet_socket_new */
	auth->http = inet_socket_new(INET_SOCKET_TCP);
	if (auth->http == nullptr) {
		return -1;
	}

	int ret = 0;

	while (true)
	{
		if (inet_socket_connect(auth->http, auth->api.data()) < 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		if (ptr_auth->authed_request(task) <= 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		ret = ptr_auth->authed_recv(task);
		if (ret == HTTP_DONE) {
			break;
		}

		if (ret != HTTP_DIRECT) {
			continue;
		}

		break;
	}

	if (ptr_auth->check_body(task) != 0) {
		goto failedHandler;
	}

	inet_socket_delete(auth->http);
	return 0;

failedHandler:
	inet_socket_delete(auth->http);
	return -1;
}

/* nodes_pulls */
class nodes_pulls
{
public:
	nodes_pulls() {

	}

	~nodes_pulls() {

	}

public:
	/* nodes_pulls_tasklist */
	int heartbeat_request(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_pulls * pulls = task->pulls;
		pulls->token = task->auths->token;

		inet_parse_uri	uri(pulls->api.data());

		std::string pack = "";
		pack += "GET /" + uri.path + "?runingTask=0" + " HTTP/1.1\r\n";
		pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
		pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
		pack += "Accept: */* \r\n";
		pack += "Accept-Encoding: gzip, deflate, br\r\n";
		pack += "Connection: keep-alive\r\n";
		pack += "Product-Auth: bearer " + pulls->token + "\r\n";
		pack += "\r\n";

		if (cmds->output_recv == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> heartbeat request <<\n";
			cmds->output_buff += pack;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(pack, 0);
		}

		/*	inet_http_request */
		return inet_socket_send(pulls->http, (u_char *)pack.data(), (u_short)pack.size());
	}

	/* recv_request */
	int heartbeat_recv(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_pulls * pull = task->pulls;

		u_char	recv_data[4000] = { 0 };
		u_int	recv_size = 3999L;

		int				ret = 0;
		std::string     data;

		tag_http_page	http;
		inet_http_init(&http);

		while (true)
		{
			data.clear();

			for (;;)
			{
				ret = inet_socket_select(pull->http);
				if (ret == 0) {
					break;
				}

				memset(recv_data, 0x0, recv_size);
				ret = inet_socket_recv(pull->http, recv_data, recv_size);
				if (ret < 0) {
					return -1;
				}

				if (ret == 0) {
					break;
				}

				// conv to ascii
				data += (char *)recv_data;
			}
				
			ret = inet_http_response_parse(&http, data);
			if (ret == HTTP_AGAIN) {
				continue;
			}

			if (ret == HTTP_DIRECT) {
				pull->api = http.head_list["location"];
			}
			break;
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += ">> heartbeat response <<\n";
			cmds->output_buff += data;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(http.page, 0);
		}

		// body;
		pull->data.clear();
		conv_current_charset(pull->data, (u_char *)http.body.data(),
			(int)http.body.size(), conv_charset::emUtf8);

		return ret;
	}

	/* nodes_pulls::handler_tasks */
	int heartbeat_parse(tag_nodes_task * task)
	{
		tag_nodes_pulls * pulls = nullptr;
		pulls = task->pulls;

		if (pulls->data.size() == 0) {
			return 0;
		}

		int		 len = 0;
		cJSON	*js_buff, *js_code;
		cJSON	*js_data, *js_tasks;
		cJSON	*js_value;

		js_buff = cJSON_Parse(pulls->data.data());
		if (js_buff == nullptr) {
			return RET_ERROR;
		}

		js_code = cJSON_GetObjectItem(js_buff, "code");
		if (js_code == nullptr) {
			goto deleteJson;
		}

		if (js_code->valueint != 200) {
			goto deleteJson;
		}

		js_data = cJSON_GetObjectItem(js_buff, "data");
		if (js_data == nullptr) {
			goto deleteJson;
		}

		js_tasks = cJSON_GetObjectItem(js_data, "taskIdList");

		len = cJSON_GetArraySize(js_tasks);
		if (len > 0)
		{
			for (int idx = 0; idx < len; idx++)
			{
				js_value = cJSON_GetArrayItem(js_tasks, idx);
				if (js_value == nullptr) {
					goto deleteJson;
				}

				// requests list;
				pulls->requests[js_value->valuestring] = 0;
			}
		}

		cJSON_Delete(js_buff);
		return RET_OK;

	deleteJson:
		cJSON_Delete(js_buff);
		return RET_ERROR;
	}

	/* pulls_request */
	int pulls_request(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;

		tag_nodes_pulls * pulls = task->pulls;
		pulls->token = task->auths->token;

		inet_parse_uri	uri(pulls->api.data());

		std::string pack = "";
		pack += "GET /" + uri.path + "?taskId=" + pulls->task_id + " HTTP/1.1\r\n";
		pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
		pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
		pack += "Accept: */* \r\n";
		pack += "Accept-Encoding: gzip, deflate, br\r\n";
		pack += "Connection: keep-alive\r\n";
		pack += "Product-Auth: bearer " + pulls->token + "\r\n";
		pack += "\r\n";
		
		if (cmds->output_recv == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> pulls request <<\n";
			cmds->output_buff += pack;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(pack, 0);
		}

		/*	inet_http_request */
		return inet_socket_send(pulls->http, (u_char *)pack.data(), (u_short)pack.size());
	}

	/* recv_request */
	int pulls_recv(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_pulls * pull = task->pulls;

		u_char	recv_data[4000] = { 0 };
		u_int	recv_size = 3999L;

		int				ret = 0;
		std::string     data;

		tag_http_page	http;
		inet_http_init(&http);

		while (true)
		{
			data.clear();
			
			for (;;)
			{
				ret = inet_socket_select(pull->http);
				if (ret == 0) {
					break;
				}

				memset(recv_data, 0x0, recv_size);
				ret = inet_socket_recv(pull->http, recv_data, recv_size);
				if (ret <= 0) {
					return -1;
				}

				if (ret == 0) {
					return -1;
				}

				// conv to ascii
				data += (char *)recv_data;
			}

			ret = inet_http_response_parse(&http, data);
			if (ret == HTTP_AGAIN) {
				continue;
			}

			if (ret == HTTP_DIRECT) {
				pull->api = http.head_list["location"];
			}

			break;
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += ">> pulls response <<\n";
			cmds->output_buff += data;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(http.page, 0);
		}

		// body;
		pull->data.clear();
		conv_current_charset(pull->data, (u_char *)http.body.data(),
			(int)http.body.size(), conv_charset::emUtf8);

		return ret;
	}

	/* nodes_handler_pulls_task */
	int pulls_task(tag_nodes_task * task)
	{
		tag_nodes_auths * auth = task->auths;
		tag_nodes_pulls * pull = task->pulls;
		pull->token = auth->token;

		/* repair_json_string */
		std::string json_text;
		nodes_parse::repair_json_string(pull->data, json_text);
		if (pull->data.size() == 0) {
			return -1;
		}

		pull->data.clear();
		task->buff = json_text;

		if (nodes_task_dispatch(task) == RET_ERROR) {
			return -1;
		}

		return 0;
	}
};

/* nodes_handler_heartbeat */
int nodes_handler_heartbeat(tag_nodes_task * task)
{
	std::shared_ptr<nodes_pulls> ptr_pulls;
	ptr_pulls = std::make_shared<nodes_pulls>();

	/* inet_socket_new */
	tag_nodes_auths * auths = task->auths;
	tag_nodes_pulls * pulls = task->pulls;

	pulls->api = auths->acct.task_api;
	pulls->token = auths->token;
	
	pulls->http = inet_socket_new(INET_SOCKET_TCP);
	if (pulls->http == nullptr) {
		return -1;
	}

	int ret = 0;

	while (true)
	{
		if (inet_socket_connect(pulls->http, pulls->api.data()) < 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		if (ptr_pulls->heartbeat_request(task) == -1) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		ret = ptr_pulls->heartbeat_recv(task);
		if (ret == HTTP_DONE) {
			break;
		}

		/* redirect */
		if (ret != HTTP_DIRECT) {
			inet_socket_delete(pulls->http);
			pulls->http = nullptr;
			continue;
		}
	}

	/* http parse */
	pulls->requests.clear();
	if (ptr_pulls->heartbeat_parse(task) != 0) {
		goto failedHandler;
	}

	inet_socket_delete(pulls->http);
	return 0;

failedHandler:
	inet_socket_delete(pulls->http);
	return -1;
}

/* nodes_handler_pull_task */
int nodes_handler_pull_task(tag_nodes_task * task)
{
	std::shared_ptr<nodes_pulls> ptr_pulls;
	ptr_pulls = std::make_shared<nodes_pulls>();

	/* inet_socket_new */
	tag_nodes_auths * auths = task->auths;
	tag_nodes_pulls * pulls = task->pulls;

	pulls->api = auths->acct.pull_api;
	pulls->token = auths->token;

	pulls->http = inet_socket_new(INET_SOCKET_TCP);
	if (pulls->http == nullptr) {
		return -1;
	}

	int ret = 0;
	while (true)
	{
		if (inet_socket_connect(pulls->http, pulls->api.data()) < 0) {
			nodes_error_output(task, "socket_connect error.");
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		if (ptr_pulls->pulls_request(task) == -1) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		ret = ptr_pulls->pulls_recv(task);
		if (ret == HTTP_DONE) {
			break;
		}
		
		/* redirect */
		if (ret != HTTP_DIRECT) {
			inet_socket_delete(pulls->http);
			pulls->http = nullptr;
			continue;
		}
	}

	/* pulls_task */
	if (ptr_pulls->pulls_task(task) != 0) {
		goto failedHandler;
	}

	inet_socket_delete(pulls->http);
	return 0;

failedHandler:
	inet_socket_delete(pulls->http);
	return -1;
}

/* nodes_handler_pulls_process */
int nodes_handler_pulls_process(tag_nodes_task * task)
{
	tag_nodes_auths * auth = task->auths;
	tag_nodes_pulls * pull = task->pulls;

	if (auth->token.size() == 0)
	{
		if (nodes_handler_authed_check(task) == -1) {
			LOG_ERROR(nullptr) << "Authorized login failed.";
			return -1;
		}
	}

	if (pull->requests.size() == 0) 
	{
		if (nodes_handler_heartbeat(task) == -1) {
			LOG_ERROR(nullptr) << "Authorized login failed.";
			auth->token.clear();
			return -1;
		}
	}

	if (pull->requests.size() == 0) {
		return 0;
	}

	int ret = 0;

	NODES_DEBUG("\nstart process task ", 0);

	std::map<std::string, int>	map_temp;

	while(true)
	{
		map_temp.clear();
		for (auto r : pull->requests)
		{
			pull->task_id = r.first;

			auto bt = std::chrono::high_resolution_clock::now();
			ret = nodes_handler_pull_task(task);
			auto et = std::chrono::high_resolution_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(et - bt);
			long long  df = diff.count();

			std::string debug = r.first + ":";
			debug += std::to_string(df) + ":" + std::to_string(ret);
			NODES_DEBUG(debug, 0);

			if (ret == -1) {
				if (r.second > 6) {
					continue;
				}
				map_temp[r.first] = r.second + 1;
			}
		}

		if (map_temp.size() == 0) {
			break;
		}

		pull->requests.clear();
		pull->requests.insert(std::begin(map_temp), std::end(map_temp));
	}

	pull->requests.clear();
	NODES_DEBUG("end process task ", 0);

	return 0;
}

/* nodes_logs */
class nodes_logs
{
public:
	/* stat_request */
	int logs_request(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_upload * logs = task->statlogs;
		logs->token = task->auths->token;

		inet_parse_uri	uri(logs->api.data());

		std::string pack = "";
		pack += "POST /" + uri.path + " HTTP/1.1\r\n";
		pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
		pack += "Accept: */* \r\n";
		pack += "Product-Auth: bearer " + logs->token + "\r\n";
		pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
		pack += "Content-Type: application/json\r\n";
		pack += "Content-Length: " + std::to_string(logs->data.size()) + "\r\n";

		pack += "\r\n";
		pack += logs->data;

		if (cmds->output_recv == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> logs request <<\n";
			cmds->output_buff += pack;
			cmds->output_buff += "\n";
		}
		else {
			NODES_TRACE(pack, 0);
		}

		/*	inet_http_request */
		return inet_socket_send(logs->http, (u_char *)pack.data(), (u_short)pack.size());
	}

	/* stat_recv */
	int logs_recv(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;

		u_char			recv_data[4000] = { 0 };
		u_int			recv_size = 3999L;

		int				ret = 0;
		std::string     data;

		tag_http_page	page;
		inet_http_init(&page);

		tag_nodes_auths * auth = task->auths;
		tag_nodes_upload * logs = task->statlogs;
		logs->token = auth->token;

		while (true)
		{
			data.clear();

			for (;;)
			{
				ret = inet_socket_select(logs->http);
				if (ret == 0) {
					break;
				}

				ret = inet_socket_recv(logs->http, recv_data, recv_size);
				if (ret < 0) {
					return -1;
				}

				// conv to ascii
				data += (char *)recv_data;
			}

			ret = inet_http_response_parse(&page, data);
			if (ret == HTTP_AGAIN) {
				continue;
			}

			if (ret == HTTP_DIRECT) {
				logs->api = page.head_list["location"];
			}

			break;
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += ">> logs response <<\n";
			cmds->output_buff += data;
			cmds->output_buff += "\n";
		}

		// body;
		logs->data.clear();
		conv_current_charset(logs->data, (u_char *)page.body.data(),
			(int)page.body.size(), conv_charset::emUtf8);

		return ret;
	}
};

/* nodes_handler_logs_process */
int nodes_handler_statlogs_process(tag_nodes_task * task)
{
	tag_nodes_upload * logs = task->statlogs;
	tag_nodes_auths * auths = task->auths;

	/*  */
	std::string				 result = "";
	tag_nodes_nginx_param	*param = task->param;

	std::shared_ptr<dev_nginx> ptr_dev;
	ptr_dev = std::make_shared<dev_nginx>(param->nginx.logs);

	ptr_dev->access_read(task->cycle, result);
	if (result.size() == 0) {
		return 0;
	}

	std::shared_ptr<nodes_logs> ptr_logs;
	ptr_logs = std::make_shared<nodes_logs>();
	logs->data = result;
	logs->api = auths->acct.logs_api;

	/* inet_socket_new */
	logs->http = inet_socket_new(INET_SOCKET_TCP);
	if (logs->http == nullptr) {
		return -1;
	}

	int ret = 0;

	while (true)
	{
		if (inet_socket_connect(logs->http, logs->api.data()) < 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		if (ptr_logs->logs_request(task) <= 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		ret = ptr_logs->logs_recv(task);
		if (ret == HTTP_DONE) {
			break;
		}

		if (ret != HTTP_DIRECT) {
			continue;
		}

		break;
	}

	inet_socket_delete(logs->http);
	return 0;

failedHandler:
	inet_socket_delete(logs->http);
	return -1;
}

/* nodes_stat */
class nodes_stat
{
public:
	/* stat_request */
	int stat_request(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;
		tag_nodes_upload * stat = task->statload;
		stat->token = task->auths->token;

		inet_parse_uri	uri(stat->api.data());

		std::string pack = "";
		pack += "POST /" + uri.path + " HTTP/1.1\r\n";
		pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
		pack += "Accept: */* \r\n";
		pack += "Product-Auth: bearer " + stat->token + "\r\n";
		pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
		pack += "Content-Type: application/json\r\n";
		pack += "Content-Length: " + std::to_string(stat->data.size()) + "\r\n";
		
		pack += "\r\n";
		pack += stat->data;

		if (cmds->output_recv == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> stat request <<\n";
			cmds->output_buff += pack;
			cmds->output_buff += "\n";
		}

		/*	inet_http_request */
		return inet_socket_send(stat->http, (u_char *)pack.data(), (u_short)pack.size());
	}

	/* stat_recv */
	int stat_recv(tag_nodes_task * task)
	{
		tag_nodes_cmdline * cmds = task->cmds;

		u_char			recv_data[4000] = { 0 };
		u_int			recv_size = 3999L;

		int				ret = 0;
		std::string     data;

		tag_http_page	page;
		inet_http_init(&page);

		tag_nodes_auths * auth = task->auths;
		tag_nodes_upload * stat = task->statload;
		stat->token = auth->token;

		while (true)
		{
			data.clear();

			for (;;)
			{
				ret = inet_socket_select(stat->http);
				if (ret == 0) {
					break;
				}

				ret = inet_socket_recv(stat->http, recv_data, recv_size);
				if (ret < 0) {
					return -1;
				}

				if (ret == 0) {
					break;
				}

				// conv to ascii
				data += (char *)recv_data;
			}

			ret = inet_http_response_parse(&page, data);
			if (ret == HTTP_AGAIN) {
				continue;
			}

			if (ret == HTTP_DIRECT) {
				stat->api = page.head_list["location"];
			}

			break;
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += "\n";
			cmds->output_buff += ">> stat-load response <<\n";
			cmds->output_buff += data;
			cmds->output_buff += "\n";
		}

		// body;
		stat->data.clear();
		conv_current_charset(stat->data, (u_char *)page.body.data(),
			(int)page.body.size(), conv_charset::emUtf8);

		return ret;
	}
};

/* stat (mem/cpu/net/load/tcp) process */
int nodes_handler_statload_process(tag_nodes_task * task)
{
	std::shared_ptr<nodes_stat> ptr_stat;
	ptr_stat = std::make_shared<nodes_stat>();

	tag_nodes_upload * stat = task->statload;
	tag_nodes_auths * auths = task->auths;

	unsigned char recv_data[4000] = { 0 };
	unsigned int  recv_size = 3999;

	dev_stat_info();
	dev_stat_bejson((u_char *)recv_data, (u_short)recv_size);

	stat->data = (char *)recv_data;
	stat->api = auths->acct.stat_api;

	/* inet_socket_new */
	stat->http = inet_socket_new(INET_SOCKET_TCP);
	if (stat->http == nullptr) {
		return -1;
	}

	int ret = 0;

	while (true)
	{
		if (inet_socket_connect(stat->http, stat->api.data()) < 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		if (ptr_stat->stat_request(task) <= 0) {
			goto failedHandler;
		}

		/* nodes_authed_send_request */
		ret = ptr_stat->stat_recv(task);
		if (ret == HTTP_DONE) {
			break;
		}

		if (ret != HTTP_DIRECT) {
			continue;
		}
		
		break;
	}

	inet_socket_delete(stat->http);
	return 0;

failedHandler:
	inet_socket_delete(stat->http);
	return -1;
}

/* nodes_handler_sync_process */
int nodes_handler_sync_process(tag_nodes_task * task) {
	return 0;
}
