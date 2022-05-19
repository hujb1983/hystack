/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */

#include <memory>
#include <string>
#include <mutex>

#include "modules.h"
#include "modules_conf.h"
#include "modules_core.h"
#include "modules_config.h"
#include "modules_cycle.h"
#include "modules_event.h"

#include <alloc/alloc_pool.h>

#include <event/event.h>
#include <event/event_timer.h>
#include <event/event_connection.h>

tag_queue					posted_accept_events;
tag_queue					posted_next_events;
tag_queue					posted_events;

std::mutex					accept_mutex;
unsigned int				accept_use_mutex;
unsigned int				accept_disabled;
unsigned int				accept_mutex_held;
unsigned long long			accept_mutex_delay;

unsigned int				timer_resolution;

tag_event_actions			proc_event_actions;
unsigned int				proc_event_flags;

extern tag_module			core_module;
extern tag_module			epoll_module;
extern tag_module			select_module;

extern tag_module			event_module;
extern tag_module			event_core_module;

unsigned int				event_max_module;

#if (WINX)
unsigned int				event_timer_alarm;
#else
sig_atomic_t				event_timer_alarm;
#endif


static char * event_connections(tag_conf *cf, tag_command *cmd, void *conf);
static char * event_use(tag_conf *cf, tag_command *cmd, void *conf);


/* event_block */
static char * event_block(tag_conf *cf, tag_command *cmd, void * ptr)
{
	char				*rc;
	void				*rv;
	tag_core_module		*core_module;
	tag_conf			 pcf;
	void			  ***ctx;

	event_max_module = modules_conf::count(cf->cycle, EVENT_MODULE);

	ctx = (void***)alloc_pool_ptr_null(cf->pool, sizeof(void *));
	if (ctx == nullptr) {
		return nullptr;
	}

	*ctx = (void**)alloc_pool_ptr_null(cf->pool, event_max_module * sizeof(void *));
	if (*ctx == nullptr) {
		return nullptr;
	}

	*(void **)ptr = ctx;

	for (auto m : cf->cycle->modules)
	{
		if (m->type != EVENT_MODULE) {
			continue;
		}

		core_module = static_cast<tag_core_module *> (m->context);
		if (core_module->create_conf != nullptr)
		{
			rv = core_module->create_conf(cf->cycle);
			if (nullptr == rv) {
				return nullptr;
			}

			(*ctx)[m->ctx_index] = static_cast<void *> (rv);
		}
	}

	pcf.context = cf->context;
	pcf.module_type = cf->module_type;
	pcf.cmd_type = cf->cmd_type;

	cf->context = ctx;
	cf->module_type = EVENT_MODULE;
	cf->cmd_type = EVENT_CONF;

	rc = modules_parse::include_file(cf);
	if (rc == nullptr) {
		return nullptr;
	}

	cf->context = pcf.context;
	cf->module_type = pcf.module_type;
	cf->cmd_type = pcf.cmd_type;

	for (auto m : cf->cycle->modules)
	{
		if (m->type != EVENT_MODULE) {
			continue;
		}

		core_module = static_cast<tag_core_module *> (m->context);
		if (core_module->init_conf != nullptr)
		{
			rv = core_module->init_conf(cf->cycle, (*ctx)[m->ctx_index]);
			if (nullptr == rv) {
				return nullptr;
			}
		}
	}

	return (char *)ctx;
}

/* event_module_init_conf */
void * event_module_init_conf(tag_cycle *cycle, void *conf)
{
	tag_event_conf *ecf = static_cast<tag_event_conf*>(conf);
	return static_cast<void*>(ecf);
}

/* event_commands */
static tag_command event_commands[] = {

	{
		init_string("events"),
		CMD_MAIN_CONF | CONF_BLOCK | CONF_NOARGS,
		event_block,
		0,
		0,
		nullptr
	},

	init_command_null
};

/* event_module_ctx */
static tag_core_module event_module_ctx = {
	init_string("events"),
	nullptr,
	event_module_init_conf
};

/* event_module */
tag_module  event_module = {
	MODULE_V1,
	&event_module_ctx,						  /* module context */
	event_commands,						  /* module directives */
	MODULE_CORE,							  /* module type */
	nullptr,                                  /* init master */
	nullptr,                                  /* init module */
	nullptr,								  /* init process */
	nullptr,                                  /* init thread */
	nullptr,                                  /* exit thread */
	nullptr,                                  /* exit process */
	nullptr,                                  /* exit master */
	MODULE_V1_PADDING
};


/* event_core_commands */
static tag_command  event_core_commands[] = {

	{	init_string("worker_connections"),
		EVENT_CONF | CONF_TAKE1,
		event_connections,
		0,
		offsetof(tag_event_conf, connections),
		nullptr
	},

	{	init_string("use"),
		EVENT_CONF | CONF_TAKE1,
		event_use,
		0,
		offsetof(tag_event_conf, use),
		nullptr
	},

	{	init_string("multi_accept"),
		EVENT_CONF | CONF_TAKE1,
		modules_parse::flag,
		0,
		offsetof(tag_event_conf, accept_multi),
		nullptr
	},

	{	init_string("accept_mutex"),
		EVENT_CONF | CONF_TAKE1,
		modules_parse::flag,
		0,
		offsetof(tag_event_conf, accept_mutex),
		nullptr
	},

	{	init_string("accept_mutex_delay"),
		EVENT_CONF | CONF_TAKE1,
		modules_parse::msec,
		0,
		offsetof(tag_event_conf, accept_mutex_delay),
		nullptr
	},

	init_command_null
};


static void *  event_core_create_conf(tag_cycle *cycle)
{
	tag_event_conf	*event_conf;

	event_conf = (tag_event_conf *)alloc_pool_ptr(cycle->pool, sizeof(tag_event_conf));
	if (nullptr == event_conf) {
		return nullptr;
	}

	event_conf->connections = CMD_CONF_UNSET;
	event_conf->use = CMD_CONF_UNSET;
	event_conf->accept_multi = -1;
	event_conf->accept_mutex = -1;
	event_conf->accept_mutex_delay = CMD_CONF_UNSET_MSEC;
	event_conf->name = nullptr;
	return event_conf;
}

static char * event_core_init_conf(tag_cycle *cycle, void *conf)
{
	tag_module		    *module;
	tag_event_conf		*ecf;

	ecf = static_cast<tag_event_conf*> (conf);
	module = nullptr;

#if (WINX)
	// module = (tag_module *)&iocp_module;
#else
	int	fd = epoll_create(100);
	if (fd != -1) {
		(void)::close(fd);
		module = (tag_module *)&epoll_module;
	}
	else if (errno != ENOSYS) {
		module = (tag_module *)&select_module;
	}
#endif

	if (ecf->connections == CMD_CONF_UNSET) {
		ecf->connections = 512;
	}

	cycle->conn_size = ecf->connections;

	if (ecf->use == CMD_CONF_UNSET) {
		ecf->use = module->ctx_index;
	}

	if (ecf->accept_multi == -1) {
		ecf->accept_multi = 0;
	}

	if (ecf->accept_mutex == -1) {
		ecf->accept_mutex = 0;
	}

	if (ecf->accept_mutex_delay == CMD_CONF_UNSET) {
		ecf->accept_mutex_delay = 500;
	}

	return (char *)-1;
}

static tag_event_module  event_core_ctx =
{
	init_string("event_core"),
	event_core_create_conf,
	event_core_init_conf,

	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};


/* event_core_init_module */
static int event_core_init_module(tag_cycle *cycle)
{
	return 0;
}

/* event_core_init_process */
static int event_core_init_process(tag_cycle *cycle)
{
	void				***cf;
	tag_core_conf         *ccf;
	tag_event_conf        *ecf;
	tag_event_module	  *module;

	cf = modules_get_conf(cycle->conf_ctx, event_module);
	ecf = (tag_event_conf*)(*cf)[event_core_module.ctx_index];
	ccf = (tag_core_conf *)modules_get_conf(cycle->conf_ctx, core_module);

	if (ccf->master && ccf->worker_processes > 1 && ecf->accept_mutex) {
		accept_use_mutex = 1;
		accept_mutex_held = 0;
		accept_mutex_delay = ecf->accept_mutex_delay;
	}
	else {
		accept_use_mutex = 0;
	}

#if (WINX)
	accept_use_mutex = 0;
#endif

	queue_init(&posted_accept_events);
	queue_init(&posted_next_events);
	queue_init(&posted_events);

	if (event_timer::init() == -1) {
		return -1;
	}

	for (auto m : cycle->modules) {

		if (m->type != EVENT_MODULE) {
			continue;
		}

		if (m->ctx_index != ecf->use) {
			continue;
		}

		module = (tag_event_module *)m->context;

		if (module->actions.init(cycle, timer_resolution) != 0) {
			exit(2);
		}

		break;
	}

	int ret = [](tag_cycle * cycle) {

		unsigned         size;
		tag_connection	*c, *qc;
		tag_event		*rev, *wev;

		size = cycle->conn_size;
		qc = (tag_connection *)alloc_system_ptr(sizeof(tag_connection) * size);
		if (qc == nullptr) {
			return -1;
		}

		cycle->conn_ptr = qc;

		rev = (tag_event *)alloc_system_ptr(sizeof(tag_event) * size);
		if (rev == nullptr) {
			return -1;
		}
		cycle->read_event = rev;

		wev = (tag_event *)alloc_system_ptr(sizeof(tag_event) * size);
		if (wev == nullptr) {
			return -1;
		}
		cycle->write_event = wev;

		for (unsigned int i = 0; i < size; i++) {

			rev[i].closed = 1;
			rev[i].instance = 1;
			wev[i].closed = 1;

			qc[i].read = &rev[i];
			qc[i].write = &wev[i];

			cycle->conn_free_queue.push(&qc[i]);
		}

		rev = nullptr;
		wev = nullptr;

		for (auto ls : cycle->listening)
		{
			c = event_connection::get(ls->sock);

			if (c == nullptr) {
				return -1;
			}

			c->type = ls->type;
			c->listening = ls;
			ls->connection = c;

			rev = c->read;
			rev->accept = 1;

			rev->ptr_handler = (c->type == SOCK_STREAM) ? event_accept : event_accept;

			if (accept_use_mutex) {
				continue;
			}

			if (proc_add_event(rev, EVENT_READ, 0) == -1) {
				return -1;
			}
		}

		return 0;

	}(cycle);

	return ret;
}

/* event_core_module */
tag_module  event_core_module = {

	MODULE_V1,
	&event_core_ctx,						/* module context */
	event_core_commands,					/* module directives */
	EVENT_MODULE,							/* module type */
	nullptr,								/* init master */
	event_core_init_module,				/* init module */
	event_core_init_process,				/* init process */
	nullptr,								/* init thread */
	nullptr,								/* exit thread */
	nullptr,								/* exit process */
	nullptr,								/* exit master */
	MODULE_V1_PADDING
};


char* event_connections(tag_conf *cf, tag_command *cmd, void *conf)
{
	tag_event_conf  *ecf = static_cast<tag_event_conf *>(conf);

	string  value;
	if (ecf->connections != CMD_CONF_UNSET) {
		return const_cast<char *>("is duplicate");
	}

	value = cf->args[1];
	ecf->connections = atoi(value.data());
	if (ecf->connections == 0) {
		return nullptr;
	}

	cf->cycle->conn_size = ecf->connections;
	return const_cast<char *> (value.data());
}

char* event_use(tag_conf *cf, tag_command *cmd, void *conf)
{
	tag_event_module		*event_module;
	string					 value;
	tag_event_conf			*ecf = nullptr;

	ecf = (tag_event_conf *)conf;

	if (ecf->use != CMD_CONF_UNSET) {
		return const_cast<char *>("is duplicate");
	}

	value = cf->args[1];
	for (auto m : cf->cycle->modules) {

		if (m->type != EVENT_MODULE) {
			continue;
		}

		event_module = (tag_event_module *)m->context;
		if (event_module->name.len != 0) {
			if (strcmp((char*)event_module->name.data, value.data()) == 0) {
				ecf->use = m->ctx_index;
				return const_cast<char *>("is ok");
			}
		}
	}

	return (char *)ecf;
}
