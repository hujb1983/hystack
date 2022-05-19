/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#pragma once


/* files_stream */
class files_stream
{
protected:
	std::string				m_file_prefix;
	std::string				m_file_redirect;
	std::string				m_file_path;
	std::string				m_file_name;

protected:
	unsigned long long		m_current_pos;
	unsigned long long		m_rpos_last;
	unsigned long long		m_wpos_last;

public:
	files_stream(const std::string &prefix);
	virtual ~files_stream();

public:
	virtual int init(const std::string &file_path, const std::string &file_name);
	virtual long long length();

public:
	virtual int redirect();
	virtual int serial(unsigned char save);
	virtual int stat(FILE * fd);

public:
	virtual int write(unsigned char * buff, unsigned int size);
	virtual int read(unsigned char * buff, unsigned int size);
};
