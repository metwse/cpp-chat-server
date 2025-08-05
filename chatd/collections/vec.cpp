extern "C" {
#include <chatd/collections/collections.h>
#include <chatd/collections/vec.h>
}

#include <chatd/collections/vec.hpp>
#include <cstddef>

#define UNWRAP(e) \
    if (e != C_OK) \
        throw e;


Vec::Vec() {
    UNWRAP(vec_init(&this->m_vec));
}

Vec::Vec(std::size_t cap) {
    UNWRAP(vec_with_capacity(&this->m_vec, cap));
}

Vec::Vec(struct vec vec) : m_vec { vec } {}

Vec::~Vec() {
    vec_destroy(&this->m_vec);
}

void *Vec::operator[](std::size_t i) {
    return this->m_vec.arr[i];
}

Vec Vec::shrink(std::size_t cap) {
    struct vec trailing_part;

    UNWRAP(vec_shrink(&this->m_vec, cap, &trailing_part));

    return Vec(trailing_part);
}

void Vec::expand(std::size_t cap) {
    UNWRAP(vec_expand(&this->m_vec, cap));
}

void Vec::insert(std::size_t index, void *element) {
    UNWRAP(vec_insert(&this->m_vec, index, element));
}

void Vec::push(void *element) {
    UNWRAP(vec_push(&this->m_vec, element));
}

void *Vec::remove(std::size_t index) {
    void *removed;

    UNWRAP(vec_remove(&this->m_vec, index, &removed));

    return removed;
}

void Vec::remove_by_value(void *element) {
    UNWRAP(vec_remove_by_value(&this->m_vec, element));
}

std::size_t Vec::index_of(void *element) {
    return vec_index_of(&this->m_vec, element);
}

void *Vec::pop() {
    void *popped;

    UNWRAP(vec_pop(&this->m_vec, &popped));

    return popped;
}

Vec Vec::operator+(const Vec &other) const {
    Vec vec = Vec(this->get_size() + other.get_size());

    for (std::size_t i = 0; i < this->get_size(); i++)
        (*vec)[i] = this->m_vec.arr[i];

    for (std::size_t i = 0; i < other.get_size(); i++)
        (*vec)[i + this->get_size()] = other.m_vec.arr[i];

    return vec;
}
