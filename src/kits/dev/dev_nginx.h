/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2022
 */

#include <core.h>
#include <core_config.h>

 /* tag_serial */
typedef struct {
	time_t			access_time;
	u_int			access_size;

	time_t			stream_time;
	u_int			stream_size;

	u_int			deal_times;
	u_int			send_size;

} tag_dev_nginx_serial;


/* dev_nginx */
class dev_nginx
{
public:
	std::string				domain_path;
	u_int					max_size;
	u_int					time_out;
	tag_dev_nginx_serial	read_logs;

public:
	dev_nginx() = delete;
	dev_nginx(std::string & damain_path);
	virtual ~dev_nginx();

	/* read_access */
	int access_read(tag_cycle *cycle, std::string &result);
	int access_write(tag_cycle *cycle, std::string &result);

private:
	int read_access_logs(const std::string & path, std::string &buff);
	int read_stream_logs(const std::string & path, std::string &buff);
	int read_dir(const std::string & path, std::vector<std::string> &list);
	int read_logs_serial(const std::string & path, u_int flag);
	int serial_logs(const std::string & path, u_int save);

private:
	bool	string_keep_digit(const std::string & src, std::string & digit);
	time_t	get_time_d_my_hms(const std::string & time);
};