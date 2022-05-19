/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "nodes.h"
#include "nodes_exception.h"
#include <core_config.h>

#include <module/modules_cycle.h>
#include <module/modules_config.h>
#include <module/modules_nodes.h>

#include <process/process_mgr.h>
#include <process/process.h>

#include <conv/conv.h>
#include <conv/conv_json.h>

#define _MAXDIR_   256 


/* return errono */
struct result_error {
	enum {
		filepath = 900,
		param,
		context,
		nginx,
		socket
	};
};


/* nodes_process_popen */
int	nodes_process_popen(const std::string & cmd, std::string & output)
{
	process_fork_safe(cmd.data(), output);
	return 0;
}


/* nodes_process_test_nginx */
int	nodes_process_test_nginx(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	std::string	c;
	c = param->nginx.sbin + "nginx -t 2>&1";

	param->err.clear();
	nodes_process_popen(c, param->err);

	char *ptr1, *ptr2, *ptr3;
	ptr1 = const_cast<char *> (param->err.data());
	ptr2 = strstr(ptr1, "syntax is ok");

	if (nullptr == ptr2) {
		return result_error::nginx;
	}

	ptr3 = strstr(ptr2, "test is successful");
	if (nullptr == ptr3) {
		ptr3 = strstr(ptr2, "[alert]");
		if (nullptr != ptr3) {
			return result_error::nginx;
		}
	}

	return 0;
}


/* nodes_process_check_nginx */
int	nodes_process_check_nginx(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	std::string	c;
	c = "ps aux |grep nginx |grep -v grep | awk '{print $2}'";

	param->err.clear();
	nodes_process_popen(c, param->err);

	if (param->err.size() == 0) {
		return  RET_OK;
	}

	return RET_ERROR;
}


/* nodes_process_start_nginx */
int	nodes_process_start_nginx(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;
	param->err.clear();

	std::string	c = param->nginx.sbin + "/nginx";
	nodes_process_popen(c, param->err);
	if (param->err.size() == 0) {
		return RET_OK;
	}

	throw(nodes_exception(param->err, result_error::nginx));
	return RET_ERROR;
}


/* nodes_process_stop_nginx */
int	nodes_process_stop_nginx(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;
	param->err.clear();

	std::string	c = param->nginx.sbin;
	c += "nginx -s stop";

	nodes_process_popen(c, param->err);
	if (param->err.size() == 0) {
		return RET_OK;
	}

	return RET_ERROR;
}


/* nodes_process::reload_nginx */
int	nodes_process_reload_nginx(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	std::string	c = param->nginx.sbin;
	c += "nginx -s reload";

	param->err.clear();
	nodes_process_popen(c, param->err);

	if (param->err.size() == 0) {
		return RET_OK;
	}

	return RET_ERROR;
}


/* nodes_nginx_backup_file */
int nodes_nginx_current_path(const char *p, char *s, int len)
{
	char str[80], *ptr, *pstr;

	if (strlen(p) > (size_t)len) {
		return RET_ERROR;
	}

	ptr = const_cast<char *> (p);
	while (ptr)
	{
		memset(str, 0, sizeof(str));

		pstr = str;

#if (WINX)
		for (; *ptr != '\0' && *ptr != '\\'; ptr++, pstr++) {
			*pstr = *ptr;
		}
#else
		for (; *ptr != '\0' && *ptr != '/'; ptr++, pstr++) {
			*pstr = *ptr;
		}
#endif

		if (*ptr == '\0') {
			break;
		}

		ptr++;

		pstr++;
		*pstr = '\0';

		strcat(s, str);

#if (WINX)
		strcat(s, "\\");
#else
		strcat(s, "/");
#endif
	}

	return RET_OK;
}


/* nodes_nginx_backup_file */
char * nodes_nginx_backup_file(tag_nodes_task * task, std::string &old_file)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	std::string	   &sz_old = old_file;
	std::string		sz_bak;
	char			p[120] = { 0 };

	nodes_nginx_current_path(param->path.data(), p, sizeof(p));

	sz_old = p;
	sz_old += param->name;
	sz_bak = sz_old;
	sz_bak += ".bak";

	rename(sz_old.data(), sz_bak.data());
	return (char *)old_file.data();
}


/* nodes_nginx_backup_file */
int nodes_nginx_update_file(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	int		 err = 0;
	char	 msg[120] = { 0 };
	char	*t = new char[param->content.size() + 1];

	if (param->content.size() == 0) {
		throw(nodes_exception("centent is null.", result_error::context));
		return RET_ERROR;
	}

	std::string old_file;
	nodes_nginx_backup_file(task, old_file);

	ofstream	write_file;
	write_file.open(old_file.data(), ios::out);

	if (write_file.fail())
	{
		err = result_error::filepath;
		strcat(msg, "open file failed. ");
		strcat(msg, old_file.c_str());
		goto error_result;
	}

	memset(t, 0x0, param->content.size() + 1);

	conv_base64_decode((u_char*)t, param->content.size(), param->content);
	write_file.write(t, strlen(t));

	delete[] t;
	write_file.close();

	return RET_OK;

error_result:
	throw(nodes_exception(msg, err));
	return RET_ERROR;
}

/* nodes_nginx_remove_file */
int nodes_nginx_remove_file(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	char			p[120] = { 0 };
	std::string		file;

	if (param->name.size() == 0 || param->path.size() == 0) {
		throw(nodes_exception("nginx-param error.", result_error::nginx));
		return RET_ERROR;
	}

	file = p;
	file += param->name;

	remove(file.data());
	return RET_OK;
}


/* nodes_nginx_delete_file */
int nodes_nginx_delete_file(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	if (param->name.size() == 0 || param->path.size() == 0) {
		throw(nodes_exception("nginx-param error.", result_error::nginx));
		return RET_ERROR;
	}

	char	p[120] = { 0 };
	nodes_nginx_current_path(param->path.data(), p, sizeof(p));

	std::string file = p;
	file += param->name;

	remove(file.data());
	return RET_OK;
}


/* nodes_nginx_delete_dir */
int nodes_nginx_delete_dir(const char * path)
{
#if (!WINX)
	DIR		*dp = nullptr;
	DIR		*dpin = nullptr;

	char	*pathname = (char*)malloc(_MAXDIR_);
	struct dirent* dirp;
	dp = opendir(path);

	if (dp == nullptr) {
		string err_str = path;
		err_str += ", the directory isn't exist";
		throw(nodes_exception(err_str, result_error::filepath));
	}

	while ((dirp = readdir(dp)) != nullptr)
	{
		if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
			continue;
		}

		strcpy(pathname, path);
		strcat(pathname, "\\");
		strcat(pathname, dirp->d_name);

		dpin = opendir(pathname);
		if (dpin != nullptr) {
			nodes_nginx_delete_dir(pathname);
		}
		else {
			remove(pathname);
		}

		strcpy(pathname, "");
		closedir(dpin);
		dpin = nullptr;
	}

	rmdir(path);
	closedir(dp);
	free(pathname);

	pathname = nullptr;
	dirp = nullptr;
#endif

	return RET_OK;
}

/* /nginx-defense? */
int nodes_nginx_unix_socket(tag_nodes_task * task)
{
	char buff[1024] = { 0 };
	std::string pack = "GET /nginx-defense?param=%s&opt=%s HTTP/1.1\r\n";
	pack += "User-Agent: curl/7.29.0\r\n";
	pack += "Host: localhost\r\n";
	pack += "Accept: */* \r\n";
	pack += "Content-Type: application/x-www-form-urlencoded\r\n";
	pack += "\r\n";

	tag_nodes_nginx_param * param = task->param;
	snprintf(buff, sizeof(buff), pack.data(), param->content.data(), param->opt.data());

	tag_nodes_cmdline * cmds = task->cmds;
	if (cmds->output_sent == 1) {
		cmds->output_buff += "\n";
		cmds->output_buff += ">> nginx-defense socket request <<\n";
		cmds->output_buff += pack;
		cmds->output_buff += "\n";
	}

	/* inet_socket_new */
	inet_handler * unix = inet_socket_new(INET_SOCKET_UNX);
	if (unix == nullptr) {
		return -1;
	}

	int		ret = 0;
	u_char	recv_data[4000] = { 0 };
	u_int	recv_size = 3999L;

	do
	{
		if (inet_socket_connect(unix, "/var/run/nginx/nginx.sock") < 0) {
			goto failedHandler;
		}

		/*	inet_http_request */
		ret = inet_socket_send(unix, (u_char *)pack.data(), (u_short)pack.size());
		if (ret <= 0) {
			goto failedHandler;
		}

		for (;;)
		{
			ret = inet_socket_select(unix);
			if (ret == 0) {
				break;
			}

			memset(recv_data, 0x0, recv_size);
			ret = inet_socket_recv(unix, recv_data, recv_size);
			if (ret < 0) {
				return -1;
			}

			if (ret == 0) {
				break;
			}
		}

		if (cmds->output_recv == 1) {
			cmds->output_buff += ">> authlogin response <<\n";
			cmds->output_buff += (char *)recv_data;
			cmds->output_buff += "\n";
		}

	} while (0);

	inet_socket_delete(unix);
	return 0;

failedHandler:
	inet_socket_delete(unix);
	return result_error::socket;
}


/* nodes_nginx_remove_domain */
int nodes_nginx_remove_domain(tag_nodes_task * task)
{
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;

	std::string rm_path = param->nginx.conf;
	
	if (strcmp(param->path.data(), "domain") == 0) {
		rm_path.append("/domain/");
	}
	else {
		rm_path.append("/stream/");
	}

	rm_path.append(param->name);
	rm_path += ".conf";

#if (!WINX)
	if (access(rm_path.data(), R_OK) == -1) {
		return RET_ERROR;
	}
#endif

	std::string new_path = rm_path;
	new_path += ".bak";

	rename(new_path.data(), new_path.data());
	return RET_OK;
}

/* nodes_nginx_update_domain */
int nodes_nginx_update_domain(tag_nodes_task * task)
{
	/* 
	tag_nodes_nginx_param * param = nullptr;
	param = task->param;
	*/

	return RET_OK;
}


/* nodes_cmdline_remove_nodes */
int nodes_cmdline_remove_nodes(tag_nodes_task * task)
{
	std::string prefix = MODULE_PREFIX;
	std::string cmdline = prefix + "/sbin/Asnode -i stopnodes";
	std::string error;

	/* nodes_process_popen */
	nodes_process_popen(cmdline.data(), error);
	if (error.size() == 0) {
		this_thread::sleep_for(chrono::seconds(1));
		return RET_OK;
	}

	return RET_ERROR;
}
