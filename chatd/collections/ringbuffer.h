#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <chatd/collections/collections.h>
#include <stddef.h>

#define RINGBUFFER_EXPAND_DELTA 16

/**
 * struct ringbuffer - Generic double-ended ring buffer
 * @cap:  Allocated capacity of the buffer
 * @head: Index of the first element (for popping from front)
 * @tail: Index one past the last element (for pushing to back)
 * @arr:  Underlying array of void pointers
 */
struct ringbuffer {
	size_t cap;
	size_t head;
	size_t tail;
	void **arr;
};

#endif
