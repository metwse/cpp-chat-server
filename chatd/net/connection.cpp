#include <unistd.h>

#include <atomic>
#include <thread>
#include <chrono>

#include <chatd/net/connection.hpp>


Connection::Connection(struct tcp_stream stream, ConnectionPool *pool) :
    m_stream { stream },
    m_pool { pool }
{
    this->is_ready = new std::atomic_bool { false };
    this->is_active = new std::atomic_bool { false };
}

void Connection::operator()() {
    int sockfd = this->m_stream.sockfd;
    char buf[1024];

    while (!is_ready->load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    while (is_active->load()) {
        if (read(sockfd, buf, sizeof(buf)) <= 0) {
            *this->is_active = false;
            tcp_stream_destroy(&this->m_stream);
        }
    }
}

void Connection::ready() {
    *this->is_ready = true;
    *this->is_active = true;
}

void Connection::terminate() {
    *this->is_active = false;
    tcp_stream_destroy(&this->m_stream);
    this->m_thread->join();
}

void Connection::clean() {
    delete this->is_ready;
    delete this->is_active;
    delete this->m_thread;
    delete this;
}
