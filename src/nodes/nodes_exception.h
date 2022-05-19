/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <exception>

 /* nodes_exception */
class nodes_exception : public std::exception
{
public:
	std::string m_what;
	signed int  m_code;
	std::string m_path;

public:
	nodes_exception() = delete;
	virtual ~nodes_exception() {}

	/* error code */
	signed int code() {
		return m_code;
	}

	const char* error() const {
		return m_what.data();
	}

public:
	nodes_exception(const std::string & _w, int _c);
	nodes_exception(const std::string & _w, int _c, const std::string & _p);

	// return code;
	int ret_i(int _l = 0);
	int track(int _type = 0);
	int debug(int _type = 0);
};

#define NODES_RET_S(c)		nodes_exception("SUCCEED", c, __FUNCTION__).ret_i()
#define NODES_RET_F(w, c)	nodes_exception(w, c, __FUNCTION__).ret_i()
#define NODES_TRACE(w, c)	nodes_exception(w, c, __FUNCTION__).track()
#define NODES_DEBUG(w, c)	nodes_exception(w, c, __FUNCTION__).debug()

