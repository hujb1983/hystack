/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std;

#include "dev_stat.h"
#include <inet/inet_socket.h>

#if (WINX)
#include <windows.h>
#else
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#endif

#include <chrono>
using namespace std;
using namespace chrono;

const unsigned int	dev_stat_info_slots = 8L;
unsigned char		dev_stat_info_times[dev_stat_info_slots]
					[sizeof("1970-09-28 12:00:00") + 1] = { 0 };

/* get_update_times */
static void get_update_times(unsigned short slot = 0)
{
	struct tm	*ptm;
	auto tt = system_clock::to_time_t(system_clock::now());

	ptm = localtime(&tt);
	unsigned int seconds = (unsigned int)ptm->tm_sec;
	unsigned int minutes = (unsigned int)ptm->tm_min;
	unsigned int hour = (unsigned int)ptm->tm_hour;
	unsigned int mday = (unsigned int)ptm->tm_mday;
	unsigned int months = (unsigned int)ptm->tm_mon;
	unsigned int years = (unsigned int)ptm->tm_year + 1900;

	char * ptr = (char *)dev_stat_info_times[0];
	sprintf(ptr, "%4d-%02d-%02d %02d:%02d:%02d", years, months + 1, mday, hour, minutes, seconds);
}

/* dev_ptr_i */
class dev_ptr_i
{
public:
	int		 fd;

	dev_ptr_i() = delete;
	dev_ptr_i(int f) {
		fd = f;
	}

	~dev_ptr_i() {
#if (!WINX)
		::close(fd);
#endif
	}
};

 /* dev_ptr_f */
 class dev_ptr_f
 {
 public:
	 FILE	*fd;

	 dev_ptr_f() = delete;
	 dev_ptr_f(FILE * pfile)  {
		 fd = pfile;
	 }

	 ~dev_ptr_f() {
#if (!WINX)
		 ::fclose(fd);
#endif
	 }
 };

 /* dev_ptr_p */
 class dev_ptr_p
 {
 public:
	 FILE	*fd;

	 dev_ptr_p() = delete;
	 dev_ptr_p(FILE * pfile)  { 
		 fd = pfile;
	 }
	 ~dev_ptr_p() {
#if (!WINX)
		 pclose(fd);
#endif
	 }
 };


/* tag_dev_stat */
typedef struct ST_DEV_STAT			tag_dev_stat;
typedef struct ST_DEV_STAT_INFO		tag_dev_stat_info;


/* ST_DEV_STAT */
struct ST_DEV_STAT 
{
	std::string				disk_path;
	unsigned int			disk_block;
	unsigned int			disk_nodes;

	long long				cpu_idl;
	long long				cpu_sys;
	unsigned int			cpu_user;
	unsigned int			cpu_nice;
	unsigned int			cpu_system;
	unsigned int			cpu_idle;

	std::string				mem_path;
	long long				mem_total;
	long long				mem_free;;
							
	std::string				net_path;
	long int				net_last_recv_size;
	long int				net_last_sent_size;
	long int				net_total_recv_size;
	long int				net_total_sent_size;

	/* stat stub */
	unsigned int			stub_collected;
	unsigned int			stub_readings;
	unsigned int			stub_writings;
	unsigned int			stub_waitings;
	unsigned int			stub_conections;

	std::vector<int>		process_ids;
	unsigned int			connected;
	unsigned int			connections;
};


/* ST_DEV_STAT */
struct ST_DEV_STAT_INFO
{
	/* stat load */
	long long			mem_total_size;
	long long			mem_free_size;

	long long			cpu_load_rate;
	long long			sys_load_rate;

	/* stat net */
	char 				net_card_name[32];
	long int			net_total_recv_size;
	long int			net_total_sent_size;

	/* stat root */
	long long			root_disk_block_rate;
	long long			root_disk_used_rate;
	long long			boot_disk_block_rate;
	long long			boot_disk_used_rate;

	/* stat stub */
	unsigned int		stub_collected;
	unsigned int		stub_readings;
	unsigned int		stub_writings;
	unsigned int		stub_waitings;
	unsigned int		stub_conections;

	/* stat tcp */
	unsigned int		connected;
	unsigned int		connections;
};


 /* dev_stat*/
class dev_stat
{
public:
	void collection_cpu(tag_dev_stat * dev);
	void collection_mem(tag_dev_stat * dev);
	void collection_disk(tag_dev_stat * dev);
	void collection_net(tag_dev_stat * dev);
	void collection_stub(tag_dev_stat * dev);
	void collection_tcp(tag_dev_stat * dev);

public:
	int do_stub(std::string &body);

public:
	int do_tcp(tag_dev_stat * tcp);
	int tcps(tag_dev_stat * tcp, const std::string &tcp_path);
	int pids(tag_dev_stat * tcp, std::vector<int> & pids);

public:
	void bejson(tag_dev_stat_info *info, u_char *data, u_short size);
};


void dev_stat::collection_cpu(tag_dev_stat * dev) {

	char	 name[20] = { 0 };
	FILE	*fd = nullptr;

	fd = fopen("/proc/stat", "r");
	if (fd == nullptr) {
		return;
	}

	dev_ptr_f guard(fd);

	long unsigned int	user, nice, system, idle;
	char				buff[0x8FF] = { 0 };

	fgets(buff, sizeof(buff), fd);
	sscanf(buff, "%s %lu %lu %lu %lu", name, &user, &nice, &system, &idle);

	unsigned int _cpu = 0, _sys = 0, od, nd, id, sd;
	od = dev->cpu_user + dev->cpu_nice + dev->cpu_system + dev->cpu_idle;
	nd = user + nice + system + idle;
	id = user - dev->cpu_user;
	sd = system - dev->cpu_system;

	if ((nd - od) > 0) {
		_cpu = (int)((sd + id) * 10000) / (nd - od);
		_sys = (int)(sd * 10000) / (nd - od);
	}

	dev->cpu_idl = _cpu;
	dev->cpu_sys = _sys;

	dev->cpu_user = user;
	dev->cpu_nice = nice;
	dev->cpu_system = system;
	dev->cpu_idle = idle;
}


void dev_stat::collection_mem(tag_dev_stat * dev)
{
#if (!WINX)
	struct sysinfo mem;

	int ret = sysinfo(&mem);
	if (ret < 0) {
		return;
	}

	long long total_size = mem.totalram;
	dev->mem_total = total_size / 1024;

	long long free_size = mem.freeram;
	dev->mem_free = free_size / 1024;
#endif
}


/* dev_stat::collection_disk */
void dev_stat::collection_disk(tag_dev_stat * dev)
{
#if (!WINX)
	struct statfs disk;

	int ret = statfs(dev->disk_path.data(), &disk);
	if (ret < 0) {
		return;
	}

	long long total = 0L, free = 0L;

	total = (disk.f_blocks * disk.f_bsize) / 1024;
	free = (disk.f_bfree * disk.f_bsize) / 1024;
	dev->disk_block = (total - free) * 10000 / total;

	total = disk.f_files;
	free = disk.f_ffree;
	dev->disk_nodes = (total - free) * 10000 / total;
#endif
}


/* dev_stat::collection */
void dev_stat::collection_net(tag_dev_stat * dev)
{
	FILE	*fd = nullptr;
	char	 buff[0x6FF] = { 0 };
	char	 data[0x6FF] = { 0 };
	char	*match = nullptr;
	char	 name[20] = { 0 };
	long int recv_size, r[7], sent_size;

	fd = fopen(dev->net_path.data(), "r");
	if (fd == nullptr) {
		return;
	}

	/* guard */
	dev_ptr_f _guard(fd);

	std::string		line;
	stringstream	stream;

	int size = fread(buff, 1, sizeof(buff), fd);
	if (size == 0) {
	}

	stream << buff;
	while (getline(stream, line))
	{
		int _min = (line.size(), sizeof(data)) ? (line.size()) : (sizeof(data));
		::memcpy(data, line.data(), _min);

		match = strstr(data, ":");
		if (match == nullptr) {
			continue;
		}

		::memcpy(name, data, match - data);
		dev->net_path = name;

		sscanf(match, ":%ld %ld %ld %ld %ld %ld %ld %ld %ld",
			&recv_size, &r[0], &r[1], &r[2], &r[3], &r[4], &r[5], &r[6], &sent_size);

		if (recv_size == 0L && sent_size == 0L) {
			continue;
		}

		if ((recv_size < dev->net_last_recv_size) || (sent_size < dev->net_last_recv_size)) {
			continue;
		}

		dev->net_total_recv_size = recv_size - dev->net_last_recv_size;
		dev->net_total_recv_size = sent_size - dev->net_last_recv_size;

		dev->net_last_recv_size = recv_size;
		dev->net_last_recv_size = sent_size;
	}
}

/* dev_stat::collection_stub */
void dev_stat::collection_stub(tag_dev_stat * dev)
{
	
}

/* dev_stat::collection_tcp */
void dev_stat::collection_tcp(tag_dev_stat * dev)
{

}

/* dev_stat::do_stub */
int dev_stat::do_stub(std::string &_body)
{
	std::string pack = "";
	pack += "GET /nginx-status HTTP/1.1\r\n";
	pack += "Host: localhost\r\n";
	pack += "User-Agent: PostmanRuntime/7.29.0\r\n";
	pack += "Accept: */* \r\n";
	pack += "Accept-Encoding: gzip, deflate, br\r\n";
	pack += "ContentType: application/x-www-form-urlencoded;charset=UTF-8\r\n";
	pack += "\r\n";

	inet_handler *h_socket = nullptr;
	h_socket = inet_socket_new(INET_SOCKET_UNX);
	if (h_socket == nullptr) {
		return -1;
	}

	/* connect socket */
	int ret_conn = inet_socket_connect(h_socket, "/var/run/nginx/nginx.sock");
	if (ret_conn == -1) {
		return -1;
	}

	/* send socket */
	int ret_sent = inet_socket_send(h_socket, (unsigned char *)pack.data(), (unsigned short)pack.size());
	if (ret_sent != (int)pack.size()) {
		return -1;
	}

	/* recv socket */
	unsigned char r_buff[2048] = { 0 };
	int ret_recv = inet_socket_recv(h_socket, r_buff, sizeof(r_buff));
	if (ret_recv != (int)pack.size()) {
		return -1;
	}

	/* close socket */
	inet_socket_delete(h_socket);
	return 0;
}

/* dev_stat::do_tcp */
int dev_stat::do_tcp(tag_dev_stat * tcp)
{
	char path[0x2f] = { 0 };

	tcp->connected = 0;
	if (tcp->process_ids.size() == 0) {
		pids(tcp, tcp->process_ids);
		if (tcp->process_ids.size() <= 1) {
			return -1;
		}
	}

	tcp->connections = 0L;

	std::vector<int> _check = tcp->process_ids;
	tcp->process_ids.clear();

	for (auto idx : _check) {

		snprintf(path, 0x8f, "/proc/%d/net/tcp", (int)idx);

		std::vector<int> _check = tcp->process_ids;
		if (tcps(tcp, path) == 0) {
			continue;
		}

		tcp->connected = 1L;
		tcp->process_ids.push_back(idx);
	}

	return 0;
}


/* dev_stat::tcps */
int dev_stat::tcps(tag_dev_stat * tcp, const std::string &tcp_path)
{

#if (!WINX)

	char	 buf[0x3FF] = { 0 };
	FILE	*fp = nullptr;

	fp = fopen(tcp_path.data(), "r");
	if (fp == nullptr) {
		return -1;
	}

	dev_ptr_f gurad(fp);

	if (fgets(buf, sizeof(buf), fp) == nullptr) {
		return -1;
	}

	std::string				 word;
	std::vector<std::string> words;

	while (fgets(buf, sizeof(buf), fp) != nullptr)
	{
		word.clear();
		words.clear();

		for (auto c : buf) {

			if (c == ' ' || c == '\t') {

				if (word.size() > 0) {
					words.push_back(word);
					word.clear();
				}
				continue;
			}

			word += (c);
		}

		if (words.size() < 16) {
			continue;
		}

		if (strcmp(words[3].data(), "01") == 0) {
			tcp->connections++;
		}
	}
#endif

	return tcp->connections;
}


/* dev_stat::bejson */
int dev_stat::pids(tag_dev_stat * tcp, std::vector<int> & pids)
{

#if (!WINX)
	FILE		*fp = nullptr;
	const char	*cmd = "ps -ef | grep 'nginx: worker process' | awk '{print $2}'";

	if ((fp = popen(cmd, "r")) == nullptr) {
		return -1;
	}

	pids.clear();
	dev_ptr_p gurad(fp);

	char b[8] = { 0 };
	while (fgets(b, 8, fp) != nullptr)
	{
		pids.push_back(atoi(b));
		memset(b, 0, 8);
	}
#endif

	return pids.size();
}


/* dev_stat_info */
tag_dev_stat			param;
tag_dev_stat_info	    info;


/* dev_stat_info */
void dev_stat_info()
{
	get_update_times(0);

	std::shared_ptr<dev_stat>	object;
	object = std::make_shared<dev_stat>();

	/* device_cpu */
	object->collection_cpu(&param);
	info.cpu_load_rate = param.cpu_idl;
	info.sys_load_rate = param.cpu_sys;

	/* device_mem */
	object->collection_mem(&param);
	info.mem_total_size = param.mem_total;
	info.mem_free_size = param.mem_free;

	/* device_disk */
	param.disk_path = "/";
	object->collection_disk(&param);
	info.root_disk_block_rate = param.disk_block;
	info.root_disk_used_rate = param.disk_nodes;

	/* device_disk */
	param.disk_path = "/boot";
	object->collection_disk(&param);
	info.boot_disk_block_rate = param.disk_block;;
	info.boot_disk_used_rate = param.disk_nodes;;

	/* device_net */
	param.net_path = "/proc/net/dev";
	object->collection_net(&param);
	info.net_total_recv_size = param.net_total_recv_size;
	info.net_total_sent_size = param.net_total_sent_size;;

	/* node_stat_stub */
	object->collection_stub(&param);
	info.stub_collected = param.stub_collected;
	info.stub_readings = param.stub_readings;
	info.stub_writings = param.stub_writings;
	info.stub_waitings = param.stub_waitings;
	info.stub_conections = param.stub_conections;

	/* node_stat_tcp */
	object->do_tcp(&param);
	info.connected = param.connected;
	info.connections = param.connections;
}


/* dev_stat_bejson */
void dev_stat_bejson(unsigned char * data, unsigned short size)
{
	std::string	text;

	text = u8R"({)";
	text += u8R"("resource_usage":)";
	text += u8R"({)";
	text += u8R"("load":)";

	// load data...
	[](tag_dev_stat_info * info, std::string & load) {

		int mem = 0;
		if (info->mem_total_size != 0L) {
			mem = (int)((info->mem_total_size - info->mem_free_size) * 10000 / info->mem_total_size);
		}

		load += u8R"([{)";
		load += u8R"("mem":")" + std::to_string(mem) + u8R"(",)";
		load += u8R"("cup":")" + std::to_string(info->cpu_load_rate) + u8R"(",)";
		load += u8R"("sys":")" + std::to_string(info->sys_load_rate) + u8R"(",)";
		load += u8R"("timestamp":")";
		load += (char *)dev_stat_info_times[0];
		load += u8R"("}])";

	}(&info, text);

	text += u8R"(,)";
	text += u8R"("disk":)";

	// disk data...
	[](tag_dev_stat_info * info, std::string &disk) {

		disk += u8R"([{)";
		disk += u8R"("partition":"/",)";
		disk += u8R"("space_ratio":")" + std::to_string(info->root_disk_block_rate) + u8R"(",)";
		disk += u8R"("inode_ratio":")" + std::to_string(info->root_disk_used_rate) + u8R"(",)";
		disk += u8R"("timestamp":")";
		disk += (char *)dev_stat_info_times[0];
		disk += u8R"("},)";

		std::string dataBoot;
		disk += u8R"({)";
		disk += u8R"("partition":"/boot",)";
		disk += u8R"("space_ratio":")" + std::to_string(info->boot_disk_block_rate) + u8R"(",)";
		disk += u8R"("inode_ratio":")" + std::to_string(info->boot_disk_used_rate) + u8R"(",)";
		disk += u8R"("timestamp":")";
		disk += (char *)dev_stat_info_times[0];
		disk += u8R"("}])";

	}(&info, text);

	text += u8R"(,)";
	text += u8R"("tcp":)";

	// tcp data...
	[](tag_dev_stat_info * info, std::string &_tcp) {

		_tcp += u8R"([{)";
		_tcp += u8R"("connection":")" + std::to_string(info->connections) + u8R"(",)";
		_tcp += u8R"("timestamp":")";
		_tcp += (char *)dev_stat_info_times[0];
		_tcp += u8R"("}])";

	}(&info, text);

	text += u8R"(,)";
	text += u8R"("bandwidth":)";

	// net data...
	[](tag_dev_stat_info * info, std::string &_net) {

		_net += u8R"([{)";
		_net += u8R"("card":")";
		_net += info->net_card_name;
		_net += u8R"(",)";
		_net += u8R"("outbound":")" + std::to_string(info->net_total_recv_size) + u8R"(",)";
		_net += u8R"("inbound":")" + std::to_string(info->net_total_sent_size) + u8R"(",)";
		_net += u8R"("timestamp":")";
		_net += (char *)dev_stat_info_times[0];
		_net += u8R"("}])";

	}(&info, text);

	text += u8R"(,)";
	text += u8R"("nginx":)";

	// stub data...
	[](tag_dev_stat_info * info, std::string &stub) {

		stub += u8R"([{)";
		stub += u8R"("collected":")" + std::to_string(info->stub_collected) + u8R"(",)";
		stub += u8R"("connecting":")" + std::to_string(info->stub_conections) + u8R"(",)";
		stub += u8R"("reading":")" + std::to_string(info->stub_readings) + u8R"(",)";
		stub += u8R"("writing":")" + std::to_string(info->stub_writings) + u8R"(",)";
		stub += u8R"("waiting":")" + std::to_string(info->stub_waitings) + u8R"(",)";
		stub += u8R"("timestamp":")";
		stub += (char *)dev_stat_info_times[0];
		stub += u8R"("}])";

	}(&info, text);

	text += u8R"(},)";
	text += u8R"("timestamp":")";
	text += (char *)dev_stat_info_times[0];
	text += u8R"(",)";
	text += u8R"("version":"0.1.0")";
	text += u8R"(})";

	int min_size = text.size();
	min_size = (min_size < size) ? (min_size) : (size);
	memcpy(data, text.c_str(), min_size);
}
