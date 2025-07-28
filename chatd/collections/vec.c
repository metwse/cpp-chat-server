#include <chatd/collections/vec.h>

#include <stdlib.h>


enum vec_result vec_init(struct vec *v)
{
	return vec_with_capacity(v, EXPAND_DELTA);
}

enum vec_result vec_with_capacity(struct vec *v, size_t cap)
{
	v->data = malloc(sizeof(void *) * cap);
	if (v->data == NULL)
		return VEC_NOMEM;

	v->size = 0;
	v->cap = cap;

	return VEC_OK;
}

void vec_destroy(struct vec *v)
{
	free(v->data);
}

enum vec_result vec_shrink(struct vec *v, size_t cap, struct vec *dst)
{
	return VEC_OK;
}

enum vec_result vec_expand(struct vec *v, size_t cap)
{
	if (v->cap > cap)
		return VEC_CAP_GREATER;

	void *new_data = realloc(v->data, cap * sizeof(void *));

	if (new_data == NULL)
		return VEC_NOMEM;

	v->data = new_data;

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
		v->data[i] = v->data[i - 1];

	v->data[index] = e;
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
		*dst = v->data[index];

	for (int i = index; i < v->size; i++)
		v->data[i] = v->data[i + 1];

	v->size--;

	return VEC_OK;
}

enum vec_result vec_pop(struct vec *v, void **dst)
{
	return vec_remove(v, v->size - 1, dst);
}
