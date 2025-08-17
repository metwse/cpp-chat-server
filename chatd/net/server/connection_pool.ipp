#pragma once
#ifndef CONNECTION_POOL_IPP
#define CONNECTION_POOL_IPP

#include <memory>

#include <chatd/net/server.hpp>


template<typename T>
std::shared_ptr<T> ConnectionPool::get(const char *name) {
    std::mutex *mutex;
    Vec *vec;

    if (typeid(T) == typeid(User)) {
        mutex = &m_users_m;
        vec = &m_users;
    } else {
        mutex = &m_channels_m;
        vec = &m_channels;
    }

    std::lock_guard<std::mutex> guard(*mutex);

    for (size_t i = 0; i < m_users.get_size(); i++) {
        auto elem = *(std::shared_ptr<T> *) (*vec)[i];

        if (!strcmp(name, elem->name)) {
            return elem;
        }
    }

    return std::shared_ptr<T>(nullptr);
}

template<typename T>
bool ConnectionPool::push(std::shared_ptr<T> *new_user) {
    std::lock_guard<std::mutex> guard(m_users_m);

    for (size_t i = 0; i < m_users.get_size(); i++) {
        auto user = *(std::shared_ptr<T> *) m_users[i];

        if (!strcmp((*new_user)->name, user->name))
            return false;
    }

    m_users.push(new_user);
    return true;
}

template<typename T>
bool ConnectionPool::remove(const char *name) {
    std::mutex *mutex;
    Vec *vec;

    if (typeid(T) == typeid(User)) {
        mutex = &m_users_m;
        vec = &m_users;
    } else {
        mutex = &m_channels_m;
        vec = &m_channels;
    }

    std::lock_guard<std::mutex> guard(*mutex);

    for (size_t i = 0; i < m_users.get_size(); i++) {
        auto elem = *(std::shared_ptr<T> *) (*vec)[i];

        if (!strcmp(name, elem->name)) {
            delete elem;
            (*vec).remove(i);
            return true;
        }
    }

    return false;
}


#endif
