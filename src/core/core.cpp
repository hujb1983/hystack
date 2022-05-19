/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#include "core.h"
#include "core_config.h"

#include <alloc/alloc_pool.h>
#include <event/event_connection.h>

#include <module/modules_core.h>
#include <module/modules_conf.h>
#include <module/modules_config.h>

#include <process/process_mgr.h>


extern tag_module		core_module;

extern unsigned int		process_type;
extern unsigned int		process_worker;

extern unsigned int		process_option_help;
extern unsigned int		process_option_version;
extern unsigned int		process_option_show_configure;
extern unsigned int		process_option_test_config;
extern unsigned int		process_option_dump_config;
extern unsigned int		process_option_quiet_node;

char				  **process_environ;

core::core() {

}

core::~core() {

}

int core::create_pid(const std::string & name)
{

#if (!WINX)
	int		rc;
	size_t	len;
	unsigned char	current_pid[INT64_LEN + 2];

	if (core_process > PROCESS_MASTER) {
		return 0;
	}

	int fd = open((char *)name.data(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd == -1) {
		return -1;
	}

	if (!process_option_test_config) {

		memset(current_pid, 0x0, sizeof(current_pid));
		len = snprintf((char *)current_pid, INT64_LEN + 2, "%d", process_pid);

		rc = pwrite(fd, current_pid, len, 0);
		if (rc == -1) {
			close(fd);
			return -1;
		}
	}
#endif

	return 0;
}

int core::delete_pid(tag_cycle *cycle)
{
#if (!WINX)
	unsigned char	*name;
	
	tag_core_conf	*ccf;
	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	name = (unsigned char *)(process_new_binary ? ccf->old_pid.data() : ccf->pid.data());
	if (unlink((char *)name) == 0) {
		std::string err = std::to_string(errno);
		err += " unlink ";
		err += (char *)name;
		err += " failed";
		LOG_ERROR(nullptr) << err;
	}
#endif
	return 0;
}

int core::exec_new_binary(tag_cycle *cycle, char *const *argv)
{
	tag_core_exec	ctx;
	memset(&ctx, 0x0, sizeof(tag_core_exec));

	ctx.path = argv[0];
	ctx.name = const_cast<char *>("new binary process");
	ctx.argv = argv;

	char			**env, *var = nullptr;
	unsigned int	  n;

	n = 2;
	env = set_environment(cycle, &n);
	if (env == nullptr) {
		return -1;
	}

	if (cycle->listening.size() > 0)
	{
		std::string sz_var = HYSTACK_VAR "=";
		for (auto ls : cycle->listening) {
			sz_var += to_string(ls->sock);
			sz_var += ";";
		}

		int len = sz_var.size() + 2;
		var = (char *)alloc_system_ptr(len);
		if (var == nullptr) {
			alloc_system_free(env);
			return -1;
		}

		strcat(var, sz_var.data());
		env[n++] = var;
	}

	env[n++] = (char *)
		"SPARE=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
		"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

	env[n] = nullptr;

	ctx.envp = (char *const *)env;

	tag_core_conf	*ccf;
	ccf = (tag_core_conf *) modules_get_conf(cycle->conf_ctx, core_module);

	if (rename(ccf->pid.data(), ccf->old_pid.data()) == -1) {

		std::string err = std::to_string(errno);
		err += "rename_file_n";
		err += ccf->pid;
		err += "back to";
		err += ccf->old_pid;
		err += "failed after an attempt to execute new binary process";
		err += argv[0];

		return -1;
	}

	int pid = process_mgr::execute(cycle, &ctx);

	if (pid == -1) 
	{
		if (rename(ccf->old_pid.data(), ccf->pid.data()) == -1) {
			
			std::string err = std::to_string(errno);
			err += " rename_file_n ";
			err += ccf->pid;
			err += "back to";
			err += ccf->old_pid;
			err += "failed after an attempt to execute new binary process";
			err += argv[0];

			LOG_ERROR(nullptr) << err;
			return -1;
		}
	}

	return pid;
}

static void core_cleanup_environment(void *data)
{
	char  **env = (char **)data;

	if (environ == env) {
		return;
	}

	::free(env);
}


char ** core::set_environment(tag_cycle * cycle, unsigned int *last)
{
	tag_core_conf	*ccf;
	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	if (last == nullptr && ccf->environment) {
		return ccf->environment;
	}

	for (auto s : ccf->env)
	{
		if (strcmp(s.data(), "TZ") == 0
			|| strncmp(s.data(), "TZ=", 3) == 0) {
			goto tz_found;
		}
	}

	ccf->env.push_back("TZ");

tz_found:

	char **p, **env;
	unsigned int n;

	n = 0;

	for (auto s : ccf->env)
	{
		char * var = (char *)s.data();
		int len = (int)s.size();

		if (var[len - 1] == '=') {
			n++;
			continue;
		}

		for (p = process_environ; *p; p++) {

			if (strncmp((const char *)*p, var, len) == 0
				&& (*p)[len] == '=')
			{
				n++;
				break;
			}
		}

	}

	if (last) {

		env = (char **)alloc_pool_ptr(cycle->pool, (*last + n + 1) * sizeof(char *));
		if (env == nullptr) {
			return nullptr;
		}

		*last = n;
	}
	else {

		env = (char **)alloc_pool_ptr(cycle->pool, (n + 1) * sizeof(char *));
		if (env == nullptr) {
			return nullptr;
		}

		[](tag_cycle * cycle, char ** env_){

			tag_pool			*pool;
			tag_pool_cleanup	*cln;

			pool = cycle->pool;

			cln = alloc_pool_add_cleanup(pool, 0);
			if (cln == nullptr) {
				return -1;
			}

			cln->handler = core_cleanup_environment;
			cln->data = (void *)env_;
			return 0;

		}(cycle, env);
	}

	n = 0;

	for (auto s : ccf->env) {

		char * var = (char *)s.data();
		int len = (int)s.size();

		if (var[len] == '=') {
			env[n++] = (char *)var;
			continue;
		}

		for (p = process_environ; *p; p++) {

			if (strncmp((const char *)*p, var, len) == 0
				&& (*p)[len] == '=')
			{
				env[n++] = *p;
				break;
			}
		}
	}

	env[n] = nullptr;
	if (last == nullptr) {
		ccf->environment = env;
		environ = env;
	}

	return env;
}
