/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <core.h>
#include <core_config.h>

#include <inet/inet_http.h>
#include <inet/inet_parse.h>
#include <inet/inet_socket.h>

#include "nodes.h"
#include "nodes_tasks.h"
#include "nodes_parse.h"
#include "nodes_process.h"
#include "nodes_exception.h"

#include <conv/conv.h>
#include <conv/conv_json.h>
using namespace nodes_parse;

/* nodes_task_backup::nodes_task_backup */
nodes_task_backup::nodes_task_backup()	{ 
	baks.clear(); 
}

nodes_task_backup::~nodes_task_backup() { 
	baks.clear(); 
}

/* nodes_task_backup::add_task */
int nodes_task_backup::add_task(tag_nodes_task_backup * backup) {
	baks.emplace_back(*backup);
	return 0;
}

/* nodes_task_backup::add_task */
int nodes_task_backup::roll_back()
{

	return 0;
}

/* nodes_task_sync_send */
int nodes_task_sync_send(tag_nodes_task * task)
{
	int				 ret = 0;
	std::string		 pack = "";

	tag_nodes_auths	*auths = task->auths;
	tag_nodes_pulls	*pulls = task->pulls;

	inet_parse_uri	uri(auths->acct.back_api.data());

	std::string param = uri.path + "?";
	param += "isSuccess=";
	param += (pulls->result == 200) ? ("false") : ("true");
	param += "&taskId=" + pulls->task_id;
	
	pack = "POST /" + param + " HTTP/1.1\r\n";
	pack += "Host: " + uri.host + (":") + std::to_string(uri.port) + "\r\n";
	pack += "Product-Auth: bearer " + auths->token + "\r\n";
	pack += "Content-Type: application/json\r\n";
	pack += "Accept: */* \r\n";
	pack += "Content-Length: " + std::to_string(pulls->sent.size()) + "\r\n";
	
	pack += "\r\n";
	pack += pulls->sent;

	/* inet_socket_new */
	pulls->sync = inet_socket_new(INET_SOCKET_TCP);
	if (pulls->sync == nullptr) {
		return -1;
	}

	if (inet_socket_connect(pulls->sync, pulls->api.data()) < 0) {
		goto failedHandler;
	}

	ret = inet_socket_send(pulls->sync, (u_char *)pack.data(), (u_short)pack.size());
	if (ret < (int)pack.size()) {
		goto failedHandler;
	}

	/* nodes_authed_send_request */
	if (true)
	{
		u_char			recv_data[4000] = { 0 };
		u_int			recv_size = 3999L;

		std::string	output;
		output.clear();

		for (;;)
		{
			ret = inet_socket_select(pulls->sync);
			if (ret == 0) {
				break;
			}

			ret = inet_socket_recv(pulls->sync, recv_data, recv_size);
			if (ret < 0) {
				return -1;
			}

			// conv to ascii
			output += (char *)recv_data;
		}
	}

	inet_socket_delete(pulls->sync);
	return 0;

failedHandler:
	inet_socket_delete(pulls->sync);
	return -1;
}

/* nodes_return_packet */
int nodes_return_packet(tag_nodes_task * task)
{
	tag_nodes_pulls * pull = task->pulls;
	tag_nodes_return_packet * packet = task->packet;

	std::string &buff = packet->buff;
	buff.clear();
	
	buff += u8R"({)";
	buff += u8R"("requestid":")";
	buff += packet->request_id;
	buff += u8R"(",)";
	buff += u8R"("code":")" + std::to_string(packet->error_id) + u8R"(",)";
	buff += u8R"("type":"base64",)";
	buff += u8R"("msg":")";

	std::string b64_str;
	b64_str.clear();

	char msg[1024] = { 0 };

	[](char * msg, std::string &b64_str) {

		u_int s = strlen(msg);
		u_int l = s * 5 / 3 + 1;

		u_char  *t = new u_char[l];
		memset(t, 0x0, l);

		conv_base64_encode((u_char*)msg, t, s);
		b64_str = (char *)t;

		delete[] t;

	}(msg, b64_str);

	buff += b64_str;
	packet->message = (char *)msg;

	packet->data = nullptr;
	if (packet->data != nullptr) {
		buff += u8R"(", "data":")";
		buff += cJSON_Print(packet->data);
	}

	buff += u8R"("})";

	pull->sent = buff;
	return nodes_task_sync_send(task);
}

/* nodes_task_nginx_process */
int nodes_task_dispatch(tag_nodes_task * task)
{
	tag_header				  header;
	unpacket				  unpack;
	cJSON					 *js_buff = nullptr, *js_data;

	unpack.header_data(task->buff, &header);
	task->request_id = header.request_id;

	tag_nodes_pulls			 *pulls = task->pulls;
	tag_nodes_return_packet  *packet = task->packet;
	map_nodes_task_process	 &proc = task->proc;
	nodes_task_process_ptr	  ptr_proc = proc[header.function_id];

	if (task->request_id.size() == 0) {
		goto failedHandler;
	}

	js_buff = cJSON_Parse(task->buff.data());
	if (js_buff == nullptr) {
		LOG_ERROR(nullptr) << "json error: ";
		LOG_ERROR(nullptr) << task->buff;
		goto failedHandler;
	}

	js_data = cJSON_GetObjectItem(js_buff, "data");
	if (js_data == nullptr)  {
		LOG_ERROR(nullptr) << "json: can't found(data): ";
		LOG_ERROR(nullptr) << task->buff;
		cJSON_Delete(js_buff);
		js_buff = nullptr;
		goto failedHandler;
	}

	try {
		
		task->data = cJSON_Print(js_data);
		// LOG_INFO(nullptr) << "f:" << header.function_id;

		packet->request_id = pulls->task_id;
		if (ptr_proc != nullptr) {
			ptr_proc(task);
		}
	}
	catch (nodes_exception e) {
		LOG_ERROR(nullptr) << "exception";
		goto failedHandler;
	}

	cJSON_free(js_buff);
	return RET_OK;

failedHandler:
	if (js_buff != nullptr) {
		cJSON_free(js_buff);
	}

	packet->error_id = 901;
	packet->message = "json error";
	nodes_return_packet(task);
	return RET_ERROR;
}


/* nodes_task_nginx_process */
int nodes_task_nginx_process(tag_nodes_task * task)
{
	tag_nodes_nginx_param	 *param = task->param;
	tag_nodes_return_packet  *packet = task->packet;

	try
	{
		tag_packet_data			conf;
		unpacket				unpacket;

		unpacket.file_data(task->data, &conf);
		LOG_INFO(nullptr) << "conf:" << conf.file_name;

		std::string file, path;
		if (strcmp(conf.file_name.data(), "nginx_main") == 0) 
		{
			param->type = "conf";
			param->name = conf.file_name + ".conf.bak";
			param->path = param->nginx.conf;
			param->content = conf.content;
			nodes_nginx_delete_file(task);

			conf.file_name = "nginx";
			param->name = conf.file_name + ".conf";
			param->path = param->nginx.conf;
			param->content = conf.content;
			nodes_nginx_update_file(task);
		}
		if (strcmp(conf.file_name.data(), "cache") == 0)
		{
			param->type = "conf";
			param->name = conf.file_name + ".conf.bak";
			param->path = param->nginx.conf;
			param->content = conf.content;
			nodes_nginx_delete_file(task);

			conf.file_name = "cache";
			nodes_nginx_update_file(task);
		}
		else 
		{
			path = param->nginx.conf;
			path += "/";
			path += conf.file_path;
			path += "/";
			 
#if (!WINX)
			std::string path_log = param->nginx.logs;
			path_log += "/";
			path_log += conf.file_path;
			path_log += "/";
			path_log += conf.file_name;
			path_log += "/";
			if (mkdir(path_log.data(), 0755) == -1) {
				if (access(path_log.data(), F_OK) == -1)
					throw (nodes_exception("mkdir failed.", 900));
			}
#endif
			param->type = "conf";
			param->name = conf.file_name + ".conf.bak";
			param->path = path;
			param->content = conf.content;
			nodes_nginx_delete_file(task);

			if (conf.content.size() > 0) {
				param->name = conf.file_name + ".conf";
				param->path = path;
				param->content = conf.content;
				nodes_nginx_update_file(task);
			}
			else {
				param->name = conf.file_name + ".conf.bak";
				param->path = param->nginx.conf;
				param->content = conf.content;
				nodes_nginx_delete_file(task);
			}

			// test nginx -t, if error then delete ./conf.
			if (nodes_process_test_nginx(task) == RET_ERROR)
			{
				std::string back;
				back = path + conf.file_name + ".conf";

				rename(file.data(), back.data());
				throw(nodes_exception(param->err, 900));
			}
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "successful!";
	nodes_return_packet(task);
	return RET_OK;
}

/* nodes_task_cache_process */
int nodes_task_cache_process(tag_nodes_task * task)
{
	tag_nodes_nginx_param	 *param = task->param;
	tag_nodes_return_packet  *packet = task->packet;

	try
	{
		unpacket			unpacket;
		tag_packet_data		cache;

		unpacket.cache_data(task->data, &cache);
		std::string dir = param->nginx.cache;
		
		if (cache.file_path.size() > 0) {

			if (strcmp(cache.file_path.data(), "domain") == 0) {
				dir += "/";
				dir += cache.file_name.data();
			}
			else if(strcmp(cache.file_path.data(), "nodes") == 0) {
				dir += "/";
			}
			else {
				throw(nodes_exception(cache.file_path, 900));
			}
		}

		if (nodes_nginx_delete_dir(dir.data()) == -1) {
			throw(nodes_exception("remove dir failed.", -1));
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "successful!";
	nodes_return_packet(task);
	return RET_OK;
}


/* nodes_task_redis_process */
int nodes_task_redis_process(tag_nodes_task * task)
{
	// tag_nodes_nginx_param	*param = task->param;
	tag_nodes_return_packet		*packet = task->packet;

	try
	{
		unpacket			unpacket;
		tag_packet_data		redis;

		unpacket.redis_data(task->data, &redis);

		std::string	urls = redis.content;
		if (strcmp(redis.operate.data(), "preheat") == 0) {
			// AsNodeCache::redisSet(node, urls);
		}
		else if (strcmp(redis.operate.data(), "refresh") == 0) {
			// AsNodeCache::redisDel(node, urls);
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "Successful!";
	nodes_return_packet(task);
	return RET_OK;
}


/* nodes_task_ssl_process */
int nodes_task_ssl_process(tag_nodes_task * task)
{
	tag_nodes_return_packet  *packet = task->packet;

	try
	{
		std::string			fpem, fkey;
		unpacket			unpacket;
		tag_packet_data		ssl;

		unpacket.file_data(task->data, &ssl);

		tag_nodes_nginx_param * param = task->param;

		param->type = "pem";
		param->name = ssl.file_name + ".pem.bak";
		param->path = param->nginx.cert;
		param->content = ssl.content;
		nodes_nginx_delete_file(task);

		param->type = "key";
		param->name = ssl.file_name + ".key.bak";
		param->path = param->nginx.cert;
		nodes_nginx_delete_file(task);

		if (ssl.operate == "update" || ssl.operate == "add") {

			param->type = "pem";
			param->name = ssl.file_name + ".pem";
			param->path = param->nginx.cert;
			param->content = ssl.content;
			nodes_nginx_update_file(task);

			param->type = "key";
			param->name = ssl.file_name + ".key";
			param->path = param->nginx.cert;
			param->content = ssl.extra;
			nodes_nginx_update_file(task);
		}
		else if (ssl.operate == "remove") {
			nodes_nginx_delete_file(task);
		}
		else if (ssl.operate == "query") {

		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "successful!";
	nodes_return_packet(task);
	return RET_OK;
}


/* nodes_task_lua_process */
int nodes_task_lua_process(tag_nodes_task * task)
{
	tag_nodes_return_packet	*packet = task->packet;

	int ret = 0;

	try
	{
		tag_packet_data		lua;
		unpacket			unpacket;

		unpacket.lua_data(task->data, &lua);

		tag_nodes_nginx_param * param = task->param;
		param->content = lua.content;
		param->type = "json";
		param->name = lua.file_name + ".json.bak";
		param->path = param->nginx.lua;

		nodes_nginx_delete_file(task);
		if (strcmp(lua.operate.data(), "update") == 0) {
			param->name = lua.file_name + ".json";
			ret = nodes_nginx_update_file(task);
		}
		else if (strcmp(lua.operate.data(), "remove") == 0) {
			param->name = lua.file_name + ".json";
			ret = nodes_nginx_delete_file(task);
		}

		if (ret == RET_ERROR) {
			throw(nodes_exception("failed", 900));
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "successful!";
	nodes_return_packet(task);
	return RET_OK;
}


/* nodes_task_lua_process */
int nodes_task_step_process(tag_nodes_task * task)
{
	tag_nodes_nginx_param		*param = task->param;
	tag_nodes_return_packet		*packet = task->packet;

	int ret = 0;

	try
	{
		unpacket				 unpacket;
		tag_packet_data			 step;

		unpacket.step_data(task->data, &step);

		if (strcmp(step.operate.data(), "start") == 0)
		{
			ret = nodes_process_check_nginx(task);
			if (ret == RET_ERROR) {
				ret = nodes_process_reload_nginx(task);
			}
			else {
				ret = nodes_process_start_nginx(task);
			}

			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
		}
		else if (strcmp(step.operate.data(), "stop") == 0) {
			ret = nodes_process_stop_nginx(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
		}
		else if (strcmp(step.operate.data(), "check") == 0) {
			ret = nodes_process_check_nginx(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
		}
		else if (strcmp(step.operate.data(), "remove") == 0) {
			ret = nodes_process_stop_nginx(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
			ret = nodes_cmdline_remove_nodes(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
		}
		else if (strcmp(step.operate.data(), "update") == 0) {
			ret = nodes_nginx_update_domain(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
			ret = nodes_process_check_nginx(task);
			if (ret == RET_ERROR) {
				throw(nodes_exception(param->err, 900));
			}
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "Successful!";
	nodes_return_packet(task);
	return RET_OK;
}

/* nodes_task_errpage_process */
int nodes_task_errpage_process(tag_nodes_task * task)
{
	tag_nodes_return_packet		*packet = task->packet;

	try
	{
		unpacket			unpacket;
		tag_packet_data		errpage;

		unpacket.file_data(task->data, &errpage);

		tag_nodes_nginx_param * param = task->param;
		param->content = errpage.content;

		param->type = "html";
		param->name = errpage.file_name + ".html.bak";
		param->path = param->nginx.html;
		nodes_nginx_delete_file(task);

		if (strcmp(errpage.operate.data(), "update") == 0) {
			param->name = errpage.file_name + ".html";
			nodes_nginx_update_file(task);
		}

		else if (strcmp(errpage.operate.data(), "remove") == 0) {
			param->name = errpage.file_name + ".html.bak";
			nodes_nginx_delete_file(task);
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "Successful!";
	nodes_return_packet(task);
	return RET_OK;
}

/* nodes_task_http_process */
int nodes_task_http_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_task_defense_process */
int nodes_task_defense_process(tag_nodes_task * task)
{
	tag_nodes_return_packet  *packet = task->packet;
	tag_nodes_nginx_param	 *param = task->param;

	int			ret = 0;
	unpacket	unpacket;

	try
	{
		tag_packet_data	 defense;
		unpacket.defense_data(task->data, &defense);

		if (strcmp(defense.operate.data(), "cc_allow_pass_temporary") == 0) {
			param->opt = "cc_allow_pass_temporary";
			param->path = param->nginx.conf;
			param->content = defense.content;
			ret = nodes_nginx_unix_socket(task);
		}
		else if (strcmp(defense.operate.data(), "cc_add_to_blacklist") == 0) {
			param->opt = "cc_add_to_blacklist";
			param->path = param->nginx.conf;
			param->content = defense.content;
			ret = nodes_nginx_unix_socket(task);
		}
		
		if (ret == RET_ERROR) {
			throw(nodes_exception("failed", 900));
		}
	}
	catch (nodes_exception e)
	{
		packet->error_id = e.code();
		packet->message = e.error();
		nodes_return_packet(task);
		return RET_ERROR;
	}

	packet->error_id = 200;
	packet->message = "Successful!";
	nodes_return_packet(task);
	return RET_OK;
}

