/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, JAN, 2021
 */

#include <mutex>
#include <core_config.h>
#include <module/modules_event.h>

#include "event.h"
#include <event/event_connection.h>
#include <event/event_timer.h>

extern std::mutex				accept_mutex;
extern unsigned int				accept_use_mutex;
extern unsigned int				accept_disabled;
extern unsigned int				accept_mutex_held;
extern unsigned long long		accept_mutex_delay;

/* timer_resolution */
extern unsigned int			timer_resolution;


/* event_process */
namespace event_process
{
	/* queue_middle */
	tag_queue * queue_middle(tag_queue *queue)
	{
		tag_queue  *middle, *next;

		middle = queue_head(queue);
		if (middle == queue_last(queue)) {
			return middle;
		}

		next = queue_head(queue);

		for (;; ) {
			middle = queue_next(middle);

			next = queue_next(next);
			if (next == queue_last(queue)) {
				return middle;
			}

			next = queue_next(next);
			if (next == queue_last(queue)) {
				return middle;
			}
		}
	}


	/* queue_sort */
	void queue_sort(tag_queue *queue, int(*cmp)(const tag_queue *, const tag_queue *))
	{
		tag_queue  *q, *prev, *next;

		q = queue_head(queue);
		if (q == queue_last(queue)) {
			return;
		}

		for (q = queue_head(q); q != queue_sentinel(queue); q = next) {

			prev = queue_prev(q);
			next = queue_next(q);
			queue_remove(q);

			do {
				if (cmp(prev, q) <= 0) {
					break;
				}

				prev = queue_prev(prev);

			} while (prev != queue_sentinel(queue));

			queue_insert_after(prev, q);
		}
	}

	/* process_posted */
	void process_posted(tag_cycle *cycle, tag_queue * posted)
	{
		tag_queue	  *q;
		tag_event	  *ev;

		while (!queue_empty(posted)) {

			q = queue_head(posted);
			ev = queue_data(q, tag_event, queue);

			posted_event(ev);
			if (ev->ptr_handler != nullptr) {
				ev->ptr_handler(ev);
			}
		}
	}


	/* move_posted_next */
	void move_posted_next(tag_cycle *cycle)
	{
		tag_queue	*q;
		tag_event	*ev;

		for (q = queue_head(&posted_next_events);
			q != queue_sentinel(&posted_next_events);
			q = queue_next(q))
		{
			ev = queue_data(q, tag_event, queue);

			ev->ready = 1;
			ev->available = -1;
		}

		queue_add(&posted_events, &posted_next_events);
		queue_init(&posted_next_events);
	}


	/* send_lowat */
	int send_lowat(tag_connection *c, unsigned int lowat);


	/* write_handler */
	int write_handler(tag_event *wev, unsigned int lowat)
	{
		tag_connection		*c;

		if (lowat) {
			c = (tag_connection *)wev->data;

			if (send_lowat(c, lowat) == -1) {
				return -1;
			}
		}

		if (proc_event_flags & EVENT_USE_CLEAR) {

			if (!wev->active && !wev->ready) {
				if (proc_add_event(wev, EVENT_WRITE,
					EVENT_CLEAR | (lowat ? EVENT_LOWAT : 0))
					== -1) {
					return -1;
				}
			}
			return 0;
		}
		else if (proc_event_flags & EVENT_USE_LEVEL) {

			/* select, poll, /dev/poll */
			if (!wev->active && !wev->ready) {
				if (proc_add_event(wev, EVENT_WRITE, EVENT_LEVEL) == -1) {
					return -1;
				}
				return 0;
			}

			if (wev->active && wev->ready) {
				if (proc_del_event(wev, EVENT_WRITE, EVENT_LEVEL) == -1) {
					return -1;
				}
				return 0;
			}
		}
		else if (proc_event_flags & EVENT_USE_EVENTPORT) {

			/* event ports */
			if (!wev->active && !wev->ready) {
				if (proc_add_event(wev, EVENT_WRITE, 0) == -1) {
					return -1;
				}
				return 0;
			}
			if (wev->one_shot && wev->ready) {
				if (proc_del_event(wev, EVENT_WRITE, 0) == -1) {
					return -1;
				}
				return 0;
			}
		}

		return 0;
	}


	/* send_lowat */
	int send_lowat(tag_connection *c, unsigned int lowat)
	{
		int  snd_low_at;

		if (lowat == 0 || c->send_lowat) {
			return 0;
		}

		snd_low_at = (int)lowat;

		if (setsockopt(c->fd, SOL_SOCKET, SO_SNDLOWAT, (const char *)&snd_low_at, sizeof(int)) == -1) {
			return -1;
		}

		c->send_lowat = 1;
		return 0;
	}
};


/*  process_event_and_timers */
void process_event_and_timers(tag_cycle * cycle)
{
	unsigned int		flags;
	unsigned long long	timer, delta;

	if (timer_resolution) {
		timer = TIMER_INFINITE;
		flags = 0;
	}
	else {
		timer = event_timer::find();
		flags = TIMES_UPDATE;
	}

	if (accept_use_mutex)
	{
		if (accept_disabled > 0) {
			accept_disabled--;
		}
		else {

			/* if (AsTryLockAcceptMutex(cycle) == AS_ERROR) {
				return;
			} */

			if (accept_mutex_held) {
				flags |= EVENT_POST;
			}
			else {
				if (timer == TIMER_INFINITE
					|| timer > accept_mutex_delay)
				{
					timer = accept_mutex_delay;
				}
			}
		}
	}

	if (!queue_empty(&posted_next_events)) {
		event_process::move_posted_next(cycle);
		timer = 0;
	}

	delta = timer;
	proc_events(cycle, delta, flags);
	delta = core_current_msec - delta;

	event_process::process_posted(cycle, &posted_accept_events);

	if (accept_mutex_held) {
		accept_mutex.unlock();
	}

	event_timer::expire();
	event_process::process_posted(cycle, &posted_events);
}