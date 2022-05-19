/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */
#pragma once

#include <string>
#include <exception>


class unix_exception : public std::exception
{
private:
	std::string _what;
	signed int  _code;
	std::string _path;

public:
	unix_exception() = delete;
	virtual ~unix_exception() {}

public:
	unix_exception(const std::string & _w, int _c);
	unix_exception(const std::string & _w, int _c, const std::string & _p);

	// return code;
	int ret_i(unsigned int _l = 0);
	int track(unsigned int _type = 0);
	int debug(unsigned int _type = 0);
};


#define UNIX_RET_S(c)		unix_exception("SUCCEED", c, __FUNCTION__).ret_i()
#define UNIX_RET_F(w, c)	unix_exception(w, c, __FUNCTION__).ret_i()
#define UNIX_TRACK(w, c)	unix_exception(w, c, __FUNCTION__).track()
#define UNIX_DEBUG(w, c)	unix_exception(w, c, __FUNCTION__).debug()
