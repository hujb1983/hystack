/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#include "unix_exception.h"

unix_exception::unix_exception(const std::string & _w, int _c)
{
	_what = _w;
	_code = _c;

}

unix_exception::unix_exception(const std::string & _w,
	int _c, const std::string & _p)
{
	_what = _w;
	_code = _c;
	_path = _p;
}


int unix_exception::ret_i(unsigned int _l) {
	return _code;
}

int unix_exception::track(unsigned int _type) {
	return _code;
}

int unix_exception::debug(unsigned int _type) {
	return _code;
}