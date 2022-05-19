/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <fstream>
#include <string>
#include <core_config.h>
#include "files_stream.h"

/* files_stream::files_stream */
files_stream::files_stream(const std::string &prefix) : m_file_prefix(prefix) {

}

/* files_stream::~files_stream */
files_stream::~files_stream() {

}


/* files_stream::init */
int files_stream::init(const std::string &file_path, const std::string &file_name)
{
	m_file_path = file_path;
	m_file_name = file_name;

	m_rpos_last = 0L;
	m_rpos_last = 0L;

	m_current_pos = m_rpos_last;
	return 0;
}

/* files_stream::file_length */
long long files_stream::length()
{
	std::string file = "";
	m_file_prefix + m_file_path + m_file_name;

	FILE * fd = nullptr;
	fd = fopen(file.data(), "r");
	if (fd == nullptr) {
		return -1;
	}

	if (fseek(fd, 0, SEEK_END) != 0) {
		fclose(fd);
		return -1;
	}

	fclose(fd);
	return ftell(fd);
}

/* files_stream::write */
int files_stream::write(unsigned char * buff, unsigned int size)
{
	std::string file_ = m_file_prefix + m_file_path + m_file_name;

	std::ofstream ofs(file_, std::ios::app);
	if (ofs.is_open() == false) {
		return -1;
	}

	this->serial(0);

	ofs.write((char *)buff, size);
	ofs.close();

	m_wpos_last += size;
	this->serial(1);

	return size;
}

/* files_stream::read */
int files_stream::read(unsigned char * buff, unsigned int size)
{
	std::string file_ = m_file_prefix + m_file_path + m_file_name;

	std::ifstream ifs(file_, std::ios::binary | std::ios::in);
	if (ifs.is_open() == false) {
		return -1;
	}

	this->serial(0);
	int ret_size = 0;

	if (m_wpos_last > m_rpos_last)
	{
		ifs.seekg(m_rpos_last, std::ios::beg);
		ifs.read((char *)buff, size);
		ret_size = (int)ifs.gcount();
		ifs.close();
	}

	m_rpos_last += ret_size;
	this->serial(1);

	return ret_size;
}

/* files_stream::serial */
int files_stream::serial(unsigned char save) {
	return 0;
}

/* files_stream::stat */
int files_stream::stat(FILE * fd)
{
#if (!WINX)
	struct stat st;
	if ((fstat(fileno(fd), &st)) != 0) {
		return -1;
	}
	return st.st_size;
#else
	return 0;
#endif
}

/* files_stream::redirect */
int files_stream::redirect()
{

	return 0;
}
