#include <chatd/collections/ringbuffer.h>
#include <chatd/collections/vec.h>

#include <stdlib.h>
#include <time.h>
#include <assert.h>


/**
 * test_init() - Test ringbuffer initialization
 */
void test_init();

/**
 * test_push_pop() - Test ringbuffer with antagonistic push/pop functions
 * @pop: Pointer to a pop function
 * @push: Pointer to a push function
 *
 * Tests the correctness of the ringbuffer when used with antagonistic push
 * and pop operations - that is, one end for pushing and the opposite end for
 * popping. This setup simulates typical queue (FIFO) behavior.
 */
void test_push_pop(enum cresult (*pop)(struct ringbuffer *, void **),
		   enum cresult (*push)(struct ringbuffer *, void *));

/**
 * test_shrink() - Test ringbuffer shrink capability
 */
void test_shrink();

/**
 * test_shrink() - Test ringbuffer insert/remove functions
 */
void test_insert();

int main()
{
	srand(time(NULL));
	test_init();

	test_push_pop(ringbuffer_pop_back, ringbuffer_push_front);
	test_push_pop(ringbuffer_pop_front, ringbuffer_push_back);

	test_shrink();
	test_insert();
}

void test_insert()
{
	struct ringbuffer rb;
	assert(!ringbuffer_with_capacity(&rb, 64));

	rb.head = rb.tail = rand() % 64;

	int data[64];
	for (int i = 0; i < 64; i++)
		data[i] = rand();

	for (int i = 0; i < 64; i += 2)
		assert(!ringbuffer_push_front(&rb, &data[i]));

	for (int i = 1; i < 64; i += 2)
		assert(!ringbuffer_insert(&rb, i, &data[i]));

	for (int i = 0; i < 64; i++)
		assert(ringbuffer_get(&rb, i) == &data[i]);

	for (int i = 63; i > 0; i -= 2) {
		int *out;
		assert(!ringbuffer_remove(&rb, i, (void **) &out));

		assert(*out == data[i]);
	}

	for (int i = 0; i < 64; i += 2) {
		int *out;
		assert(!ringbuffer_remove(&rb, 0, (void **) &out));

		assert(*out == data[i]);
	}

	ringbuffer_destroy(&rb);
}

void test_shrink()
{
	struct ringbuffer rb;

	// test: shrink filled ringbuffer, initial head/tail is random
	for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
		assert(!ringbuffer_with_capacity(&rb, 64));

		rb.head = rb.tail = rand() % 64;

		int data[64];
		for (int i = 0; i < 64; i++) {
			data[i] = rand();
			assert(!ringbuffer_push_front(&rb, &data[i]));
		}

		assert(!ringbuffer_shrink(&rb, 32, NULL));

		for (int i = 0; i < 32; i++)
			assert(ringbuffer_get(&rb, i) == &data[i]);

		for (int i = 0; i < 16; i++) {
			int *out;
			assert(!ringbuffer_pop_back(&rb, (void **) &out));
		}

		for (int i = 0; i < 16; i++)
			assert(ringbuffer_get(&rb, i) == &data[i + 16]);

		assert(!ringbuffer_shrink(&rb, 8, NULL));

		for (int i = 0; i < 8; i++)
			assert(ringbuffer_get(&rb, i) == &data[i + 16]);

		ringbuffer_shrink(&rb, 0, NULL);
	}

	// test: shrink non-filled ringbuffer, initial head/tail is random
	for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
		int size = rand() % 32 + 1;

		assert(!ringbuffer_with_capacity(&rb, 64));

		rb.head = rb.tail = rand() % 64;

		int data[size];
		for (int i = 0; i < size; i++) {
			data[i] = rand();
			assert(!ringbuffer_push_front(&rb, &data[i]));
		}

		struct vec trimmed;

		int new_size = (rand() % 32 + 1) % size;
		assert(!ringbuffer_shrink(&rb, new_size, &trimmed));

		for (int i = 0; i < new_size; i++)
			assert(ringbuffer_get(&rb, i) == &data[i]);

		// test: trimmed part copied into the vec
		if (trimmed.size)
			for (int i = 0; i < size - new_size; i++)
				assert(trimmed.arr[i] == &data[new_size + i]);

		vec_destroy(&trimmed);

		ringbuffer_shrink(&rb, 0, NULL);
	}
}

void test_init()
{
	struct ringbuffer rb;

	// test: init
	assert(!ringbuffer_init(&rb));
	ringbuffer_destroy(&rb);

	// test: init with capacity
	for (int i = 0; i < 128; i++) {
		assert(!ringbuffer_with_capacity(&rb, rand() % 256 + 1));
		ringbuffer_destroy(&rb);
	}

	// test: init from vec
	for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
		size_t size = rand() % 128 + 1;
		struct vec v;
		int data[size];

		// used vec_init instead of vec_with_capacity to test vec to
		// ringbuffer casting when cap != size
		assert(!vec_init(&v));
		for (int i = 0; i < size; i++) {
			data[i] = rand();
			assert(!vec_push(&v, &data[i]));
		}

		ringbuffer_from_vec(&rb, &v);
		for (int i = 0; i < size; i++) {
			int *out;
			assert(!ringbuffer_pop_front(&rb, (void **) &out));

			assert(*out == data[size - 1 - i]);
		}

		ringbuffer_from_vec(&rb, &v);
		for (int i = 0; i < size; i++) {
			int *out;
			assert(!ringbuffer_pop_back(&rb, (void **) &out));

			assert(*out == data[i]);
		}

		ringbuffer_from_vec(&rb, &v);
		for (int i = 0; i < size; i++)
			assert(*(int *) ringbuffer_get(&rb, i) == data[i]);

		ringbuffer_destroy(&rb);
	}
}

void test_push_pop(enum cresult (*pop)(struct ringbuffer *, void **),
		   enum cresult (*push)(struct ringbuffer *, void *))
{
	struct ringbuffer rb;

	int data[1024];
	for (int i = 0; i < 1024; i++)
		data[i] = rand();

	// test: push/pop (without expansion)
	ringbuffer_with_capacity(&rb, 16);
	for (int i = 0; i < 8; i++)
		assert(!push(&rb, &data[i]));

	for (int i = 0; i < 8; i++) {
		int *out;
		assert(!pop(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	for (int i = 8; i < 24; i++)
		assert(!push(&rb, &data[i]));

	for (int i = 8; i < 24; i++) {
		int *out;
		assert(!pop(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	ringbuffer_destroy(&rb);

	// test: push/pop (auto growing)
	ringbuffer_with_capacity(&rb, 16);
	for (int i = 0; i < 64; i++)
		assert(!push(&rb, &data[i]));

	for (int i = 0; i < 64; i++) {
		int *out;
		assert(!pop(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	for (int i = 0; i < 96; i++)
		assert(!push(&rb, &data[i]));

	for (int i = 0; i < 96; i++) {
		int *out;
		assert(!pop(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	ringbuffer_destroy(&rb);

	// test: push/pop (auto growing, fuzz)
	for (int _fuzz = 0; _fuzz < 8; _fuzz++) {
		assert(!ringbuffer_with_capacity(&rb, 4));

		int index = 0;
		// test scenario:
		// loop 8 times:
		//	decide total count to be pushed to the buffer, push_count
		//	push first rand1 (rand1 < push_count) elements
		//	delete first rand2 (rand2 < rand1) elements
		//	push remaining elements (push_count - rand1)
		//	delete all elements (push_count - rand2)
		for (int i = 0; i < 8; i++) {
			int push_count = rand() % 127 + 1;
			int rand1 = rand() % push_count + 1;
			int rand2 = rand() % rand1;

			for (int j = 0; j < rand1; j++)
				assert(!push(&rb, &data[j + index]));

			for (int j = 0; j < rand2; j++) {
				int *out;
				assert(!pop(&rb, (void **) &out));

				assert(*out == data[j + index]);
			}

			for (int j = rand1; j < push_count; j++)
				assert(!push(&rb, &data[j + index]));

			for (int j = rand2; j < push_count; j++) {
				int *out;
				assert(!pop(&rb, (void **) &out));

				assert(*out == data[j + index]);
			}

			index += push_count;
		}

		ringbuffer_destroy(&rb);
	}
}
