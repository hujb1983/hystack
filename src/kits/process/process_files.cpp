/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#include "process_mgr.h"


 /* process files */
class process_files
{
public:
	std::string		path;
	unsigned int	mode;
	int				fd;

public:
	process_files();
	virtual ~process_files();

	int open(const char * filepath, int mode);
	int close();

public:
	int write(const char * buf, unsigned int len, unsigned int offset);
	int read(const char * buf, unsigned int len, unsigned int offset);

public:
	int is_exist();
	int create();
};


/* process_files::process_files */
process_files::process_files() {
	path.clear();
	mode = 0;
	fd = -1;
}

/* process_files::~process_files */
process_files::~process_files() {

}

/* process_files::open */
int process_files::open(const char * _path, int _mode)
{
#if (!WINX)
	fd = ::open(_path, O_CREAT | O_RDWR | O_APPEND, 0777);
	if (fd != -1) {
		return -1;
	}

	path = _path;
	mode = _mode;
#endif	
	return 0;
}

/* process_files::close */
int process_files::close()
{
#if (!WINX)
	::close(fd);
#endif	
	return 0;
}

/* process_files::write */
int process_files::write(const char * buf, unsigned int len, unsigned int offset)
{
#if (!WINX)
	unsigned int    n;

	if (fd != -1) {
		n = ::write(fd, buf, len);
		if (n == 0xFFFFFFFF) {
			return -1;
		}
	}
#endif	

	return 0;
}

/* process_files::read */
int process_files::read(const char * buf, unsigned int len, unsigned int offset)
{
	return -1;
}

/* process_files::is_exist */
int process_files::is_exist()
{
#if (!WINX)
	if (::access(path.data(), 0) != 0) {
		return 0;
	}
#endif
	return -1;
}

/* process_files::create */
int process_files::create()
{
#if (!WINX)
	if (::access(path.data(), 0) != 0)
	{
		if (::mkdir(path.data(), mode | 0777) == -1) {
			return -1;
		}
		return 0;
	}
#endif
	return -1;
}
