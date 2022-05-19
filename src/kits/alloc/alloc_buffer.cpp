/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#include "alloc_buffer.h"
#include "alloc_pool.h"

 /* alloc_buffer_destory */
int alloc_buffer_destory(tag_buffer *buff)
{

	return 0;
}

/* alloc_buffer_destory */
tag_buffer* alloc_buffer_create(tag_pool *pool, unsigned int size)
{
	tag_buffer * buff = nullptr;

	buff = (tag_buffer *)alloc_pool_ptr_null(pool, sizeof(tag_buffer));
	if (buff == nullptr) {
		return nullptr;
	}

	buff->ptr_start = (unsigned char *)alloc_pool_ptr_null(pool, size);
	if (buff->ptr_start == nullptr) {
		return nullptr;
	}

	buff->ptr_pos = buff->ptr_start;
	buff->ptr_last = buff->ptr_start;
	buff->ptr_end = buff->ptr_last + size;
	buff->temporary = 1;

	return buff;
}

/* alloc_buffer_chain_link */
tag_chain* alloc_buffer_chain_link(tag_pool *pool)
{
	tag_chain  *cl;

	cl = pool->chain;

	if (cl) {
		pool->chain = cl->next;
		return cl;
	}

	cl = (tag_chain *)alloc_pool_ptr_null(pool, sizeof(tag_chain));
	if (cl == nullptr) {
		return nullptr;
	}

	return cl;
}


/* alloc_chain_get_free_buf */
tag_chain *alloc_chain_get_free_buf(tag_pool *p, tag_chain **free)
{
	tag_chain  *cl;

	if (*free) {
		cl = *free;
		*free = cl->next;
		cl->next = nullptr;
		return cl;
	}

	cl = alloc_buffer_chain_link(p);
	if (cl == nullptr) {
		return nullptr;
	}

	cl->buff = (tag_buffer *) alloc_buffer_nullptr(p);
	if (cl->buff == nullptr) {
		return nullptr;
	}

	cl->next = nullptr;

	return cl;
}