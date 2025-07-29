#ifndef VEC_HPP
#define VEC_HPP

#include <chatd/collections/vec.h>
#include <cstddef>

/**
 * class Vec - C++ wrapper for a generic dynamic vector
 *
 * Wraps `struct vec` providing RAII and object-oriented access.
 *
 * @see `collections/vec.h` for underlying structure and behaivor.
 */
class Vec {
public:
    Vec();
    Vec(std::size_t capacity);
    Vec(struct vec);

    ~Vec();

    Vec shrink(std::size_t cap);
    void expand(std::size_t cap);

    void insert(std::size_t index, void *element);
    void push(void *element);

    void *remove(std::size_t index);
    void *pop();

    std::size_t get_size() const;
    std::size_t get_cap() const;
    void **as_ref();

    /**
     * Vec::operator+() - Concatenates two vectors
     * @other: Vector to append
     *
     * Returns a new vector containing the elements of this vector followed by
     * the elements of @other.
     *
     * Return: A new Vec instance containing the combined elements.
     */
    Vec operator+(const Vec &other) const;

private:
    /**
     * @m_vec: Underlying C dynamic vector
     */
    struct vec m_vec;
};

#endif
