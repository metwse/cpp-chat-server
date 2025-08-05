#include <chatd/collections/collections.h>
#include <chatd/collections/ringbuffer.h>
#include <chatd/collections/vec.h>

#include <stddef.h>
#include <stdlib.h>


enum cresult ringbuffer_init(struct ringbuffer *rb)
{
	return ringbuffer_with_capacity(rb, VEC_EXPAND_DELTA);
}

enum cresult ringbuffer_with_capacity(struct ringbuffer *rb, size_t cap)
{
	if (cap == 0)
		rb->arr = NULL;
	else
		rb->arr = malloc(sizeof(void *) * cap);

	if (rb->arr == NULL)
		return C_NOMEM;

	rb->cap = cap;
	rb->tail = 0;
	rb->head = 0;
	rb->size = 0;

	return C_OK;
}

void ringbuffer_from_vec(struct ringbuffer *rb, const struct vec *src)
{
	rb->cap = src->cap;
	rb->tail = 0;
	if (src->cap == src->size)
		rb->head = 0;
	else
		rb->head = src->size;
	rb->size = src->size;

	rb->arr = src->arr;
}

void ringbuffer_destroy(struct ringbuffer *rb)
{
	if (rb->arr != NULL)
		free(rb->arr);
	rb->arr = NULL;
}

void *ringbuffer_get(struct ringbuffer *rb, size_t index)
{
	return rb->arr[(rb->tail + index) % rb->cap];
}

enum cresult ringbuffer_shrink(struct ringbuffer *rb, size_t cap,
			       struct vec *dst)
{
	if (dst)
		vec_with_capacity(dst, 0);

	if (rb->cap < cap)
		return C_CAP_LOWER;
	else if (rb->cap == cap)
		return C_OK;

	if (cap == 0) {
		if (rb->arr)
			free(rb->arr);
		rb->arr = NULL;
		rb->size = rb->cap = 0;
		rb->head = rb->tail = 0;

		return C_OK;
	}

	// Special case of no shifting required - shrinking will not infer with
	// any element in the buffer.
	if (rb->tail + rb->size <= cap) {
		void **new_arr = realloc(rb->arr, sizeof(void *) * cap);
		if (new_arr == NULL)
			return C_NOMEM;

		rb->arr = new_arr;
		rb->cap = cap;
		return C_OK;
	}

	if (rb->size >= cap) {
		size_t stripped_count = rb->size - cap;

		if (dst) {
			enum cresult vec_result = vec_with_capacity(dst,
					       stripped_count);

			if (vec_result)
				return vec_result;

			for (size_t i = 0; i < stripped_count; i++) {
				rb->head = ringbuffer_dec(rb->head, rb->cap);
				dst->arr[stripped_count - i - 1] = rb->arr[rb->head];
			}

			dst->size = stripped_count;
		} else {
			rb->head = (rb->tail + cap) % rb->cap;
		}
	}

	if (rb->head > rb->tail) {
		for (size_t i = 0; i < cap; i++)
			rb->arr[i] = rb->arr[i + rb->tail];
	} else if (rb->size > 0) {
		void **hold_head = NULL;
		if (rb->head > 0) {
			hold_head = malloc(sizeof(void *) * rb->head);
			if (hold_head == NULL)
				return C_NOMEM;
		}

		for (int i = 0; i < rb->head; i++)
			hold_head[i] = rb->arr[i];

		for (size_t i = 0; i < cap - rb->head; i++)
			rb->arr[i] = rb->arr[i + rb->tail];
		for (size_t i = 0; i < rb->head; i++)
			rb->arr[cap - rb->head + i] = hold_head[i];

		if (hold_head)
			free(hold_head);
	}

	void **new_arr = realloc(rb->arr, sizeof(void *) * cap);
	if (new_arr == NULL)
		return C_NOMEM;

	rb->arr = new_arr;
	rb->cap = cap;

	if (rb->size > cap)
		rb->size = cap;

	rb->tail = 0;
	rb->head = (rb->tail + rb->size) % rb->cap;

	return C_OK;
}

enum cresult ringbuffer_expand(struct ringbuffer *rb, size_t cap)
{
	if (rb->cap > cap)
		return C_CAP_GREATER;
	else if (rb->cap == cap)
		return C_OK;

	void *new_arr = rb->arr == NULL ?
		malloc(cap * sizeof(void *)) :
		realloc(rb->arr, cap * sizeof(void *));

	if (new_arr == NULL)
		return C_NOMEM;

	rb->arr = new_arr;

	if (rb->size == 0 || rb->head > rb->tail) {
		// No shifting is required if the buffer is empty
		// or if the head is ahead of the tail (wrapped state).
		rb->cap = cap;
	} else {
		// If head equals or greater than tail, shift the initial
		// elements to the end of the new buffer space.
		size_t shift_count = cap - rb->cap;
		if (rb->head >= shift_count) {
			for (size_t i = 0; i < shift_count; i++)
				rb->arr[rb->cap + i] = rb->arr[i];

			for (size_t i = 0; i < rb->head - shift_count; i++)
				rb->arr[i] = rb->arr[shift_count + i];

			rb->head -= shift_count;
		} else {
			for (size_t i = 0; i < rb->head; i++)
				rb->arr[rb->cap + i] = rb->arr[i];

			rb->head = (rb->tail + rb->size) % cap;
		}

		rb->cap = cap;
	}

	return C_OK;
}

enum cresult ringbuffer_insert(struct ringbuffer *rb, size_t index, void *e)
{
	if (index > rb->size)
		return C_OUT_OF_BOUNDS;

	if (rb->size == rb->cap) {
		enum cresult expand_result = ringbuffer_expand(rb,
					 rb->cap + RINGBUFFER_EXPAND_DELTA);
		if (expand_result)
			return expand_result;
	}

	size_t cursor = rb->head, prev;
	for (size_t i = 0; i < rb->size - index; i++) {
		prev = ringbuffer_dec(cursor, rb->cap);

		rb->arr[cursor] = rb->arr[prev];
		cursor = prev;
	}
	rb->arr[cursor] = e;

	rb->head = ringbuffer_inc(rb->head, rb->cap);
	rb->size++;

	return C_OK;
}

enum cresult ringbuffer_remove(struct ringbuffer *rb, size_t index, void **dst)
{
	if (index >= rb->size)
		return C_OUT_OF_BOUNDS;

	size_t cursor = (rb->tail + index) % rb->cap, next;
	*dst = rb->arr[cursor];

	for (size_t i = index; i < rb->size; i++) {
		next = ringbuffer_inc(cursor, rb->cap);

		rb->arr[cursor] = rb->arr[next];
		cursor = next;
	}

	rb->head = ringbuffer_dec(rb->head, rb->cap);
	rb->size--;

	return C_OK;
}

enum cresult ringbuffer_push_front(struct ringbuffer *rb, void *e)
{
	if (rb->size == rb->cap) {
		enum cresult expand_result = ringbuffer_expand(rb,
					 rb->cap + RINGBUFFER_EXPAND_DELTA);
		if (expand_result)
			return expand_result;
	}

	rb->arr[rb->head] = e;

	rb->head = ringbuffer_inc(rb->head, rb->cap);
	rb->size++;

	return C_OK;
}

enum cresult ringbuffer_pop_front(struct ringbuffer *rb, void **dst)
{
	if (rb->size == 0)
		return C_EMPTY;

	rb->head = ringbuffer_dec(rb->head, rb->cap);

	*dst = rb->arr[rb->head];

	rb->size--;

	return C_OK;
}

enum cresult ringbuffer_push_back(struct ringbuffer *rb, void *e)
{
	if (rb->size == rb->cap) {
		enum cresult expand_result = ringbuffer_expand(rb,
					 rb->cap + RINGBUFFER_EXPAND_DELTA);
		if (expand_result)
			return expand_result;
	}

	rb->tail = ringbuffer_dec(rb->tail, rb->cap);

	rb->arr[rb->tail] = e;

	rb->size++;

	return C_OK;
}

enum cresult ringbuffer_pop_back(struct ringbuffer *rb, void **dst)
{
	if (rb->size == 0)
		return C_EMPTY;

	*dst = rb->arr[rb->tail];

	rb->tail = ringbuffer_inc(rb->tail, rb->cap);
	rb->size--;

	return C_OK;
}
