#include <chatd/collections/vec.h>

#include <stdlib.h>
#include <time.h>
#include <assert.h>


int main()
{
	struct vec v;
	srand(time(NULL));

	// test: vec_expand
	for (int i = 0; i < 64; i++) {
		size_t cap = rand() % 32;

		assert(!vec_with_capacity(&v, cap));
		assert(v.cap == cap);

		cap += rand() % 32;
		assert(!vec_expand(&v, cap));

		vec_destroy(&v);
	}

	int data[1024];
	assert(!vec_init(&v));
	// test: vec_push (auto growing)
	for (int i = 0; i < 1024; i++) {
		data[i] = rand();

		assert(!vec_push(&v, &data[i]));
	}

	// test: vec_pop
	for (int i = 0; i < 1024; i++) {
		int *popped_data;

		assert(!vec_pop(&v, (void **) &popped_data));
		assert(*popped_data == data[1023 - i]);
		assert(popped_data == &data[1023 - i]);
	}
	vec_destroy(&v);

	// test: vec_insert, vec_remove
	assert(!vec_init(&v));
	for (int i = 0; i < 64; i += 2)
		assert(!vec_push(&v, &data[i]));

	for (int i = 1; i < 64; i += 2)
		assert(!vec_insert(&v, i, &data[i]));

	for (int i = 0; i < 32; i++) {
		int *popped_data;

		assert(!vec_pop(&v, (void **) &popped_data));
		assert(*popped_data == data[63 - i]);
	}
	for (int i = 0; i < 16; i++) {
		int *removed_data;

		assert(!vec_remove(&v, i, (void **) &removed_data));
		assert(*removed_data == data[i * 2]);
	}
	vec_destroy(&v);


	assert(!vec_with_capacity(&v, 64));
	struct vec trailing;
	for (int i = 0; i < 64; i ++)
		assert(!vec_push(&v, &data[i]));

	int trimmed = 0;
	for (int i = 64; i > 0; i /= 2) {
		trimmed += i;
		assert(!vec_with_capacity(&trailing, i));

		assert(!vec_shrink(&v, i, &trailing));

		for (int j = 0; j < trailing.size; j++)
			assert(*(int *) trailing.arr[j] ==
			       data[128 - trimmed + j]);

		vec_destroy(&trailing);
	}

	vec_destroy(&v);
}
