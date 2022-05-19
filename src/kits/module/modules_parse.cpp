/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <string>
#include <fstream>
using namespace std;

#include "modules.h"
#include "modules_conf.h"
#include "modules_config.h"
#include "modules_cycle.h"
#include <logs/logs.h>

/* The eight fixed arguments */
static unsigned int argument_number[] = {
	CONF_NOARGS,
	CONF_TAKE1,
	CONF_TAKE2,
	CONF_TAKE3,
	CONF_TAKE4,
	CONF_TAKE5,
	CONF_TAKE6,
	CONF_TAKE7
};

char* modules_parse::include_file(tag_conf * conf)
{
	int				rc;
	int				type;
	unsigned int	line_count;
	std::string		line, sentence;

	if (!conf->conf_ptr.is_open())
	{
		conf->conf_ptr.open(conf->conf_file.data());
		if (!conf->conf_ptr.is_open()) {
			return nullptr;
		}

		type = emParseFile;
	}
	else if (conf->conf_ptr.is_open() == true) {
		type = emParseBlock;
	}
	else {
		type = emParseParam;
	}

	line_count = 0;

	for (;;)
	{
		line_count++;

		if (!getline(conf->conf_ptr, line)) {
			goto done;
		}

		if (line.size() <= 0) {
			continue;
		}

		conf->line = line;
		rc = read_token(conf);

		if (rc == -1) {
			goto done;
		}

		if (rc == CONF_BLOCK_DONE) {
			if (type != emParseBlock) {
				goto failed;
			}
			goto done;
		}

		if (rc == CONF_FILE_DONE) {
			if (type == emParseBlock) {
				LOG_EMERG(nullptr) << ("unexpected end of file, expecting \"}\"");
				goto failed;
			}
			goto done;
		}

		if (rc == CONF_BLOCK_START) {
			if (type == emParseParam) {
				LOG_EMERG(nullptr) << ("block directives are not supported in -g option");
				goto failed;
			}
		}

		if (conf->args.size() == 0) {
			continue;
		}

		rc = handler(conf, rc);
		if (rc == RET_ERROR) {
			goto failed;
		}

		conf->args.clear();
	}

failed:

	LOG_STDERR << "(emerg) " << "in " << line_count << " line," << "error: " << line;
	rc = RET_ERROR;

	return nullptr;

done:
	return const_cast<char *>(line.data());
}

char* modules_parse::include_param(tag_conf * cf) {
	return nullptr;
}

int	modules_parse::read_token(tag_conf * conf)
{
	int				rc;
	size_t			pos;
	std::string     word, line, sentence;

	line = conf->line;

	pos = line.find_first_of('#', 0);
	if (pos != string::npos) {

		if (pos == 0) {
			return 0;
		}
	}

	sentence = line.substr(0, pos - 1);
	line.clear();

	for (auto ch : sentence) {

		switch (ch) {
		case '\n':
		case '\r':
		case '\0':
		case '\t':
			line += ' ';
			continue;

		case ';':
			rc = 0;
			goto sentence_parse;

		case '{':
			rc = CONF_BLOCK_START;
			goto sentence_parse;

		case '}':
			rc = CONF_BLOCK_DONE;
			goto sentence_parse;

		default:
			line += ch;
		}
	}

	word.clear();
	line.clear();
	conf->args.clear();

	return 0;

sentence_parse:

	sentence = line;

	word.clear();
	line.clear();
	conf->args.clear();

	for (auto ch : sentence) {

		if (' ' == ch || '\"' == ch) {
			if (word.size() > 0) {
				conf->args.push_back(word);
				word.clear();
			}
		}
		else {
			word += ch;
		}
	}

	// last work
	if (word.size() > 0) {
		conf->args.push_back(word);
	}

	return rc;
}

int	modules_parse::handler(tag_conf *cf, int last)
{
	char            *rv;
	tag_string		 name;
	tag_command		*cmd;
	std::string		 str_name;
	unsigned int	 found;
	void            *conf, **ptrConf;

	str_name = cf->args[0];
	name.len = str_name.size();
	name.data = (unsigned char *)str_name.data();

	found = 0;

	for (auto m : cf->cycle->modules) {

		cmd = m->commands;
		if (cmd == nullptr) {
			continue;
		}

		for ( /* void */; cmd->name.len; cmd++) {

			if (name.len != cmd->name.len) {
				continue;
			}

			if (strcmp((char *)name.data, (char *)cmd->name.data) != 0) {
				continue;
			}

			found = 1;

			if (m->type != MODULE_CONF &&
				m->type != cf->module_type) {
				continue;
			}

			if (!(cmd->type & cf->cmd_type)) {
				continue;
			}

			if (!(cmd->type & CONF_BLOCK) && last != 0) {
				return -1;
			}

			if ((cmd->type & CONF_BLOCK) && last != CONF_BLOCK_START) {
				return -1;
			}

			if (!(cmd->type & CONF_ANY)) {
				if (cmd->type & CONF_FLAG) {
					if (cf->args.size() != 2) {
						goto invalid;
					}
				}
				else if (cmd->type & CONF_1MORE) {
					if (cf->args.size() < 2) {
						goto invalid;
					}
				}
				else if (cmd->type & CONF_2MORE) {
					if (cf->args.size() < 3) {
						goto invalid;
					}
				}
				else if (cf->args.size() > CONF_MAX_ARGS) {
					goto invalid;
				}
				else if (!(cmd->type & argument_number[cf->args.size() - 1])) {
					goto invalid;
				}
			}

			conf = nullptr;

			if (cmd->type & CMD_DIRECT_CONF) {
				conf = ((void **)cf->context)[m->index];
			}
			else if (cmd->type & CMD_MAIN_CONF) {
				conf = &(((void **)cf->context)[m->index]);
			}
			else if (cf->context) {
				ptrConf = *(void ***)((char *)cf->context + cmd->conf);
				if (ptrConf) {
					conf = (ptrConf)[m->ctx_index];
				}
			}

			rv = cmd->ptr_set(cf, cmd, conf);
			if (nullptr == rv) {
				return -1;
			}

			return 0;
		}
	}

	if (found == 0) {
		return -1;
	}

	return 0;

invalid:
	return -1;
}

char* modules_parse::flag(tag_conf *cf, tag_command *cmd, void *conf)
{
	if (cf->args.size() != 2) {
		return nullptr;
	}

	int  *p = (int *)((char *)conf + cmd->offset);

	*p = 0;
	if (strcmp(cf->args[1].data(), "on") == 0) {
		*p = 1;
	}

	if (cmd->post) {
		tag_conf_post * post = (tag_conf_post *)cmd->post;
		return post->post_handler(cf, post, (void*)p);
	}

	return (char *)p;
}

char* modules_parse::msec(tag_conf *cf, tag_command *cmd, void *conf)
{
	return nullptr;
}

char* modules_parse::string(tag_conf *cf, tag_command *cmd, void *conf)
{
	std::string  *p = (std::string *)((char *)conf + cmd->offset);

	if (cf->args.size() != 2) {
		return nullptr;
	}

	p->append(cf->args[1]);
	if (cmd->post) {
		tag_conf_post * post = (tag_conf_post *)cmd->post;
		return post->post_handler(cf, post, const_cast<char *>(p->data()));
	}

	return const_cast<char *> (p->data());
}

char* modules_parse::number(tag_conf *cf, tag_command *cmd, void *conf)
{
	char *p = (char *)conf;

	int *num;
	num = (int *)(p + cmd->offset);

	if (cf->args.size() != 2) {
		return nullptr;
	}

	if (cmd->post) {
		tag_conf_post * post = (tag_conf_post *)cmd->post;
		return post->post_handler(cf, post, num);
	}

	return const_cast<char *> (cf->args[1].data());
}
