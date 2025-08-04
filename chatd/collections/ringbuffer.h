#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <chatd/collections/collections.h>
#include <chatd/collections/vec.h>
#include <stddef.h>

#define RINGBUFFER_EXPAND_DELTA 16

/**
 * struct ringbuffer - Generic double-ended ring buffer
 * @cap: Allocated capacity of the buffer
 * @head: Index of the first element (for popping from front)
 * @tail: Index one past the last element (for pushing to back)
 * @size: Number of elements in the ringbuffer
 * @arr: Underlying array of void pointers
 *
 * The default usage of this type as a queue is to use `push_back` to add to
 * the queue, and `pop_front` to remove from the queue. `remove`, `expand`, and
 * `insert` push from the back in this manner.
 */
struct ringbuffer {
	size_t cap;
	size_t head;
	size_t tail;
	size_t size;
	void **arr;
};

#define RINGBUFFER_ITER_BEGIN(rb, i, elem) do { \
	size_t RINGBUFFER_ ## elem = rb.tail, i; \
	void *elem; \
	for (i = 0; i < rb.size; i++) { \
		elem = rb.arr[RINGBUFFER_ ## elem]; \
		RINGBUFFER_ ## elem = ringbuffer_inc(RINGBUFFER_ ## elem, \
						     rb.cap);
#define RINGBUFFER_ITER_END } } while(0);

#define RINGBUFFER_ITERREV_BEGIN(rb, i, elem) do { \
	if (rb.size == 0) \
		break; \
	size_t RINGBUFFER_ ## elem = rb.head, i; \
	void *elem; \
	for (i = rb.size - 1;; i--) { \
		RINGBUFFER_ ## elem = ringbuffer_dec(RINGBUFFER_ ## elem, \
						     rb.cap); \
		elem = rb.arr[RINGBUFFER_ ## elem];
#define RINGBUFFER_ITERREV_END \
	if (i == 0) \
		break; \
	} } while(0);


/**
 * ringbuffer_init() - Initializes a ring buffer with default capacity
 */
enum cresult ringbuffer_init(struct ringbuffer *);

/**
 * ringbuffer_with_capacity() - Initializes a ring buffer with given capacity
 */
enum cresult ringbuffer_with_capacity(struct ringbuffer *, size_t cap);

/**
 * ringbuffer_from_vec - Initialize a ringbuffer from a vector
 * @src: Pointer to the vector whose contents will populate the ringbuffer
 */
void ringbuffer_from_vec(struct ringbuffer *, const struct vec *src);

/**
 * ringbuffer_with_destroy() - Frees arr field of a ring buffer, elements are
 * untouched
 */
void ringbuffer_destroy(struct ringbuffer *);

/**
 * ringbuffer_get() - Get a pointer to an element at a given index in the ring
 *                    buffer
 *
 * Returns a pointer to the element at the specified index. The index is
 * relative to the current tail position of the buffer.
 */
void *ringbuffer_get(struct ringbuffer *, size_t index);

/**
 * ringbuffer_dec() - Decrement a ring buffer index with wrap-around.
 */
static inline size_t ringbuffer_dec(size_t val, size_t cap)
{
	return val == 0 ? cap - 1 : val - 1;
}

/**
 * ringbuffer_inc() - Increment a ring buffer index with wrap-around.
 */
static inline size_t ringbuffer_inc(size_t val, size_t cap)
{
	return val == cap - 1 ? 0 : val + 1;
}

/**
 * ringbuffer_shrink() - Splits the vector by shrinking capacity
 * @cap: New capacity to retain
 * @dst: Vector to receive trimmed elements
 *
 * If ringbuffer size exceeds the @cap, keeps the first @cap elements in @vec,
 * and moves the rest into @dst. Useful for slicing off the tail of a
 * ring buffer. Trimmed data will be discarded if @dst is NULL.
 */
enum cresult ringbuffer_shrink(struct ringbuffer *, size_t cap, struct vec *dst);

/**
 * ringbuffer_expand() - Increases the size of the vec
 */
enum cresult ringbuffer_expand(struct ringbuffer *, size_t cap);

/**
 * ringbuffer_insert - Insert an element at the specified index
 *
 * Inserts an element at the given logical index in the ringbuffer. Elements at
 * and after the index will be shifted to make room. The index must be between
 * 0 and the current size of the buffer (inclusive).
 */
enum cresult ringbuffer_insert(struct ringbuffer *, size_t index, void *);

/**
 * ringbuffer_remove - Remove an element at the specified index
 *
 * Removes an element at the given logical index and optionally returns it.
 * Elements after the index will be shifted to fill the gap. The index must be
 * less than the current size of the buffer.
 */
enum cresult ringbuffer_remove(struct ringbuffer *rb, size_t index, void **dst);

/**
 * ringbuffer_push_front() - Push an element to the front of the ringbuffer
 */
enum cresult ringbuffer_push_front(struct ringbuffer *, void *);

/**
 * ringbuffer_pop_front() - Remove an element from the front of the ringbuffer
 * @dst: Out-pointer to store removed element
 */
enum cresult ringbuffer_pop_front(struct ringbuffer *, void **dst);

/**
 * ringbuffer_push_back() - Push an element to the front of the ringbuffer
 */
enum cresult ringbuffer_push_back(struct ringbuffer *, void *);

/**
 * ringbuffer_pop_back() - Remove an element from the front of the ringbuffer
 * @dst: Out-pointer to store removed element
 */
enum cresult ringbuffer_pop_back(struct ringbuffer *, void **dst);

#endif
