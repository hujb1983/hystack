/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   27, SEP, 2021
 */

#include "unix_config.h"
#include "unix_exception.h"

#if (!WINX)
extern pid_t					process_parent;
extern pid_t					process_pid;
extern pid_t					process_new_binary;
#else 
extern int						process_parent;
extern int						process_pid;
extern int						process_new_binary;
#endif

unsigned int unix_daemon()
{
#if (!WINX)
	int  fd;

	switch (::fork())
	{
	case -1:
		return UNIX_RET_F("fork() failed", -1);
	case 0:
		break;
	default:
		::exit(0);
	}

	process_parent = process_pid;
	process_pid = getpid();

	if (setsid() == -1) {
		return UNIX_RET_F("setsid() failed", -1);
	}

	umask(0);

	fd = open("/dev/null", O_RDWR);
	if (fd == -1) {
		return UNIX_RET_F("open(\"/dev/null\") failed", -1);
	}

	if (dup2(fd, STDIN_FILENO) == -1) {
		return UNIX_RET_F("dup2(STDIN) failed", -1);
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		return UNIX_RET_F("dup2(STDOUT) failed", -1);
	}

	if (fd > STDERR_FILENO)
	{
		if (close(fd) == -1) {
			return UNIX_RET_F("close() failed", -1);
		}
	}
#endif
	return UNIX_RET_S(0);
}
