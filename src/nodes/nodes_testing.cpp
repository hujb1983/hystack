/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, Feb, 2021
 */

#include <core.h>
#include <core_config.h>
#include <method/circuit_queue.hpp>

#include "nodes.h"
#include "nodes_testing.h"

 /* nodes_buffer */
class nodes_buffer
{
public:
	nodes_buffer();
	virtual ~nodes_buffer();

public:
	void create(int length, unsigned char extra_len);
	void completion(int bytes_recvd);
	void clear();

public:
	void get_recv_param(unsigned char **recv_ptr, int &length);
	void remove_first_packet(unsigned short length);

public:
	unsigned char* get_first_packet_ptr(int length = -1);
	int get_length();
	int get_space();

private:
	circuit_queue<unsigned char> * m_queue;
};

/* nodes_buffer::nodes_buffer */
nodes_buffer::nodes_buffer() {
	m_queue = nullptr;
}

/* nodes_buffer::~nodes_buffer */
nodes_buffer::~nodes_buffer() {
	if (m_queue) {
		delete m_queue;
	}
}

/* nodes_buffer::create */
void nodes_buffer::create(int len, unsigned char extra_len)
{
	if (m_queue != nullptr) {
		delete m_queue;
	}

	m_queue = new circuit_queue<unsigned char>;
	m_queue->create(len, extra_len);
}

/* nodes_buffer::completion */
void nodes_buffer::completion(int bytes_recvd) {
	m_queue->enqueue(nullptr, bytes_recvd);
}

/* nodes_buffer::clear */
void nodes_buffer::clear() {
	m_queue->clear();
}

/* nodes_buffer::get_recv_param */
void nodes_buffer::get_recv_param(unsigned char **recv_ptr, int &len)
{
	*recv_ptr = m_queue->get_write_ptr();
	len = m_queue->get_writable_len();
}

/* nodes_buffer::get_first_packet_ptr */
unsigned char * nodes_buffer::get_first_packet_ptr(int length)
{
	if (m_queue->get_length() < length)
		return nullptr;

	if (m_queue->get_back_data_count() < length) {
		m_queue->copy_head_data_to_extra_buffer(length - m_queue->get_back_data_count());
	}

	return  m_queue->get_read_ptr();
}

/* nodes_buffer::get_length */
int nodes_buffer::get_length() {
	return m_queue->get_length();
}

/* nodes_buffer::get_space */
int nodes_buffer::get_space() {
	return m_queue->get_space();
}

/* nodes_buffer::remove_first_packet */
void nodes_buffer::remove_first_packet(unsigned short len) {
	m_queue->dequeue(nullptr, len);
}

/* nodes_testing */
class nodes_testing
{
public:
	nodes_testing();
	virtual ~nodes_testing();

public:
	void packet_nginx(tag_nodes_task * task);
	void packet_cache(tag_nodes_task * task);
	void packet_redis(tag_nodes_task * task);
	void packet_ssl(tag_nodes_task * task);
	void packet_lua(tag_nodes_task * task);
	void packet_step(tag_nodes_task * task);
	void packet_errpage(tag_nodes_task * task);
	void packet_http(tag_nodes_task * task);
	void packet_defense(tag_nodes_task * task);
};

/* nodes_testing::nodes_testing */
nodes_testing::nodes_testing() {

}

/* nodes_testing::~nodes_testing */
nodes_testing::~nodes_testing() {

}

/* nodes_testing::packet_nginx */
void nodes_testing::packet_nginx(tag_nodes_task * task)
{

}

/* nodes_testing::packet_cache */
void nodes_testing::packet_cache(tag_nodes_task * task)
{

}

/* nodes_testing::packet_redis */
void nodes_testing::packet_redis(tag_nodes_task * task)
{

}

/* nodes_testing::packet_ssl */
void nodes_testing::packet_ssl(tag_nodes_task * task)
{

}

/* nodes_testing::packet_lua */
void nodes_testing::packet_lua(tag_nodes_task * task)
{

}

/* nodes_testing::packet_step */
void nodes_testing::packet_step(tag_nodes_task * task)
{

}

/* nodes_testing::packet_errpage */
void nodes_testing::packet_errpage(tag_nodes_task * task)
{

}

/* nodes_testing::packet_http */
void nodes_testing::packet_http(tag_nodes_task * task)
{

}

/* nodes_testing::packet_defense */
void nodes_testing::packet_defense(tag_nodes_task * task)
{

}


/* nodes_testing_nginx_process */
int nodes_testing_nginx_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_cache_process */
int nodes_testing_cache_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_cache_process */
int nodes_testing_redis_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_ssl_process */
int nodes_testing_ssl_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_lua_process */
int nodes_testing_lua_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_step_process */
int nodes_testing_step_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_errpage_process */
int nodes_testing_errpage_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_http_process */
int nodes_testing_http_process(tag_nodes_task * task) {
	return 0;
}

/* nodes_testing_defense_process */
int nodes_testing_defense_process(tag_nodes_task * task) {
	return 0;
}
