/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "unix_config.h"
#include "unix_exception.h"

extern int			process_param_argc;
extern char		  **process_param_argv;
extern char		  **process_param_os_environ;

static char		   *process_param_os_argv_last;
extern char		  **process_param_os_argv;

#define SET_PROCTITLE_USES_ENV			1
#define SET_PROCTITLE_PAD				'\0'


/* process_option::copy_string */
static u_char * unix_copy_string(u_char *dst, u_char *src, u_int n)
{
	if (n == 0) {
		return dst;
	}

	while (--n) {
		*dst = *src;

		if (*dst == '\0') {
			return dst;
		}

		dst++;
		src++;
	}

	*dst = '\0';
	return dst;
}


int unix_proctitle_init()
{
	size_t			 size;
	unsigned int	 i;
	unsigned char	*p;

	size = 0;

	for (i = 0; environ[i]; i++) {
		size += strlen(environ[i]) + 1;
	}

	p = (unsigned char *)alloc_system_ptr(size);
	if (p == nullptr) {
		return -1;
	}

	process_param_os_argv_last = process_param_os_argv[0];

	for (i = 0; process_param_os_argv[i]; i++) {
		if (process_param_os_argv_last == process_param_os_argv[i]) {
			process_param_os_argv_last = process_param_os_argv[i] + strlen(process_param_os_argv[i]) + 1;
		}
	}

	for (i = 0; environ[i]; i++) {
		if (process_param_os_argv_last == environ[i]) {

			size = strlen(environ[i]) + 1;
			process_param_os_argv_last = environ[i] + size;

			(void) unix_copy_string((u_char *)p, (u_char *)environ[i], size);
			environ[i] = (char *)p;
			p += size;
		}
	}

	process_param_os_argv_last--;
	return 0;
}

int unix_set_proctitle(const char * title)
{
	u_char     *p;

	process_param_os_argv[1] = nullptr;

	p = unix_copy_string((u_char *)process_param_os_argv[0], (u_char *) "Asnode: ",
		process_param_os_argv_last - process_param_os_argv[0]);

	p = unix_copy_string(p, (u_char *)title, process_param_os_argv_last - (char *)p);

	if (process_param_os_argv_last - (char *)p) {
		memset(p, SET_PROCTITLE_PAD, process_param_os_argv_last - (char *)p);
	}

	return 0;
}