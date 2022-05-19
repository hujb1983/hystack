/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>

#include <conv/conv.h>
#include <conv/conv_json.h>

#include "nodes.h"


 /* nodes_accounts */
class nodes_accounts
{
public:
	nodes_accounts();
	~nodes_accounts();

public:
	std::string	json_text;

public:
	std::string	auth_api;
	std::string	stat_api;
	std::string	logs_api;
	std::string	sync_api;
	std::string	accounts;
	std::string	password;
	std::string	secret;

public:
	int accounts_handler(tag_nodes_acct & acct);
	int read_accounts(const std::string &_acct);
	int read_nodejson(const std::string &_acct);
};


/* nodes_accounts::nodes_accounts */
nodes_accounts::nodes_accounts()
{
	auth_api = "http://dev.aosings.top/api/product-auth-service/tokenByCdnNode";
	stat_api = "ws://dev.aosings.top/api/product-node-log-service/ws";
	logs_api = "ws://dev.aosings.top/api/product-node-log-service/log_ws";
	sync_api = "ws://dev.aosings.top/api/product-sync-service/ws";
	accounts = "node01";
	password = "asy@123456";
	secret = "RyQFL4UsGf9RFxj9tDIA1jdBaRDxyod2r";
}


/* nodes_accounts::nodes_accounts */
nodes_accounts::~nodes_accounts()
{

}


/* nodes_accounts::accounts_handler */
int nodes_accounts::accounts_handler(tag_nodes_acct & acct)
{
	cJSON *jr, *jd;

	jr = cJSON_Parse(json_text.data());
	if (jr == nullptr) {
		LOG_STDERR << "json error: ";
		LOG_STDERR << json_text;
		return -1;
	}

	jd = cJSON_GetObjectItem(jr, "data");
	if (jd == nullptr) {
		LOG_STDERR << "conf/node.json is invalid:";
		LOG_STDERR << json_text;
		cJSON_Delete(jr);
		return -1;
	}

	{
		cJSON *auth, *stat, *logs, *acc, *pwd, *sec;
		auth = cJSON_GetObjectItem(jd, "auth");
		stat = cJSON_GetObjectItem(jd, "stat");
		logs = cJSON_GetObjectItem(jd, "logs");
		acc = cJSON_GetObjectItem(jd, "acct");
		pwd = cJSON_GetObjectItem(jd, "pwd");
		sec = cJSON_GetObjectItem(jd, "secret");

		if (auth == nullptr || pwd == nullptr || sec == nullptr) {
			LOG_STDERR << "conf/node.json is invalid: ";
			LOG_STDERR << json_text;
			cJSON_Delete(jr);
			return -1;
		}

		acct.auth_api = auth->valuestring;
		acct.accounts = acc->valuestring;
		acct.password = pwd->valuestring;
		acct.secret = sec->valuestring;

		acct.stat_api.clear();
		if (stat != nullptr) {
			acct.stat_api = stat->valuestring;
		}

		acct.logs_api.clear();
		if (logs != nullptr) {
			acct.logs_api = logs->valuestring;
		}
	}


	{
		cJSON *heartbeat, *tasks, *notify;
		heartbeat = cJSON_GetObjectItem(jd, "heartbeat");
		tasks = cJSON_GetObjectItem(jd, "tasks");
		notify = cJSON_GetObjectItem(jd, "notify");

		if (heartbeat == nullptr || tasks == nullptr || notify == nullptr) {
			LOG_STDERR << "conf/node.json is invalid: ";
			LOG_STDERR << json_text;
			cJSON_Delete(jr);
			return -1;
		}

		acct.task_api = heartbeat->valuestring;
		acct.pull_api = tasks->valuestring;
		acct.back_api = notify->valuestring;
	}

	cJSON_Delete(jr);
	return 0;
}


/* nodes_accounts::read_nodejson */
int nodes_accounts::read_nodejson(const std::string &node_json)
{
	std::ifstream  files(node_json);
	if (files.is_open() == false) {
		return -1;
	}

	files.seekg(0, std::ios::end);
	int len = (u_int)files.tellg();

	char buf[1024] = {0};
	memset(buf, 0x0, len + 1);

	files.seekg(0, std::ios::beg);
	files.read(buf, len);

	json_text = buf;
	return len;
}


/* nodes_accounts::read_accounts */
int nodes_authed_read_acct(const std::string & node_json, tag_nodes_acct & acct)
{
	// "node.json";
	std::string file = node_json;

	nodes_accounts object;
	if (object.read_nodejson(file.data()) == -1) {
		return -1;
	}

	if (object.accounts_handler(acct) == -1) {
		return -1;
	}

	return 0;
}

