#include <chatd/collections/collections.h>
#include <chatd/collections/vec.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


enum cresult vec_init(struct vec *v)
{
	return vec_with_capacity(v, VEC_EXPAND_DELTA);
}

enum cresult vec_with_capacity(struct vec *v, size_t cap)
{
	if (cap > 0) {
		v->arr = malloc(sizeof(void *) * cap);
		if (v->arr == NULL)
			return C_NOMEM;
	} else {
		v->arr = NULL;
	}

	v->size = 0;
	v->cap = cap;

	return C_OK;
}

void vec_destroy(struct vec *v)
{
	if (v->arr != NULL)
		free(v->arr);
	v->arr = NULL;
}

enum cresult vec_shrink(struct vec *v, size_t cap, struct vec *dst)
{
	if (v->cap < cap)
		return C_CAP_LOWER;
	else if (v->cap == cap)
		return C_OK;

	if (dst != NULL) {
		if (cap < v->size &&
		    (vec_expand(dst, v->size - cap) != C_NOMEM)) {
			for (size_t i = 0; i < v->size - cap; i++)
				dst->arr[i] = v->arr[cap + i];
			dst->size = v->size - cap;
		} else {
			dst->size = dst->cap = 0;
			dst->arr = NULL;
		}
	}

	v->cap = cap;
	if (v->size > cap)
		v->size = cap;

	if (cap == 0) {
		if (v->arr != NULL)
			free(v->arr);
		v->arr = NULL;
	} else {
		v->arr = realloc(v->arr, sizeof(void *) * cap);
	}

	return C_OK;
}

enum cresult vec_expand(struct vec *v, size_t cap)
{
	if (v->cap > cap)
		return C_CAP_GREATER;
	else if (v->cap == cap)
		return C_OK;

	void *new_arr = v->arr == NULL ?
		malloc(cap * sizeof(void *)) :
		realloc(v->arr, cap * sizeof(void *));

	if (new_arr == NULL)
		return C_NOMEM;

	v->arr = new_arr;
	v->cap = cap;

	return C_OK;
}

enum cresult vec_insert(struct vec *v, size_t index, void *e)
{
	if (index > v->size)
		return C_OUT_OF_BOUNDS;

	if (v->size == v->cap) {
		enum cresult expand_result = vec_expand(v,
					     v->cap + VEC_EXPAND_DELTA);

		if (expand_result != C_OK)
			return expand_result;
	}

	for (size_t i = v->size; i > index; i--)
		v->arr[i] = v->arr[i - 1];

	v->arr[index] = e;
	v->size++;

	return C_OK;
}

enum cresult vec_push(struct vec *v, void *e)
{
	return vec_insert(v, v->size, e);
}

enum cresult vec_remove(struct vec *v, size_t index, void **dst)
{
	if (index >= v->size)
		return C_OUT_OF_BOUNDS;

	if (dst != NULL)
		*dst = v->arr[index];

	for (size_t i = index; i < v->size - 1; i++)
		v->arr[i] = v->arr[i + 1];

	v->size--;

	return C_OK;
}

size_t vec_index_of(struct vec *v, void *e)
{
	size_t found_index;
	bool found = false;
	for (found_index = 0; found_index < v->size; found_index++)
		if (v->arr[found_index] == e) {
			found = true;
			break;
		}

	return found ? found_index : SIZE_MAX;
}

enum cresult vec_remove_by_value(struct vec *v, void *e)
{
	size_t i = vec_index_of(v, e);

	return i == SIZE_MAX ? C_NOT_FOUND : vec_remove(v, i, NULL);
}

enum cresult vec_pop(struct vec *v, void **dst)
{
	return vec_remove(v, v->size - 1, dst);
}
