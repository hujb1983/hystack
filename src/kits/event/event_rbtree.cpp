/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "event_rbtree.h"

/* rbtree_min */
tag_rbtree_node* rbtree_min(tag_rbtree_node *node, tag_rbtree_node *sentinel)
{
	while (node->left != sentinel) {
		node = node->left;
	}
	return node;
}

/* rbtree_next */
tag_rbtree_node* rbtree_next(tag_rbtree *tree, tag_rbtree_node *node)
{
	tag_rbtree_node  *root, *sentinel, *parent;

	sentinel = tree->sentinel;
	if (node->right != sentinel) {
		return rbtree_min(node->right, sentinel);
	}

	root = tree->root;

	for (;; ) {

		parent = node->parent;

		if (node == root) {
			return nullptr;
		}

		if (node == parent->left) {
			return parent;
		}

		node = parent;
	}
}

/* rbtree_insert */
void rbtree_insert(tag_rbtree *tree, tag_rbtree_node *node)
{
	tag_rbtree_node  **root, *temp, *sentinel;

	/* a binary tree insert */

	root = &tree->root;
	sentinel = tree->sentinel;

	if (*root == sentinel) {
		node->parent = nullptr;
		node->left = sentinel;
		node->right = sentinel;
		rbtree_black(node);
		*root = node;

		return;
	}

	tree->insert(*root, node, sentinel);

	/* re-balance tree */
	while (node != *root && rbtree_is_red(node->parent))
	{

		if (node->parent == node->parent->parent->left) {
			temp = node->parent->parent->right;

			if (rbtree_is_red(temp)) {
				rbtree_black(node->parent);
				rbtree_black(temp);
				rbtree_red(node->parent->parent);
				node = node->parent->parent;

			}
			else {
				if (node == node->parent->right) {
					node = node->parent;
					rbtree_left_rotate(root, sentinel, node);
				}

				rbtree_black(node->parent);
				rbtree_red(node->parent->parent);
				rbtree_right_rotate(root, sentinel, node->parent->parent);
			}

		}
		else {
			temp = node->parent->parent->left;

			if (rbtree_is_red(temp)) {
				rbtree_black(node->parent);
				rbtree_black(temp);
				rbtree_red(node->parent->parent);
				node = node->parent->parent;

			}
			else {
				if (node == node->parent->left) {
					node = node->parent;
					rbtree_right_rotate(root, sentinel, node);
				}

				rbtree_black(node->parent);
				rbtree_red(node->parent->parent);
				rbtree_left_rotate(root, sentinel, node->parent->parent);
			}
		}
	}

	rbtree_black(*root);
}

/* rbtree_insert_timer_value */
void rbtree_insert_timer_value(tag_rbtree_node *temp, tag_rbtree_node *node, tag_rbtree_node *sentinel)
{
	tag_rbtree_node  **p;

	for (;; ) {

		p = ((tag_rbtree_node *)(node->key < temp->key) ? &temp->left : &temp->right);
		if (*p == sentinel) {
			break;
		}
		temp = *p;
	}

	*p = node;
	node->parent = temp;
	node->left = sentinel;
	node->right = sentinel;
	rbtree_red(node);
}

/* rbtree_delete */
void rbtree_delete(tag_rbtree *tree, tag_rbtree_node *node)
{
	unsigned int	   red;
	tag_rbtree_node  **root, *sentinel, *subst, *temp, *w;

	/* a binary tree delete */
	root = &tree->root;
	sentinel = tree->sentinel;

	if (node->left == sentinel) {
		temp = node->right;
		subst = node;

	}
	else if (node->right == sentinel) {
		temp = node->left;
		subst = node;

	}
	else {
		subst = rbtree_min(node->right, sentinel);
		temp = subst->right;
	}

	if (subst == *root) {
		*root = temp;
		rbtree_black(temp);

		/* DEBUG stuff */
		node->left = nullptr;
		node->right = nullptr;
		node->parent = nullptr;
		node->key = 0;

		return;
	}

	red = rbtree_is_red(subst);
	if (subst == subst->parent->left) {
		subst->parent->left = temp;

	}
	else {
		subst->parent->right = temp;
	}

	if (subst == node) {
		temp->parent = subst->parent;
	}
	else {

		if (subst->parent == node) {
			temp->parent = subst;
		}
		else {
			temp->parent = subst->parent;
		}

		subst->left = node->left;
		subst->right = node->right;
		subst->parent = node->parent;
		rbtree_copy_color(subst, node);

		if (node == *root) {
			*root = subst;

		}
		else {
			if (node == node->parent->left) {
				node->parent->left = subst;
			}
			else {
				node->parent->right = subst;
			}
		}

		if (subst->left != sentinel) {
			subst->left->parent = subst;
		}

		if (subst->right != sentinel) {
			subst->right->parent = subst;
		}
	}

	/* DEBUG stuff */
	node->left = nullptr;
	node->right = nullptr;
	node->parent = nullptr;
	node->key = 0;

	if (red) {
		return;
	}

	/* a delete fixup */
	while (temp != *root && rbtree_is_black(temp)) {

		if (temp == temp->parent->left) {
			w = temp->parent->right;

			if (rbtree_is_red(w)) {
				rbtree_black(w);
				rbtree_red(temp->parent);

				rbtree_left_rotate(root, sentinel, temp->parent);
				w = temp->parent->right;
			}

			if (rbtree_is_black(w->left) && rbtree_is_black(w->right)) {
				rbtree_red(w);
				temp = temp->parent;

			}
			else {
				if (rbtree_is_black(w->right)) {
					rbtree_black(w->left);
					rbtree_red(w);

					rbtree_right_rotate(root, sentinel, w);
					w = temp->parent->right;
				}

				rbtree_copy_color(w, temp->parent);
				rbtree_black(temp->parent);
				rbtree_black(w->right);

				rbtree_left_rotate(root, sentinel, temp->parent);
				temp = *root;
			}

		}
		else {
			w = temp->parent->left;

			if (rbtree_is_red(w)) {
				rbtree_black(w);
				rbtree_red(temp->parent);

				rbtree_right_rotate(root, sentinel, temp->parent);
				w = temp->parent->left;
			}

			if (rbtree_is_black(w->left) && rbtree_is_black(w->right)) {
				rbtree_red(w);
				temp = temp->parent;

			}
			else {
				if (rbtree_is_black(w->left)) {
					rbtree_black(w->right);
					rbtree_red(w);

					rbtree_left_rotate(root, sentinel, w);
					w = temp->parent->left;
				}

				rbtree_copy_color(w, temp->parent);
				rbtree_black(temp->parent);
				rbtree_black(w->left);

				rbtree_right_rotate(root, sentinel, temp->parent);
				temp = *root;
			}
		}
	}

	rbtree_black(temp);
}

/* rbtree_left_rotate */
void rbtree_left_rotate(tag_rbtree_node **root, tag_rbtree_node *sentinel, tag_rbtree_node *node)
{
	tag_rbtree_node  *temp;

	temp = node->right;
	node->right = temp->left;

	if (temp->left != sentinel) {
		temp->left->parent = node;
	}

	temp->parent = node->parent;

	if (node == *root) {
		*root = temp;

	}
	else if (node == node->parent->left) {
		node->parent->left = temp;

	}
	else {
		node->parent->right = temp;
	}

	temp->left = node;
	node->parent = temp;
}

/* rbtree_insert_value */
void rbtree_insert_value(tag_rbtree_node *temp, tag_rbtree_node *node, tag_rbtree_node *sentinel)
{
	tag_rbtree_node  **p;

	for (;; ) {

		p = (node->key < temp->key) ? &temp->left : &temp->right;

		if (*p == sentinel) {
			break;
		}

		temp = *p;
	}

	*p = node;
	node->parent = temp;
	node->left = sentinel;
	node->right = sentinel;
	rbtree_red(node);
}

/* rbtree_right_rotate */
void rbtree_right_rotate(tag_rbtree_node **root, tag_rbtree_node *sentinel, tag_rbtree_node *node)
{
	tag_rbtree_node  *temp;

	temp = node->left;
	node->left = temp->right;

	if (temp->right != sentinel) {
		temp->right->parent = node;
	}

	temp->parent = node->parent;

	if (node == *root) {
		*root = temp;

	}
	else if (node == node->parent->right) {
		node->parent->right = temp;

	}
	else {
		node->parent->left = temp;
	}

	temp->right = node;
	node->parent = temp;
}
