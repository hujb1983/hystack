/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#include <conv/conv_json.h>
#include <core_config.h>
#include <string.h>

#include "nodes_parse.h"

namespace nodes_parse
{
	tag_parse  packet_parses[] = 
	{
		{ 
			init_string("code"),
			NODES_PACKET_NUMBER,
			parse_integer,
			offsetof(tag_error_code, error),
			nullptr
		},

		{ init_string("functionId"),
			NODES_PACKET_NUMBER,
			parse_integer,
			offsetof(tag_header, function_id),
			nullptr
		},

		{ init_string("requestId"),
			NODES_PACKET_NUMBER,
			parse_string,
			offsetof(tag_header, request_id),
			nullptr
		},

		{ init_string("accessToken"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, access_token),
			nullptr
		},

		{ init_string("fileName"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, file_name),
			nullptr
		},
		
		{ init_string("fileType"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, file_type),
			nullptr
		},

		{ init_string("filePath"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, file_path),
			nullptr
		},
		
		{ init_string("extra"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, extra),
			nullptr
		},

		{ init_string("content"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, content),
			nullptr
		},

		{ init_string("path"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, file_path),
			nullptr
		},

		{ init_string("opt"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, operate),
			nullptr
		},

		{ init_string("option"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, option),
			nullptr
		},

		{ init_string("url"),
			NODES_PACKET_STRING,
			parse_string,
			offsetof(tag_packet_data, urls),
			nullptr
		},

		{ init_string("size"),
			NODES_PACKET_NUMBER,
			parse_integer,
			offsetof(tag_packet_data, size),
			nullptr
		},

		{ init_string_null, 0, nullptr, 0, nullptr }
	};

	char * json_buffer_parse(const std::string & ptr_buff, std::string key_str, void * ptr_packet)
	{
		int			 ret;
		char		*name, *ptr_reture, sz_key[0xFF] = { 0 };
		void		*offset;
		cJSON		*json;

		json = cJSON_Parse(ptr_buff.data());
		if (json == nullptr) {
			return nullptr;
		}

		strcat(sz_key, key_str.data());

		ptr_reture = nullptr;

		for (auto parse : packet_parses) {

			if (parse.name.data == nullptr) {
				break;
			}

			name = (char *)(parse.name.data);

			ret = strcmp(sz_key, name);
			if (ret != 0) {
				continue;
			}

			if (nullptr != parse.set) {

				offset = ((char *)ptr_packet) + parse.offset;
				ptr_reture = parse.set(json, key_str, offset);

				if (nullptr != ptr_reture) {
					goto delete_result;
				}
				break;
			}
		}

	delete_result:
		cJSON_free(json);
		return ptr_reture;
	}

	char * parse_integer(cJSON * json, std::string & key, void * param)
	{
		cJSON * str = cJSON_GetObjectItem(json, key.data());
		if (str == nullptr) {
			return nullptr;
		}

		if (cJSON_IsString(str)) {
			return nullptr;
		}

		int * p = (int *)param;
		*p = str->valueint;

		return const_cast<char *>(std::to_string(*p).data());
	}

	char * parse_string(cJSON * json, std::string & key, void * param)
	{
		cJSON *str = cJSON_GetObjectItem(json, key.data());
		if (str == nullptr) {
			return nullptr;
		}

		if (cJSON_IsObject(str)) {
			return nullptr;
		}

		std::string * p = (std::string *)param;
		p->append(str->valuestring);

		return const_cast<char *>(p->data());
	}

	char * parse_json_string(cJSON * json, std::string & key, void * param)
	{
		cJSON	*str;

		str = cJSON_GetObjectItem(json, key.data());
		if (str == nullptr) {
			return nullptr;
		}

		if (!cJSON_IsNumber(str)) {
			return nullptr;
		}

		int * p = (int *)param;
		*p = str->valueint;

		return nullptr;
	}

	int repair_json_string(const std::string & json, std::string & data)
	{
		char * start = (char *)json.data();
		char * end = start + json.size();

		char * flag = strstr(start, "{");
		if (flag == nullptr) {
			return RET_ERROR;
		}

		start = flag;
		while (flag != end) {
			flag = strstr(flag, "}");
			if (flag == nullptr) {
				break;
			}

			end = flag;
			flag++;
		}

		end++;

		data.clear();
		data.append(start, end - start);
		return RET_OK;
	}
};
