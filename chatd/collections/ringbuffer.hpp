#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <chatd/collections/ringbuffer.h>
#include <chatd/collections/vec.hpp>
#include <chatd/collections/vec.h>
#include <cstddef>

/**
 * class Ringbuffer - C++ wrapper for a generic dynamic ringbuffer
 *
 * Wraps `struct ringbuffer` providing RAII and object-oriented access.
 *
 * @see `collections/ringbuffer.h` for underlying structure and behaivor.
 */
class Ringbuffer {
public:
    Ringbuffer();
    Ringbuffer(std::size_t capacity);
    Ringbuffer(struct ringbuffer);
    Ringbuffer(const struct vec *);

    ~Ringbuffer();

    void *operator[](std::size_t i);

    Vec *shrink(std::size_t cap);
    void expand(std::size_t cap);

    void insert(std::size_t i, void *);
    void *remove(std::size_t);

    void push_front(void *);
    void *pop_front();
    void push_back(void *);
    void *pop_back();

    std::size_t get_size() const {
        return this->m_rb.size;
    }
    std::size_t get_cap() const {
        return this->m_rb.cap;
    }

private:
    /**
     * @m_rb: Underlying C dynamic vector
     */
    struct ringbuffer m_rb;
};

#endif
