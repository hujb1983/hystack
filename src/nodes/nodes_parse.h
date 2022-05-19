/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once
#include <core_config.h>
#include <string.h>

#define NODES_PACKET_STRING        0x01000000
#define NODES_PACKET_NUMBER        0x02000000

namespace nodes_parse
{
	/* nodes::tag_parse */
	typedef struct 
	{
		tag_string 			name;
		unsigned long		type;
		char			 *(*set)(cJSON * json, std::string & key, void *conf);
		u_int				offset;
		void			   *post;
	} tag_parse;

	/* nodes::json_buffer_parse */
	char * json_buffer_parse(
		const std::string	&ptr_buff,
		std::string			 key_str,
		void				*ptr_packet);

	/* nodes::tag_error_code */
	typedef struct {
		int					 error;
		std::string			 result;
	} tag_error_code;

	/* nodes::tag_header */
	typedef struct {
		int					function_id;
		std::string			request_id;
	} tag_header;

	/* nodes::tag_packet_data */
	struct tag_packet_data {

		tag_packet_data() {
			index = 0;
			size = 0;
		}

		int					index;
		int					size;

		std::string			access_token;
		std::string			error_result;

		std::string			file_name;
		std::string			file_type;
		std::string			file_path;

		std::string			operate;

		std::string			option;
		std::string			urls;

		std::string			extra;
		std::string			content;
	};


	/* nodes::unpacket */
	class unpacket
	{
	public:
		char * error_data(const std::string & buff, tag_error_code * errorData) {
			json_buffer_parse(buff, "code", errorData);
			return (char *)errorData;
		}

		char * header_data(const std::string & buff, tag_header * headerData) {
			json_buffer_parse(buff, "functionId", headerData);
			json_buffer_parse(buff, "requestId", headerData);
			return (char *)headerData;
		}

		char * auther_data(const std::string & buff, tag_packet_data * authData) {
			json_buffer_parse(buff, "accessToken", authData);
			return (char *)authData;
		}

		char * file_data(const std::string & buff, tag_packet_data * configData) {
			json_buffer_parse(buff, "fileName", configData);
			json_buffer_parse(buff, "fileType", configData);
			json_buffer_parse(buff, "content", configData);
			json_buffer_parse(buff, "extra", configData);
			json_buffer_parse(buff, "path", configData);
			json_buffer_parse(buff, "opt", configData); 
			return (char *)configData;
		}

		char * cache_data(const std::string & buff, tag_packet_data * cache) {
			json_buffer_parse(buff, "fileName", cache);
			json_buffer_parse(buff, "fileType", cache);
			json_buffer_parse(buff, "filePath", cache);
			json_buffer_parse(buff, "opt", cache);
			return (char *)cache;
		}

		char * redis_data(const std::string & buff, tag_packet_data * redis) {
			json_buffer_parse(buff, "opt", redis);
			json_buffer_parse(buff, "url", redis);
			json_buffer_parse(buff, "size", redis);
			return (char *)redis;
		}

		char * lua_data(const std::string & buff, tag_packet_data * lua) {
			json_buffer_parse(buff, "fileName", lua);
			json_buffer_parse(buff, "content", lua);
			json_buffer_parse(buff, "opt", lua);
			return (char *)lua;
		}

		char * defense_data(const std::string & buff, tag_packet_data * def) {
			json_buffer_parse(buff, "opt", def);
			json_buffer_parse(buff, "content", def);
			return (char *)def;
		}

		char * step_data(const std::string & buff, tag_packet_data * lua) {
			json_buffer_parse(buff, "content", lua);
			json_buffer_parse(buff, "opt", lua);
			return (char *)lua;
		}
	};

	char * parse_integer(cJSON * json, std::string & key, void * param);
	char * parse_string(cJSON * json, std::string & key, void * param);
	char * parse_json_string(cJSON * json, std::string & key, void * param);

	// java -> c++
	int repair_json_string(const std::string & json, std::string & data);
};
