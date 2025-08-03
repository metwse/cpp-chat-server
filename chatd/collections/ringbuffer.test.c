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


int main()
{
	srand(time(NULL));
	test_init();

	test_push_pop(ringbuffer_pop_back, ringbuffer_push_front);
	test_push_pop(ringbuffer_pop_front, ringbuffer_push_back);
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
		ringbuffer_with_capacity(&rb, 4);

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
