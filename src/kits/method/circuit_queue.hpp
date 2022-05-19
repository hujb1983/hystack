/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */
#pragma once

#include <mutex>
#include <core_config.h>
#include "circuit_mutex.h"

/* circuit_queue */
template<typename T>
class circuit_queue
{
public:
	circuit_queue();
	virtual ~circuit_queue();

public:
	void create(int length, int extra_length = 0);
	inline void clear();

public:
	inline int get_space();
	inline int get_length();
	inline int get_back_data_count();

	inline int get_writable_len();
	inline int get_readable_len();

	inline int enqueue(T *ptr_src, int length);
	inline int dequeue(T *ptr_tar, int length);
	inline int peek(T *ptr_tar, int length);

	inline T* get_read_ptr();
	inline T* get_write_ptr();

	inline void copy_head_data_to_extra_buffer(int length);
	void dump_buffer(FILE * fp);

public:
	circuit_mutex	 m_mutex;
	T				*m_data;
	int				 m_length;
	int				 m_size;
	int				 m_head;
	int				 m_tail;
	int				 m_extra_size;
};


template<typename T>
circuit_queue<T>::circuit_queue()
	: m_data(nullptr), m_length(0), m_size(0), m_head(0), m_tail(0)
{

}

template<typename T>
circuit_queue<T>::~circuit_queue() {

}


template<typename T>
void circuit_queue<T>::create(int length, int extra_length)
{
	m_mutex.lock();
	if (m_data != nullptr) {
		delete[] m_data;
	}

	m_data = new T[length + extra_length];
	m_size = length;
	m_extra_size = extra_length;
	m_mutex.unlock();
}

template<typename T>
inline void circuit_queue<T>::clear()
{
	m_mutex.lock();
	m_length = 0;
	m_head = 0;
	m_tail = 0;
	m_mutex.unlock();
}


template<typename T>
inline int circuit_queue<T>::get_space()
{
	m_mutex.lock();
	int ret;
	ret = m_size - m_length;
	m_mutex.unlock();
	return ret;
}

template<typename T>
inline int circuit_queue<T>::get_length()
{
	m_mutex.lock();
	int ret = m_length;
	m_mutex.unlock();
	return ret;
}

template<typename T>
inline int circuit_queue<T>::get_back_data_count()
{
	m_mutex.lock();
	int ret;
	ret = m_size - m_head;
	m_mutex.unlock();
	return ret;
}

template<typename T>
inline T* circuit_queue<T>::get_read_ptr()
{
	m_mutex.lock();
	T *ptr_ret;
	ptr_ret = m_data + m_head;
	m_mutex.unlock();
	return ptr_ret;
}

template<typename T>
inline T* circuit_queue<T>::get_write_ptr()
{
	m_mutex.lock();
	T *ptr_ret;
	ptr_ret = m_data + m_tail;
	m_mutex.unlock();
	return ptr_ret;
}

template<typename T>
inline int circuit_queue<T>::get_readable_len()
{
	m_mutex.lock();
	int ret;
	if (m_head == m_tail) {
		ret = get_length() > 0 ? m_size - m_head : 0;
	}
	else if (m_head < m_tail) {
		ret = m_tail - m_head;
	}
	else {
		ret = m_size - m_head;
	}
	m_mutex.unlock();
	return ret;
}


template<typename T>
inline int circuit_queue<T>::get_writable_len()
{
	m_mutex.lock();
	int ret;
	if (m_head == m_tail) {
		ret = get_length() > 0 ? 0 : m_size - m_tail;
	}
	else if (m_head < m_tail) {
		ret = m_size - m_tail;
	}
	else {
		ret = m_head - m_tail;
	}
	m_mutex.unlock();
	return ret;
}


template<typename T>
inline int circuit_queue<T>::enqueue(T *ptr_src, int length)
{
	m_mutex.lock();
	if (get_space() < length) {
		m_mutex.unlock();
		return false;
	}

	if (ptr_src)
	{
		if (m_head <= m_tail)
		{
			int back_space_count = m_size - m_tail;
			if (back_space_count >= length) {
				memcpy(m_data + m_tail, ptr_src, sizeof(T) * length);
			}
			else {
				memcpy(m_data + m_tail, ptr_src, sizeof(T) * back_space_count);
				memcpy(m_data, ptr_src + back_space_count, sizeof(T) * (length - back_space_count));
			}
		}
		else {
			memcpy(m_data + m_tail, ptr_src, sizeof(T) * length);
		}
	}

	m_tail += length;
	m_tail %= m_size;
	m_length += length;
	m_mutex.unlock();
	return true;
}


template<typename T>
inline int circuit_queue<T>::dequeue(T *ptr_tar, int length)
{
	m_mutex.lock();
	if (!peek(ptr_tar, length) == 0) {
		m_mutex.unlock();
		return false;
	}

	m_head += length;
	m_head %= m_size;
	m_length -= length;
	m_mutex.unlock();
	return true;
}


template<typename T>
inline int circuit_queue<T>::peek(T *ptr_tar, int length)
{
	m_mutex.lock();
	if (m_length < length) {
		m_mutex.unlock();
		return false;
	}

	if (ptr_tar != nullptr)
	{
		if (m_head < m_tail)
		{
			memcpy(ptr_tar, m_data + m_head, sizeof(T) * length);
		}
		else
		{
			if (get_back_data_count() >= length)
			{
				memcpy(ptr_tar, m_data + m_head, sizeof(T) * length);
			}
			else
			{
				memcpy(ptr_tar, m_data + m_head, sizeof(T) * get_back_data_count());
				memcpy(ptr_tar + get_back_data_count(), m_data, sizeof(T) * (length - get_back_data_count()));
			}
		}
	}

	m_mutex.unlock();
	return true;
}

template<typename T>
inline void circuit_queue<T>::copy_head_data_to_extra_buffer(int length)
{
	m_mutex.lock();
	memcpy(m_data + m_size, m_data, length);
	m_mutex.unlock();
}

template<typename T>
void circuit_queue<T>::dump_buffer(FILE * fp)
{
	m_mutex.lock();
	T *ptr_ret;
	ptr_ret = m_data + m_head;
	if (m_head < m_tail)
	{
		fwrite(ptr_ret, (m_tail - m_head) * sizeof(T), 1, fp);
	}
	else
	{
		fwrite(ptr_ret, (m_size - m_head) * sizeof(T), 1, fp);
		fwrite(m_data, m_tail * sizeof(T), 1, fp);
	}
	m_mutex.unlock();
}
