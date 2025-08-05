#include <chatd/collections/vec.hpp>

#include <cassert>
#include <cstdlib>
#include <ctime>


int main() {
    srand(time(NULL));
    Vec vec = Vec();

    int data[128];

    // test: Vec::push (vec_expand, vec_push)
    for (int i = 0; i < 128; i++) {
        data[i] = rand();
        vec.push(&data[i]);
    }

    // test: Vec::pop (vec_pop)
    for (int i = 0; i < 128; i++)
        assert(*(int *) vec.pop() == data[127 - i]);

    // test: Vec::insert
    for (int i = 0; i < 128; i += 2)
        vec.push(&data[i]);
    for (int i = 1; i < 128; i += 2)
        vec.insert(i, &data[i]);

    // test: Vec::remove
    for (int i = 0; i < 128; i++)
        assert(*(int *) vec.remove(0) == data[i]);

    Vec other_vec;

    for (int i = 0; i < 64; i++) {
        vec.push(&data[i]);
        other_vec.push(&data[64 + i]);
    }

    // test: Vec::operator+
    Vec concation = vec + other_vec;

    for (int i = 0; i < 128; i++) {
        assert(*(int *) (*concation)[i] == data[i]);
    }
}
