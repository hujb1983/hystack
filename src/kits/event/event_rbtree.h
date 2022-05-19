/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once

typedef unsigned long long		rbtree_key;
typedef int						rbtree_int;

#define TIMER_INFINITE			(unsigned long long) -1
#define TIMER_LAZY_DELAY		300

typedef struct ST_RBTREE		tag_rbtree;
typedef struct ST_RBTREE_NODE	tag_rbtree_node;

/* rbtree_insert_ptr */
typedef void(*rbtree_insert_ptr) (
	tag_rbtree_node *root,
	tag_rbtree_node *node,
	tag_rbtree_node *sentinel);

/* tag_rbtree */
struct ST_RBTREE {
	tag_rbtree_node		*root;
	tag_rbtree_node		*sentinel;
	rbtree_insert_ptr   insert;
};

/* tag_rbtree_node */
struct ST_RBTREE_NODE {
	rbtree_key			 key;
	tag_rbtree_node		*left;
	tag_rbtree_node		*right;
	tag_rbtree_node		*parent;
	unsigned char		 color;
	unsigned char		 data;
};

#define rbtree_red(node)				((node)->color = 1)
#define rbtree_black(node)				((node)->color = 0)
#define rbtree_is_red(node)				((node)->color)
#define rbtree_is_black(node)			(!rbtree_is_red(node))
#define rbtree_copy_color(n1, n2)		(n1->color = n2->color)

/* a sentinel must be black */
#define rbtree_sentinel_init(node)		rbtree_black(node)


/* rbtree_init */
#define rbtree_init(tree, s, i)				\
    rbtree_sentinel_init(s);				\
    (tree)->root = s;						\
    (tree)->sentinel = s;					\
    (tree)->insert = i

/* rbtree_min */
tag_rbtree_node* rbtree_min(tag_rbtree_node *node, tag_rbtree_node *sentinel);

/* rbtree_next */
tag_rbtree_node* rbtree_next(tag_rbtree *tree, tag_rbtree_node *node);

/* rbtree_insert */
void rbtree_insert(tag_rbtree *tree, tag_rbtree_node *node);

/* rbtree_insert_timer_value */
void rbtree_insert_timer_value(tag_rbtree_node *temp, tag_rbtree_node *node, tag_rbtree_node *sentinel);

/* rbtree_delete */
void rbtree_delete(tag_rbtree *tree, tag_rbtree_node *node);

/* rbtree_left_rotate */
void rbtree_left_rotate(tag_rbtree_node **root, tag_rbtree_node *sentinel, tag_rbtree_node *node);

/* rbtree_insert_value */
void rbtree_insert_value(tag_rbtree_node *temp, tag_rbtree_node *node, tag_rbtree_node *sentinel);

/* rbtree_right_rotate */
void rbtree_right_rotate(tag_rbtree_node **root, tag_rbtree_node *sentinel, tag_rbtree_node *node);
