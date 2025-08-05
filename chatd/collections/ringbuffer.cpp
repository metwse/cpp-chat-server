extern "C" {
#include <chatd/collections/collections.h>
#include <chatd/collections/ringbuffer.h>
#include <chatd/collections/vec.h>
}

#include <chatd/collections/ringbuffer.hpp>
#include <cstddef>

#define UNWRAP(e) \
    if (e != C_OK) \
        throw e;


Ringbuffer::Ringbuffer() {
    UNWRAP(ringbuffer_init(&this->m_rb));
}

Ringbuffer::Ringbuffer(std::size_t cap) {
    UNWRAP(ringbuffer_with_capacity(&this->m_rb, cap));
}

Ringbuffer::Ringbuffer(struct ringbuffer rb)
    : m_rb(rb)
{ }

Ringbuffer::Ringbuffer(const struct vec *v) {
    ringbuffer_from_vec(&this->m_rb, v);
}

Ringbuffer::~Ringbuffer() {
    ringbuffer_destroy(&this->m_rb);
}

void *Ringbuffer::operator[](std::size_t i) {
    return ringbuffer_get(&this->m_rb, i);
}

Vec *Ringbuffer::shrink(std::size_t cap) {
    struct vec v;

    UNWRAP(ringbuffer_shrink(&this->m_rb, cap, &v));

    return new Vec(v);
}

void Ringbuffer::expand(std::size_t cap) {
    UNWRAP(ringbuffer_expand(&this->m_rb, cap));
}

void Ringbuffer::push_front(void *e) {
    UNWRAP(ringbuffer_push_front(&this->m_rb, e));
}

void *Ringbuffer::pop_front() {
    void *out;

    UNWRAP(ringbuffer_pop_front(&this->m_rb, &out));

    return out;
}

void Ringbuffer::push_back(void *e) {
    UNWRAP(ringbuffer_push_back(&this->m_rb, e));
}

void *Ringbuffer::pop_back() {
    void *out;

    UNWRAP(ringbuffer_pop_back(&this->m_rb, &out));

    return out;
}
