#ifndef VEC_H
#define VEC_H

#include <stddef.h>

#define EXPAND_DELTA 16

/**
 * struct vec - Generic dynamic array
 * @data: Array of void pointers
 * @cap: Allocated capacity
 * @size: Current element count
 */
struct vec {
	size_t cap;
	size_t size;
	void **data;
};

/**
 * enum vec_result - Return codes for vector operations
 * @VEC_OK: Operation succeeded
 * @VEC_NOMEM: Memory allocation failed
 * @VEC_EMPTY: Vector is empty
 * @VEC_OUT_OF_BOUNDS: Attempted to acces an index that is greater than size
 * @VEC_CAP_GREATER: Attempted to expand to a smaller capacity than current
 * @VEC_CAP_LOWER: Attempted to shrink to a larger capacity than current
 */
enum vec_result {
	VEC_OK,
	VEC_NOMEM,
	VEC_EMPTY,
	VEC_OUT_OF_BOUNDS,
	VEC_CAP_GREATER,
	VEC_CAP_LOWER,
};


/**
 * vec_init() - Initializes a vector with default capacity
 */
enum vec_result vec_init(struct vec *);

/**
 * vec_with_capacity() - Initializes a vector with given capacity
 */
enum vec_result vec_with_capacity(struct vec *, size_t cap);

/**
 * vec_with_destroy() - Frees data field of a vec, elements are untouched
 */
void vec_destroy(struct vec *);

/**
 * vec_shrink() - Splits the vector by shrinking capacity
 * @cap: New capacity to retain
 * @dst: Vector to receive trimmed elements
 *
 * If vec size exceeds the @cap, keeps the first @cap elements in @vec, and
 * moves the rest into @dst. Useful for slicing off the tail of a vector.
 * Trimmed data will be discarded if @dst is NULL.
 */
enum vec_result vec_shrink(struct vec *, size_t cap, struct vec *dst);

/**
 * vec_expand() - Increases the size of the vec
 */
enum vec_result vec_expand(struct vec *, size_t cap);

/**
 * vec_insert() - Inserts the element at index, shifts elements right
 */
enum vec_result vec_insert(struct vec *, size_t index, void *);

/**
 * vec_push() - Pushes an element to end of the vec
 *
 * Grows the vector if necessary.
 */
enum vec_result vec_push(struct vec *, void *);

/**
 * vec_remove() - Removes element at index, shifts elements left
 * @dst: Out-pointer to store removed element
 */
enum vec_result vec_remove(struct vec *, size_t index, void **dst);

/**
 * vec_pop() - Removes the last element
 * @dst: Out-pointer to store removed element
 */
enum vec_result vec_pop(struct vec *, void **dst);

#endif
