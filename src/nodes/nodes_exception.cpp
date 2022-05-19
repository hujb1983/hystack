/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#include <thread>
#include <chrono>
#include <functional>
#include <string>

#include <logs/logs.h>
#include <core_config.h>

#include "nodes_stream.h"
#include "nodes_exception.h"

std::shared_ptr<files_stream>	nodes_exception_debug;
std::shared_ptr<files_stream>	nodes_exception_error;
std::shared_ptr<files_stream>	nodes_exception_track;

/* nodes_exception_init */
int nodes_exception_init(const std::string &prefix)
{
	nodes_exception_debug = std::make_shared<files_stream>(prefix);
	nodes_exception_debug->init("/net/", u8R"(nodes_stream_debug)");

	nodes_exception_error = std::make_shared<files_stream>(prefix);
	nodes_exception_error->init("/net/", u8R"(nodes_stream_error)");

	nodes_exception_track = std::make_shared<files_stream>(prefix);
	nodes_exception_track->init("/net/", u8R"(nodes_stream_track)");
	return 0;
}

/* nodes_exception::nodes_exception */
nodes_exception::nodes_exception(const std::string & _w, int _c)
{
	m_what = _w + "\n";
	m_code = _c;
}

/* nodes_exception::nodes_exception */
nodes_exception::nodes_exception(const std::string & _w, int _c, const std::string & _p)
{
	m_what = _w + "\n";
	m_code = _c;
	m_path = _p;
}

/* nodes_exception::ret_i */
int nodes_exception::ret_i(int type) {
	return nodes_exception_error->write((u_char *)m_what.data(), (u_int)m_what.size());
}

/* nodes_exception::track */
int nodes_exception::track(int type) {
	return nodes_exception_track->write((u_char *)m_what.data(), (u_int)m_what.size());
}

/* nodes_exception::debug */
int nodes_exception::debug(int type) {
	return nodes_exception_debug->write((u_char *) m_what.data(), (u_int)m_what.size());
}
