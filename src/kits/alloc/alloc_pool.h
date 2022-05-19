/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once

 /* alloc_cleanup_ptr */
typedef void(*alloc_cleanup_ptr)	(void * _data);

typedef struct ST_POOL_LARGE		tag_pool_large;
typedef struct ST_POOL_CLEANUP		tag_pool_cleanup;
typedef struct ST_POOL				tag_pool;

typedef struct ST_BUFFER			tag_buffer;
typedef struct ST_CHAIN				tag_chain;

#define ALLOC_DEFAULT_POOL_SIZE		(16 * 1024)
#define ALLOC_POOL_ALIGNMENT		16

#define ALLOC_MIN_POOL_SIZE			alloc_align((sizeof(tagPool) + 2 * sizeof(tag_pool_large)),	\
									ALLOC_DEFAULT_POOL_SIZE) 

/* alloc_cleanup */
struct  ST_POOL_LARGE {
	tag_pool_large	*next;
	void			*alloc;
};

/* alloc_cleanup */
struct ST_POOL_CLEANUP
{
	alloc_cleanup_ptr	 handler;
	tag_pool_cleanup	*next;
	void				*data;

	ST_POOL_CLEANUP() {
		data = nullptr;
		next = nullptr;
	}
};


/* alloc_data */
typedef struct
{
	unsigned char		*last;
	unsigned char		*end;
	tag_pool			*next;

	unsigned int		 failed;
} tag_pool_data;


/* alloc_pool */
struct ST_POOL
{
	tag_pool_data		 d;
	unsigned int         max;
	tag_pool			*current;
	tag_chain			*chain;
	tag_pool_large		*large;
	tag_pool_cleanup	*cleanup;

	ST_POOL() {
		current = nullptr;
		large = nullptr;
		cleanup = nullptr;
	}
};


/* alloc_pool_create */
void * alloc_pool_ptr(tag_pool * pool, unsigned int size);

/* alloc_pool_create_null */
void * alloc_pool_ptr_null(tag_pool * pool, unsigned int size);

/* alloc_pool_create */
tag_pool * alloc_pool_create(unsigned int size);

/* alloc_pool_reset */
int alloc_pool_reset(tag_pool * pool);

/* alloc_pool_add_cleanup */
tag_pool_cleanup * alloc_pool_add_cleanup(tag_pool * pool, unsigned int size);

/* alloc_pool_destory */
int alloc_pool_destory(tag_pool * pool);

/* alloc_system_mem */
void * alloc_system_ptr(unsigned int size);

/* alloc_system_free */
void alloc_system_free(void * mem);