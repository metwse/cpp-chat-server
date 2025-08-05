#include <chatd/collections/ringbuffer.hpp>

#include <cassert>
#include <cstdlib>
#include <ctime>


int main() {
    srand(time(NULL));

    for (int _fuzz = 0; _fuzz < 128; _fuzz++) {
        Ringbuffer rb;

        // Shifts head/tail cursor to a random index
        int cursor = rand() % 128;

        for (int i = 0; i < cursor; i++)
            rb.push_front(NULL);

        for (int i = 0; i < cursor; i++)
            rb.pop_back();

        int data[128];

        for (int i = 0; i < 128; i++)
            data[i] = rand();

        // test: Ringbuffer::push_front (push_front, expand)
        for (int i = 64; i < 128; i++)
            rb.push_front(&data[i]);

        // test: Ringbuffer::push_back (push_back, expand)
        for (int i = 63; i >= 0; i--)
            rb.push_back(&data[i]);

        // test: Ringbuffer::operator[] (get)
        for (int i = 0; i < 128; i++)
            assert(rb[i] == &data[i]);

        Vec *v = rb.shrink(64);

        // test: Ringbuffer::pop_front (pop_front)
        for (int i = 64; i < 128; i++)
            assert((*v)[i - 64] == &data[i]);

        // test: Ringbuffer::pop_front (pop_front)
        for (int i = 63; i >= 32; i--)
            assert(rb.pop_front() == &data[i]);

        // test: Ringbuffer::pop_front (pop_front)
        for (int i = 0; i < 32; i++)
            assert(rb.pop_back() == &data[i]);

        delete v;
    }
}
