#include <chatd/collections/vec.h>

#include <stdlib.h>


enum vec_result vec_init(struct vec *v)
{
	return vec_with_capacity(v, EXPAND_DELTA);
}

enum vec_result vec_with_capacity(struct vec *v, size_t cap)
{
	if (cap > 0) {
		v->arr = malloc(sizeof(void *) * cap);
		if (v->arr == NULL)
			return VEC_NOMEM;
	} else {
		v->arr = NULL;
	}

	v->size = 0;
	v->cap = cap;

	return VEC_OK;
}

void vec_destroy(struct vec *v)
{
	if (v->arr != NULL)
		free(v->arr);
}

enum vec_result vec_shrink(struct vec *v, size_t cap, struct vec *dst)
{
	if (v->cap < cap)
		return VEC_CAP_LOWER;
	else if (v->cap == cap)
		return VEC_OK;

	if (!(dst == NULL || cap >= v->size) &&
	    (vec_expand(dst, v->size - cap) != VEC_NOMEM)) {
		for (int i = 0; i < v->size - cap; i++)
			dst->arr[i] = v->arr[cap + i];
		dst->size = v->size - cap;
	}

	v->cap = cap;
	if (v->size > cap)
		v->size = cap;

	v->arr = realloc(v->arr, sizeof(void *) * cap);

	return VEC_OK;
}

enum vec_result vec_expand(struct vec *v, size_t cap)
{
	if (v->cap > cap)
		return VEC_CAP_GREATER;
	else if (v->cap == cap)
		return VEC_OK;

	void *new_arr = v->arr == NULL ?
		malloc(cap * sizeof(void *)) :
		realloc(v->arr, cap * sizeof(void *));

	if (new_arr == NULL)
		return VEC_NOMEM;

	v->arr = new_arr;
	v->cap = cap;

	return VEC_OK;
}

enum vec_result vec_insert(struct vec *v, size_t index, void *e)
{
	if (index > v->size)
		return VEC_OUT_OF_BOUNDS;

	if (v->size == v->cap) {
		enum vec_result expand_result = vec_expand(v,
					     v->cap + EXPAND_DELTA);

		if (expand_result != VEC_OK)
			return expand_result;
	}

	for (int i = v->size; i > index; i--)
		v->arr[i] = v->arr[i - 1];

	v->arr[index] = e;
	v->size++;

	return VEC_OK;
}

enum vec_result vec_push(struct vec *v, void *e)
{
	return vec_insert(v, v->size, e);
}

enum vec_result vec_remove(struct vec *v, size_t index, void **dst)
{
	if (index >= v->size)
		return VEC_OUT_OF_BOUNDS;

	if (dst != NULL)
		*dst = v->arr[index];

	for (int i = index; i < v->size - 1; i++)
		v->arr[i] = v->arr[i + 1];

	v->size--;

	return VEC_OK;
}

enum vec_result vec_pop(struct vec *v, void **dst)
{
	return vec_remove(v, v->size - 1, dst);
}
