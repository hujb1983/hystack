/*
 * Copyright (C) AsRepository, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include "event.h"
#include "event_timer.h"
#include "event_rbtree.h"

tag_rbtree				event_timer_rbtree;
static tag_rbtree_node	event_timer_sentinel;


/* event_timer::init */
int event_timer::init() {
	rbtree_init(&event_timer_rbtree, &event_timer_sentinel, rbtree_insert_timer_value);
	return 0;
}

/* event_timer::expire */
void event_timer::expire()
{
	tag_event			*ev;
	tag_rbtree_node		*node, *root, *sentinel;

	sentinel = event_timer_rbtree.sentinel;

	for (;; )
	{
		root = event_timer_rbtree.root;

		if (root == sentinel) {
			return;
		}

		node = rbtree_min(root, sentinel);

		if ((rbtree_int)(node->key - core_current_msec) > 0) {
			return;
		}

		ev = (tag_event *)((char *)node - offsetof(tag_event, key_timer));

		LOG_DEBUG(nullptr) << "event timer del: "
			<< ev->key_timer.key;

		rbtree_delete(&event_timer_rbtree, &ev->key_timer);

		ev->timer_set = 0;
		ev->timed_out = 1;
		ev->ptr_handler(ev);
	}
}


/* event_timer::no_left*/
int event_timer::no_left(void)
{
	tag_event			*ev;
	tag_rbtree_node		*node, *root, *sentinel;

	sentinel = event_timer_rbtree.sentinel;
	root = event_timer_rbtree.root;

	if (root == sentinel) {
		return 0;
	}

	for (node = rbtree_min(root, sentinel); node;
		node = rbtree_next(&event_timer_rbtree, node))
	{
		ev = (tag_event *)((char *)node - offsetof(tag_event, key_timer));

		if (!ev->cancelable) {
			return RET_AGAIN;
		}
	}

	return 0;
}

/* event_timer::find */
unsigned long long event_timer::find()
{
	rbtree_key			 timer;
	tag_rbtree_node		*node, *root, *sentinel;

	if (event_timer_rbtree.root == &event_timer_sentinel) {
		return EVENT_TIMER_INFINITE;
	}

	root = event_timer_rbtree.root;
	sentinel = event_timer_rbtree.sentinel;

	node = rbtree_min(root, sentinel);

	timer = (rbtree_int)(node->key - core_current_msec);

	return (rbtree_key)(timer > 0 ? timer : 0);
}



/* event_timer::del_timer */
void event_timer::del_timer(tag_event *ev)
{
	rbtree_delete(&event_timer_rbtree, &ev->key_timer);
	ev->key_timer.left = nullptr;
	ev->key_timer.right = nullptr;
	ev->key_timer.parent = nullptr;
	ev->timer_set = 0;
}

/* event_timer::add_timer */
void event_timer::add_timer(tag_event *ev, unsigned long long timer)
{
	unsigned long long		key;
	int						diff;

	key = core_current_msec + timer;
	if (ev->timer_set) {

		diff = (int)(key - ev->key_timer.key);
		if (abs(diff) < TIMER_LAZY_DELAY) {
			return;
		}

		event_timer::del_timer(ev);
	}

	ev->key_timer.key = key;
	rbtree_insert(&event_timer_rbtree, &ev->key_timer);

	ev->timer_set = 1;
}
