/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2022
 */

#include <core.h>
#include <core_config.h>

#include <fstream>
#include <time.h>

#include "dev_nginx.h"

/* tag_logs_param */
struct tag_logs_param
{
	time_t		start_time;
	time_t		last_time;

	u_short		port;
	u_short		sent;
	u_short		recv;
};


dev_nginx::dev_nginx(std::string & logs_path) 
{
	max_size = 10;
	domain_path.clear();
	domain_path = logs_path;

	read_logs.access_time = 0;
	read_logs.access_size = 0;
	read_logs.stream_time = 0;
	read_logs.stream_size = 0;
	read_logs.deal_times = 0;
	read_logs.send_size = 0;
}

dev_nginx::~dev_nginx() 
{

}

/* dev_nginx::access_write */
int dev_nginx::access_write(tag_cycle *cycle, std::string &result)
{
	std::string path = domain_path;
	path.append("/domain/");



	return 0;
}

/* dev_nginx::read_access */
int dev_nginx::access_read(tag_cycle *cycle, std::string &result)
{
	int ret(0);

	std::string path = domain_path;
	path.append("/domain/");

	std::vector<std::string> domain;
	if (read_dir(path, domain) == RET_ERROR) {
		return -1;
	}

	if (domain.size() == 0) {
		return -1;
	}

	result.clear();
	for (auto dir : domain)
	{
		path = domain_path;
		path += "/domain/";
		path += dir;
		path += "/";

		std::string	buff = "";
		read_logs_serial(path, 0);
		read_access_logs(path, buff);
		read_logs_serial(path, 1);

		/* send buff */
		int s = buff.size();
		if (s > 0) {
			ret += s;
			result += buff.data();
		}
	}

	return ret;
}


int dev_nginx::read_access_logs(const std::string & path, std::string &packct)
{
	std::string logs_file = path;
	logs_file += "/access.log";

	struct stat info;
	stat(logs_file.data(), &info);

	time_t	aTime;
	u_int	aSize;

	aTime = read_logs.access_time;
	aSize = read_logs.access_size;
	if (aTime != info.st_ctime) {
		read_logs.access_time = info.st_ctime;
		read_logs.access_size = 0L;
	}

	if (info.st_size == 0 || aSize > (u_int)info.st_size) {
		return RET_DONE;
	}

	std::ifstream if_read;
	if_read.open(logs_file.data(), std::ios::out);
	if (if_read.is_open() != true) {
		return RET_ERROR;
	}

	if_read.seekg(aSize, std::ios::beg);
	packct.clear();

	int count = 0;
	for (int i = 0; i < (int)max_size + 1; i++) {

		std::string line;
		if (!getline(if_read, line)) {
			break;
		}

		count++;

		packct.append(line);
		packct.append("\r\n");
	}

	read_logs.access_size = (u_int)if_read.tellg();
	if_read.close();
	return RET_OK;
}


int dev_nginx::read_stream_logs(const std::string & path, std::string &packct)
{
	std::string logs_file = path;
	logs_file += "/stream.log";

	struct stat file_info;
	stat(logs_file.data(), &file_info);

	time_t logs_time;
	u_int  logs_size;

	logs_time = read_logs.stream_time;
	logs_size = read_logs.stream_size;

	if (logs_time != file_info.st_ctime) {

		read_logs.access_time = file_info.st_ctime;
		read_logs.access_size = 0L;
	}
	else if (file_info.st_size == 0 || logs_size == file_info.st_size) {
		return RET_DONE;
	}

	std::ifstream   if_read;
	if_read.open(logs_file.data(), std::ios::out);
	if (if_read.is_open() != true) {
		return RET_ERROR;
	}

	if_read.seekg(logs_size, std::ios::beg);

	packct.clear();
	int count = 0;

	for (int i = 0; i < (int)max_size; i++) {

		std::string line;
		if (!getline(if_read, line)) {
			break;
		}

		count++;
		packct.append(line);
		packct.append("\r\n");
	}

	read_logs.access_size = (u_int)if_read.tellg();

	if_read.close();
	return RET_OK;
}


int dev_nginx::serial_logs(const std::string & serial_file, u_int save)
{
	if (save == 1)
	{
		std::ofstream ofs(serial_file, std::ios::binary | std::ios::out);
		if (ofs.is_open() == false) {
			return RET_ERROR;
		}

		ofs.seekp(std::ios::beg);

		tag_dev_nginx_serial write_param;
		write_param.access_time = read_logs.access_time;
		write_param.access_size = read_logs.access_size;

		ofs.write((char *)&write_param, sizeof(tag_dev_nginx_serial));
		ofs.close();
	}
	else
	{
		std::ifstream ifs(serial_file, std::ios::binary | std::ios::in);
		if (ifs.is_open() == false) {
			return RET_ERROR;
		}

		ifs.seekg(0, std::ios::beg);

		tag_dev_nginx_serial read_param;
		ifs.read((char *)&read_param, sizeof(tag_dev_nginx_serial));
		ifs.close();

		read_logs.access_time = read_param.access_time;
		read_logs.access_time = read_param.access_time;
	}

	return RET_OK;
}

int dev_nginx::read_dir(const std::string & path, std::vector<std::string> &list)
{
#if (!WINX)
	DIR		*dir;
	struct	 dirent *ptr;

	if ((dir = opendir(path.data())) == nullptr) {
		return RET_ERROR;
	}

	while ((ptr = readdir(dir)) != nullptr)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
			continue;
		}
		else if (ptr->d_type == 8) {
			continue;
		}
		else if (ptr->d_type == 10) {
			continue;
		}
		else if (ptr->d_type == 4) {
			list.push_back(ptr->d_name);
		}
	}

	closedir(dir);
#endif
	return RET_OK;
}

int dev_nginx::read_logs_serial(const std::string & path, u_int flag)
{
	std::string serial_file = path;
	serial_file += "/read_logs.serial";

	return serial_logs(serial_file, flag);
}


time_t dev_nginx::get_time_d_my_hms(const std::string & time)
{
	std::string					w;
	std::vector<std::string>	words;

	static char month_array[13][5] = {
		"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec", "XXXX"
	};

	for (auto c : time) {

		if (c == '/' || c == ':' || c == ' ') {

			if (w.size() > 0) {
				words.push_back(w);
			}

			w.clear();
			continue;
		}

		w += c;
	}

	if (w.size() > 0) {
		words.push_back(w);
	}

	if (words.size() < 6) {
		return -1L;
	}

	int i = 0;
	for (i = 0; i < 13; i++)
	{
		if (strcmp(month_array[i], words[1].data()) == 0) {
			break;
		}
	}

	if (i == 13) {
		return -1L;
	}

	struct tm info;
	memset(&info, 0, sizeof(struct tm));

	info.tm_mday = atoi(words[0].data());
	info.tm_mon = i;
	info.tm_year = atoi(words[2].data());
	info.tm_hour = atoi(words[3].data());
	info.tm_min = atoi(words[4].data());
	info.tm_sec = atoi(words[5].data());

	info.tm_year = info.tm_year - 1900;
	info.tm_mon = info.tm_mon - 1;
	info.tm_isdst = -1;

	return mktime(&info);
}


bool dev_nginx::string_keep_digit(const std::string & src, std::string & digit)
{
	int space = 0;
	int count = 0;

	digit.clear();
	for (auto c : src)
	{
		if (isdigit(c) == 0) {
			space = (space == 0) ? (!space) : (space);
			continue;
		}

		if (space == 1) {
			digit += "\t";
			space = 0;
			count++;
		}

		digit += c;
	}

	return count;
}