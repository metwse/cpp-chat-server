#ifndef VEC_H
#define VEC_H

#include <chatd/collections/collections.h>
#include <stddef.h>

#define VEC_EXPAND_DELTA 16

/**
 * struct vec - Generic dynamic array
 * @data: Array of void pointers
 * @cap: Allocated capacity
 * @size: Current element count
 */
struct vec {
	size_t cap;
	size_t size;
	void **arr;
};


/**
 * vec_init() - Initializes a vector with default capacity
 */
enum cresult vec_init(struct vec *);

/**
 * vec_with_capacity() - Initializes a vector with given capacity
 */
enum cresult vec_with_capacity(struct vec *, size_t cap);

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
enum cresult vec_shrink(struct vec *, size_t cap, struct vec *dst);

/**
 * vec_expand() - Increases the size of the vec
 */
enum cresult vec_expand(struct vec *, size_t cap);

/**
 * vec_insert() - Inserts the element at index, shifts elements right
 */
enum cresult vec_insert(struct vec *, size_t index, void *);

/**
 * vec_push() - Pushes an element to end of the vec
 *
 * Grows the vector if necessary.
 */
enum cresult vec_push(struct vec *, void *);

/**
 * vec_remove() - Removes element at index, shifts elements left
 * @dst: Out-pointer to store removed element
 */
enum cresult vec_remove(struct vec *, size_t index, void **dst);

/**
 * vec_remove() - Removes given element, shifts elements left
 */
enum cresult vec_remove_by_value(struct vec *, void *e);

/**
 * vec_find_index() - Finds the index of given element
 *
 * Returns `SIZE_MAX` if no element has been found.
 */
size_t vec_index_of(struct vec *, void *e);

/**
 * vec_pop() - Removes the last element
 * @dst: Out-pointer to store removed element
 */
enum cresult vec_pop(struct vec *, void **dst);

#endif
