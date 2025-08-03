#include <chatd/collections/ringbuffer.h>

#include <stdlib.h>
#include <time.h>
#include <assert.h>


int main()
{
	struct ringbuffer rb;
	srand(time(NULL));

	int data[1024];
	for (int i = 0; i < 1024; i++)
		data[i] = rand();

	// test: push_front/pop_back (without expansion)
	ringbuffer_with_capacity(&rb, 16);
	for (int i = 0; i < 8; i++)
		assert(!ringbuffer_push_front(&rb, &data[i]));

	for (int i = 0; i < 8; i++) {
		int *out;
		assert(!ringbuffer_pop_back(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	for (int i = 8; i < 24; i++)
		assert(!ringbuffer_push_front(&rb, &data[i]));

	for (int i = 8; i < 24; i++) {
		int *out;
		assert(!ringbuffer_pop_back(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	ringbuffer_destroy(&rb);

	// test: push_front/pop_back (auto growing)
	ringbuffer_with_capacity(&rb, 16);
	for (int i = 0; i < 64; i++)
		assert(!ringbuffer_push_front(&rb, &data[i]));

	for (int i = 0; i < 64; i++) {
		int *out;
		assert(!ringbuffer_pop_back(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	for (int i = 0; i < 96; i++)
		assert(!ringbuffer_push_front(&rb, &data[i]));

	for (int i = 0; i < 96; i++) {
		int *out;
		assert(!ringbuffer_pop_back(&rb, (void **) &out));

		assert(*out == data[i]);
	}

	ringbuffer_destroy(&rb);

	// test: push_front/pop_back (auto growing, fuzz)
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
				assert(!ringbuffer_push_front(&rb, &data[j + index]));

			for (int j = 0; j < rand2; j++) {
				int *out;
				assert(!ringbuffer_pop_back(&rb, (void **) &out));

				assert(*out == data[j + index]);
			}

			for (int j = rand1; j < push_count; j++)
				assert(!ringbuffer_push_front(&rb, &data[j + index]));

			for (int j = rand2; j < push_count; j++) {
				int *out;
				assert(!ringbuffer_pop_back(&rb, (void **) &out));

				assert(*out == data[j + index]);
			}

			index += push_count;
		}

		ringbuffer_destroy(&rb);
	}

}
