/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <string>

#include <module/modules.h>
#include <module/modules_config.h>
#include <alloc/alloc_pool.h>

#include "process.h"
#include "process_mgr.h"

/* top_process_option_cmdline */
process_option_cmdline_ptr	top_process_option_cmdline;


#define LINEFEED_SIZE       1
#define LINEFEED            "\x0a"

extern int				    process_param_argc;
extern char				  **process_param_argv;
extern char				  **process_param_os_argv;
extern char				  **process_param_os_environ;

extern std::string 			process_signal;
extern std::string			process_prefix;
extern std::string 			process_prefix_conf;
extern std::string			process_errorlog;
extern std::string 			process_conf_params;
extern std::string 			process_conf_file;

extern unsigned int			process_type;
extern unsigned int			process_option_help;
extern unsigned int			process_option_version;
extern unsigned int			process_option_show_configure;
extern unsigned int			process_option_test_config;
extern unsigned int			process_option_dump_config;
extern unsigned int			process_option_quiet_node;

extern unsigned int			process_option_command_line;
extern std::string			process_option_command_str;

extern std::string 			process_signal;
extern unsigned int			process_inherited;
extern unsigned int			process_daemonized;

/* process option */
class process_option
{
public:
	process_option();
	~process_option();

public:
	int parse(int argc, char *const *argv);
	int parse_command(int argc, char *const *argv);

public:
	static u_char * copy_string(u_char *dst, u_char *src, u_int n);

public:
	int save_file(int argc, char *const *argv);
	int show_help();
};

/* process_option::process_option */
process_option::process_option() {

}

/* process_option::process_option */
process_option::~process_option() {

}

/* process_option::copy_string */
u_char * process_option::copy_string(u_char *dst, u_char *src, u_int n)
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

/* process_option::parse */
int process_option::parse(int argc, char *const *argv)
{
	int rc = -1;

	rc = save_file(argc, argv);
	if (rc == -1) {
		return -1;
	}

	process_option_show_configure = 0xFFFFFFFF;

	process_option_test_config = 0;
	process_option_dump_config = 0;
	process_inherited = 0;

	rc = parse_command(argc, argv);
	if (rc == -1) {
		return -1;
	}

	return RET_DONE;
}

/* process_option::save_file */
int process_option::save_file(int argc, char *const *argv)
{
#if (!WINX)
	process_pid = getpid();
	process_parent = getppid();
#endif

	process_param_os_argv = (char **)argv;
	process_param_argc = (unsigned int)argc;

	process_param_argv = (char **) alloc_system_ptr((argc + 1) * sizeof(char *));
	if (process_param_argv == nullptr) {
		return -1;
	}

	int			 i;
	unsigned int len;

	for (i = 0; i < argc; i++) {

		len = strlen(argv[i]) + 1;

		process_param_argv[i] = (char *) alloc_system_ptr(len);
		if (process_param_argv[i] == nullptr) {
			return -1;
		}

		(void)copy_string((u_char *)process_param_argv[i], (u_char *)argv[i], len);
	}

	process_param_argv[i] = nullptr;
	process_param_os_environ = environ;
	return 0;
}

/* process_option::save_file */
int process_option::parse_command(int argc, char *const *argv)
{
	char 		*p;
	int			 i;

	if (argc < 2) {
		return RET_DONE;
	}

	for (i = 1; i < argc; i++) {

		p = argv[i];

		if (*p++ != '-') {
			return -1;
		}

		while (*p) {

			switch (*p++) {

			case '?':
			case 'h':
				process_option_help = 1;
				process_option_version = 1;
				break;

			case 'v':
				process_option_version = 1;
				break;

			case 'V':
				process_option_version = 1;
				process_option_show_configure = 1;
				break;

			case 't':
				process_option_test_config = 1;
				break;

			case 'T':
				process_option_test_config = 1;
				process_option_dump_config = 1;
				break;

			case 'i':
				process_option_command_str.clear();
				process_option_command_line = 1;

				if (argv[++i]) {
					process_option_command_str = (char *)argv[i];
					goto next;
				}

				break;
			case 'p':
				if (*p) {
					process_prefix = p;
					goto next;
				}

				if (argv[++i]) {
					process_prefix = (char *)argv[i];
					goto next;
				}
				return RET_ERROR;

			case 'g':
				if (*p) {
					process_conf_params = p;
					goto next;
				}

				if (argv[++i]) {
					process_conf_params = (char *)argv[i];
					goto next;
				}
				return RET_ERROR;

			case 'c':
				if (*p) {
					process_prefix_conf = p;
					goto next;
				}

				if (argv[++i]) {
					process_prefix_conf = (char *)argv[i];
					goto next;
				}
				return RET_OK;

			case 's':
				if (*p) {
					process_signal = (char *)p;
				}
				else if (argv[++i]) {
					process_signal = (char *)argv[i];
				}
				else {
					return RET_OK;
				}

				if (strcmp(process_signal.data(), "stop") == 0
					|| strcmp(process_signal.data(), "quit") == 0
					|| strcmp(process_signal.data(), "reopen") == 0
					|| strcmp(process_signal.data(), "reload") == 0)
				{
					process_type = PROCESS_SIGNALLER;
					goto next;
				}

				fprintf(stderr, u8R"("invalid option: "-s %s" ")" "\n", process_signal.data());
				return RET_ERROR;

			default:

				fprintf(stderr, u8R"("invalid option: "%c" ")" "\n", *(p - 1));
				return RET_ERROR;
			}
		}

	next:
		continue;
	}

	return RET_OK;
}

/* process_option::show_help */
int process_option::show_help()
{
	fprintf(stderr, "As Could version: " MODULE_VERSION LINEFEED);

	if (process_option_help)
	{
		fprintf(stderr, LINEFEED
			"Usage: AsRepository [-?hvVtTq]" LINEFEED
			"			  [-s signal]" LINEFEED
			"             [-c filename]" LINEFEED
			LINEFEED

			"Options:" LINEFEED
			"  -?,-h         : this help" LINEFEED
			"  -v            : show version and exit" LINEFEED
			"  -V            : show version and configure options then exit" LINEFEED
			"  -t            : test configuration and exit" LINEFEED
			"  -T            : test configuration, dump it and exit"
			"  -i            : command line to test function." LINEFEED
			"  -I            : command line to test function, dump it." LINEFEED
			LINEFEED
			"  -q            : suppress non-error messages "
								"during configuration testing" LINEFEED
			"  -s signal     : send signal to a master process:"
								"stop, quit, reopen, reload" LINEFEED

#ifdef MODULE_PREFIX
			"  -p prefix     : set prefix path (default: " MODULE_PREFIX ")" LINEFEED
#else
			"  -p prefix     : set prefix path (default: NONE)" LINEFEED
#endif

#ifdef MODULE_PREFIX_CONF
			"  -c filename   : set configuration file (default:" MODULE_PREFIX_CONF ")" LINEFEED
#else
			"  -c filename   : set configuration file (default: NONE)" LINEFEED
#endif

			LINEFEED);
	}

	if (process_option_show_configure) {
		fprintf(stderr, "configuration file(default: conf/server.node)" LINEFEED);
	}

	return 0;
}

/* process_option_parse */
int process_option_parse(int argc, char *const *argv)
{
	int				ret = 0;
	process_option	option;
	
	ret = option.parse(argc, argv);
	if (ret == -1) {
		return RET_ERROR;
	}

	if (process_option_show_configure != 0xFFFFFFFF) {
		return option.show_help();
	}

	if (process_option_command_line == 1) {
		process_option_test_config = 1;
		return RET_DONE;
	}

	return ret;
}
