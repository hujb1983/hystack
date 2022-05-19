/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <core_config.h>
#include <fstream>
#include "nodes.h"
#include "nodes_stream.h"


 /* files_stream::nodes_stream */
nodes_stream::nodes_stream(const std::string &prefix) : files_stream(prefix) 
{

}

/* files_stream::~nodes_stream */
nodes_stream::~nodes_stream() 
{

}

/* files_stream::init */
int nodes_stream::init(const std::string &file_path, const std::string &file_name) 
{
	if (files_stream::init(file_path, file_name) == -1) {
		return -1;
	}

	file_scene = m_file_prefix + file_path + file_name + ".serial";

	this->m_wpos_last = 0;
	this->m_rpos_last = 0;
	return 0;
}

/* files_stream::serial */
int nodes_stream::serial(unsigned char save)
{
	if (save == 1)
	{
		std::ofstream ofs(file_scene, std::ios::out | std::ios::trunc);
		if (ofs.is_open() == false) {
			return RET_ERROR;
		}

		ofs.seekp(std::ios::beg);
		
		std::string param = "";
		param += "#stream param\r\n";
		param += "file_path: " + file_scene + "\r\n";
		param += "read_pos: " + std::to_string(this->m_rpos_last) + "\r\n";
		param += "\r\n";

		ofs.write((char *)param.data(), param.size());
		ofs.close();
	}
	else
	{
		std::ifstream ifs(file_scene, std::ios::binary | std::ios::in);
		if (ifs.is_open() == false) {
			return RET_ERROR;
		}

		ifs.seekg(0, std::ios::beg);
		this->m_wpos_last = ifs.tellg();

		unsigned char read_stream[1024] = { 0 };
		ifs.read((char *)&read_stream, sizeof(read_stream));
		ifs.close();

		char   number[32] = { 0 };
		char * ptr = nullptr;
		char * last = nullptr;

		ptr = (char *)read_stream;
		last = strstr(ptr, "read_pos:");
		if (last != nullptr) 
		{
			ptr = last + strlen("read_pos:");
			last = strstr(ptr, "\r\n");
			if (last != nullptr) {
				memcpy(number, ptr, last - ptr);
				this->m_rpos_last = atoi(ptr);
			}
		}

		std::string data_file = m_file_prefix + m_file_path + m_file_name;
		std::ifstream ifd(data_file, std::ios::binary | std::ios::in);
		if (ifd.is_open() == false) {
			return RET_ERROR;
		}

		ifd.seekg(0, std::ios::end);
		this->m_wpos_last = ifd.tellg();
		ifd.close();

		/* wpos | rpos */
		if (this->m_wpos_last < this->m_rpos_last)
		{
			LOG_ERROR(nullptr) << "wpos < rpos.";
			return RET_ERROR;
		}
	}

	return RET_OK;
}

/* nodes stream redirect */
int nodes_stream::redirect()
{
	std::string file = m_file_prefix + m_file_path + m_file_name;
	std::string bak = file + ".bak";
	rename(file.data(), bak.data());

	this->m_wpos_last = 0;
	this->serial(1);
	return RET_OK;
}
