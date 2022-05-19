/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <mutex>
#include <string>
#include <fstream>
#include <chrono>
using namespace std;
using namespace chrono;

#include "logs.h"
#include "logs_config.h"
#include "logs_stream.h"

#include <core_config.h>
#include <core_times.h>

#include <alloc/alloc_buffer.h>
#include <alloc/alloc_pool.h>

/* tag_log logs_root */
extern tag_log logs_root;

typedef struct ST_LOGS_STRING		tag_logs_string;
typedef struct ST_LOGS_HANDLER		tag_logs_handler;

const unsigned int	logs_handler_info_slots = 8L;
unsigned char		logs_handler_info_times[logs_handler_info_slots]
[sizeof("1970-09-28 12:00:00") + 1] = { 0 };

/* get_update_times */
void get_update_times(unsigned short slot = 0)
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

	char * ptr = (char *)logs_handler_info_times[0];
	sprintf(ptr, "%4d-%02d-%02d %02d:%02d:%02d", years, months + 1, mday, hour, minutes, seconds);
}

/* ST_LOGS_HANDLER */
struct ST_LOGS_HANDLER
{
	unsigned int		 level;

	unsigned char		*data;
	unsigned int		 size;

	tag_log				*log;
	std::mutex			 thread_mutex;
};


/*
static const int LOG_MAX_FILESIZE = (1024L * 1024L * 1024L); */
static const long LOG_MAX_FILESIZE = (1000L * 1000L * 1000L * 2L);


/* ST_LOGS_STRING */
struct ST_LOGS_STRING {
	unsigned int	 len;
	unsigned char	*data;
};


/* init_logs_string */
#define init_logs_string(str)		{ sizeof(str) - 1, (unsigned char *)str }
#define init_logs_string_null		{ 0, nullptr }


/* error level */
static tag_logs_string error_level_string[] = {

	init_logs_string("STDERR"),
	init_logs_string("EMERG"),
	init_logs_string("ALERT"),
	init_logs_string("CRIT"),
	init_logs_string("ERROR"),
	init_logs_string("WARN"),
	init_logs_string("NOTICE"),
	init_logs_string("INFO"),
	init_logs_string("TRACE"),
	init_logs_string("DEBUG"),
	init_logs_string_null
};

#if (!WINX)
extern pid_t process_parent;
extern pid_t process_pid;
#endif


/* logs_stream */

logs_stream::logs_stream() : _stream("")
{

}

logs_stream::logs_stream(unsigned char * pos, unsigned char * last)
{
	if (pos == nullptr || last == nullptr) {
		return;
	}

	_stream.append((char *)pos, last - pos);
}

logs_stream::logs_stream(const std::string & data, unsigned int level) : _stream("")
{
	int	pid = 0;

#if (!WINX)
	pid = process_pid;
#endif

	char temp[32] = { 0 };
	snprintf(temp, sizeof(temp), " %d:%s ", pid, (char *)error_level_string[level].data);

	_stream = (char *)core_cached_errorlog_time.data;
	_stream += temp;
	_stream += data;
	_stream += "\n";
}

logs_stream::~logs_stream() {

}

unsigned int logs_stream::write(std::string & _path) {
	return write(_path.data());
}

unsigned int logs_stream::write(const char * path)
{
	std::fstream	_wf;
	_wf.open(path, std::ios::out | std::ios::app);
	if (_wf.is_open() == false) {

		fprintf(stderr, "%s open() failed. %s\r\n",
			(char *)core_cached_errorlog_time.data,
			path);

		return 0;
	}

	_wf.seekg(0, _wf.end);

	unsigned int fSize = 0L;
	fSize = (unsigned int)_wf.tellg();

	_wf.clear();
	_wf << _stream;
	_wf.close();

	if (fSize > LOG_MAX_FILESIZE)
	{
		std::string fnew;
		fnew = path;
		fnew += ".bak";
		rename(path, fnew.data());
		return 0L;
	}

	return fSize;
}

const char  *logs_stream::data() {
	return _stream.data();
}

unsigned int logs_stream::size() {
	return _stream.size();
}

void logs_stream::print() {
	fprintf(stderr, "%s", _stream.data());
}


/* logs_handler_console */
int logs_handler_console(tag_log *log, int level, u_char * data, u_short size) 
{
	if (log != nullptr) {
		logs_stream((char *)data, level).print();
	}

	return 0;
}

/* logs_handler_write_files */
int logs_handler_write_files(tag_log *log, int level, u_char * data, u_short size) {

	if (log->level != level) {
		return 0;
	}

	log->file_size = logs_stream((char *)data, level).write(log->file_path);
	return 0;
}


/* logs_handler_write_buffer */
int logs_handler_write_buffer(tag_log *log, int level, u_char * data, u_short size) 
{
	int			len = 0;
	int			need = 0;

	if (log->level != level) {
		return 0;
	}

	logs_stream ss((char *)data, level);
	if (ss.size() == 0) {
		return 0;
	}

	tag_buffer *buf = (tag_buffer*)log->data;
	len = (buf->ptr_end - buf->ptr_last);

	need = size + 0x3f;
	if (need > len) {
		log->file_size = logs_stream((char *)data, level).write(log->file_path);
		buf->ptr_last = buf->ptr_start;
	}

	if (need > (int)const_logs_page_size) {
		log->file_size = ss.write(log->file_path);
		buf->ptr_last = buf->ptr_start;
		return 0;
	}

	memcpy((char *)buf->ptr_last, ss.data(), ss.size());
	buf->ptr_last = buf->ptr_last + ss.size();
	return 0;
}


/* logs_init */
tag_log * logs_init(unsigned char *prefix, unsigned char * error_log)
{
	logs_root.level = logs::Stderr;
	logs_root.ptr_handler = logs_handler_console;
	logs_root.buffer = nullptr;
	logs_root.ptr_writer = nullptr;
	logs_root.data = nullptr;
	logs_root.file_name = "error.log";
	logs_root.file_path = (char *)prefix;

#if (!WINX)
	if (::access(logs_root.file_path.data(), 0) == -1) {
		::mkdir(logs_root.file_path.data(), 0777);
	}
#endif

	logs_root.file_path += "/logs/";

#if (!WINX)
	if (::access(logs_root.file_path.data(), 0) == -1) {
		::mkdir(logs_root.file_path.data(), 0777);
	}
#endif

	logs_root.file_path += logs_root.file_name;
	logs_root.nickname = (char *)error_log;
	logs_root.ptr_next = nullptr;

	return &logs_root;
}

/* logs_print */
int logs_print(tag_log * log, int level, const std::string & _error)
{
	int				 size;
	unsigned char	*buff;

	if (log == nullptr) {
		log = &logs_root;
	}

	size = _error.size();
	buff = (unsigned char *) const_cast<char *> (_error.data());

	while (log) {

		{
			std::lock_guard<std::mutex> lock(log->mutex);

			if (log->level != level) {
				goto next_log;
			}

			if (log->ptr_handler) {
				if (log->ptr_handler(log, level, buff, size) == 0) {
					goto next_log;
				}
			}

			if (log->ptr_writer) {
				log->ptr_writer(log, level, buff, size);
			}
		}

	next_log:
		log = log->ptr_next;
	}

	return 0;
}
