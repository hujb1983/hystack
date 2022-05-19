/*
 * Copyright (C) AsRepository, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#pragma once
#include "event.h"

#define EVENT_TIMER_INFINITE		(unsigned long long) -1
#define EVENT_TIMER_LAZY_DELAY		300

 /* events_timer_rbtree */
extern tag_rbtree	events_timer_rbtree;

/* events_timer */
class event_timer
{
public:
	event_timer() {}
	virtual ~event_timer() {}

public:
	static void del_timer(tag_event *ev);
	static void	add_timer(tag_event *ev, unsigned long long timer);

public:
	static int  init();
	static int  no_left(void);
	static void expire(void);

public:
	static unsigned long long find();
};

