/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alloc_pool.h"

 /* alloc_page_size */
unsigned int	alloc_page_size;
unsigned int	alloc_page_size_shift;
unsigned int	alloc_cache_line_size;

/* platform word */
#define alloc_align_ment		sizeof(unsigned long)

/* alloc_align_ptr */
#define alloc_align_ptr(p, a)       \
    (unsigned char *) (((unsigned long long) (p) + ((unsigned int) a - 1)) & ~((unsigned int) a - 1))


static void * alloc_pool_small(tag_pool * pool, unsigned int size, unsigned int align);
static void * alloc_pool_large(tag_pool * pool, unsigned int size);


/* alloc_pool_create */
void * alloc_pool_ptr(tag_pool * pool, unsigned int size)
{
	if ((size + sizeof(tag_pool)) <= pool->max) {
		return alloc_pool_small(pool, size, 1);
	}

	return alloc_pool_large(pool, size);
}

/* alloc_pool_create_null */
void * alloc_pool_ptr_null(tag_pool * pool, unsigned int size)
{
	void	*p = nullptr;

	p = alloc_pool_ptr(pool, size);
	if (p) {
		memset(p, 0x0, size);
	}

	return p;
}

/* alloc_pool_create_temp */
tag_pool * alloc_pool_create(unsigned int size)
{
	tag_pool  *p;

	p = (tag_pool *)malloc(size);
	if (nullptr == p) {
		return 0;
	}

	p->d.last = (unsigned char *)p + sizeof(tag_pool);
	p->d.end = (unsigned char *)p + size;
	p->d.next = nullptr;
	p->d.failed = 0;

	size = size - sizeof(tag_pool);
	p->max = (unsigned short)0x8FFF;

	p->current = p;
	p->large = nullptr;
	p->cleanup = nullptr;
	
	return p;
}

/* alloc_pool_destory */
int alloc_pool_destory(tag_pool * pool)
{
	tag_pool			*p, *n;
	tag_pool_large		*l;
	tag_pool_cleanup	*c;

	if (pool == nullptr) {
		return 0;
	}

	for (c = pool->cleanup; c; c = c->next) {
		if (c->handler) {
			c->handler(c->data);
		}
	}

	for (l = pool->large; l; l = l->next) {
		if (l->alloc != nullptr) {
			::free(l->alloc);
		}
	}

	for (p = pool, n = pool->d.next; ; p = n, n = n->d.next)
	{
		::free(p);

		if (n == nullptr) {
			break;
		}
	}

	return 0;
}

/* alloc_pool_reset */
int alloc_pool_reset(tag_pool * pool)
{
	tag_pool			*p;
	tag_pool_large		*l;

	for (l = pool->large; l; l = l->next) {
		if (l->alloc) {
			::free(l->alloc);
		}
	}

	for (p = pool; p; p = p->d.next) {
		p->d.last = (unsigned char *)p + sizeof(tag_pool);
		p->d.failed = 0;
	}

	pool->current = pool;
	pool->large = nullptr;

	return 0;
}

/* alloc_pool_add_cleanup */
tag_pool_cleanup *alloc_pool_add_cleanup(tag_pool * pool, unsigned int size)
{
	tag_pool_cleanup  *c;

	c = (tag_pool_cleanup *)alloc_pool_ptr_null(pool, sizeof(tag_pool_cleanup));
	if (c == nullptr) {
		return nullptr;
	}

	if (size > 0) {
		c->data = alloc_pool_ptr_null(pool, size);
		if (c->data == nullptr) {
			return nullptr;
		}
	}
	else {
		c->data = nullptr;
	}

	c->handler = nullptr;
	c->next = pool->cleanup;
	pool->cleanup = c;
	return c;
}


/* alloc_pool_palloc_block */
void* alloc_pool_block(tag_pool * pool, unsigned int size)
{
	unsigned char		*m;
	size_t				 ptr_size;
	tag_pool		*p, *new_pool;

	ptr_size = (size_t)(pool->d.end - (unsigned char *)pool);

	m = (unsigned char *)malloc(ptr_size);
	if (m == nullptr) {
		return nullptr;
	}

	new_pool = (tag_pool *)m;

	new_pool->d.end = m + ptr_size;
	new_pool->d.next = nullptr;
	new_pool->d.failed = 0;

	m += sizeof(tag_pool_data);
	m = alloc_align_ptr(m, alloc_align_ment);
	new_pool->d.last = m + size;

	for (p = pool->current; p->d.next; p = p->d.next) {

		if (p->d.failed++ > 4) {
			pool->current = p->d.next;
		}
	}

	new_pool->large = nullptr;
	new_pool->cleanup = nullptr;

	p->d.next = new_pool;
	return m;
}


/* alloc_pool_palloc_block */
void* alloc_pool_small(tag_pool * pool, unsigned int size, unsigned int align)
{
	unsigned char	*m;
	tag_pool  *p;

	p = pool->current;

	do {
		m = p->d.last;

		if (align) {
			m = alloc_align_ptr(m, alloc_align_ment);
		}

		if ((size_t)(p->d.end - m) >= size) {
			p->d.last = m + size;

			return m;
		}

		p = p->d.next;

	} while (p);

	return alloc_pool_block(pool, size);
}


/* alloc_pool_large */
void* alloc_pool_large(tag_pool * pool, unsigned int size)
{
	void				*p;
	unsigned int         n;
	tag_pool_large			*large;

	p = malloc(size);
	if (p == nullptr) {
		return nullptr;
	}

	n = 0;

	for (large = pool->large; large; large = large->next)
	{
		if (large->alloc == nullptr) {
			large->alloc = p;
			return p;
		}

		if (n++ > 3) {
			break;
		}
	}

	large = (tag_pool_large *)alloc_pool_small(pool, sizeof(tag_pool_large), 1);
	if (large == nullptr) {
		::free(p);
		return nullptr;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;
	return p;
}


/* alloc_system_mem */
void * alloc_system_ptr(unsigned int size) {
	return malloc(size);
}

/* alloc_system_free */
void alloc_system_free(void * ptr) {
	if (ptr == nullptr) {
		return;
	}
	return ::free(ptr);
}
