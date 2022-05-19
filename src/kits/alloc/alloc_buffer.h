/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once

#include <core_config.h>
#include "alloc_pool.h"

/*  tag_buffer || tag_chain */
typedef struct ST_BUFFER		tag_buffer;
typedef struct ST_CHAIN			tag_chain;


/*  ST_BUFFER */
struct ST_BUFFER {

	unsigned char			*ptr_pos;
	unsigned char			*ptr_last;

	unsigned int			file_pos;
	unsigned int			file_last;

	unsigned char			*ptr_start;
	unsigned char			*ptr_end;

	void					*tag;
	tag_buffer				*ptr_shadow;

	unsigned int			 temporary : 1;
	unsigned int			 memory : 1;
	unsigned int			 mmap : 1;

	unsigned int			 recycled : 1;
	unsigned int			 in_file : 1;
	unsigned int			 flush : 1;
	unsigned int			 sync : 1;
	unsigned int			 last_buff : 1;
	unsigned int			 last_inchain : 1;
	unsigned int			 last_shadow : 1;
	unsigned int			 temp_file : 1;
};


/*  ST_CHAIN */
struct ST_CHAIN {
	tag_buffer			*buff;
	tag_chain			*next;
};


#define alloc_buffer(pool)			alloc_pool_ptr(pool, sizeof(tag_buffer))
#define alloc_buffer_nullptr(pool)	alloc_pool_ptr_null(pool, sizeof(tag_buffer))

/* alloc_buffer_destory */
int alloc_buffer_destory(tag_buffer *buff);

/* alloc_buffer_create */
tag_buffer* alloc_buffer_create(tag_pool *pool, unsigned int size);

/* alloc_buffer_chain_link */
tag_chain* alloc_buffer_chain_link(tag_pool *pool);

/* alloc_chain_get_free_buf */
tag_chain* alloc_chain_get_free_buf(tag_pool *p, tag_chain **free);