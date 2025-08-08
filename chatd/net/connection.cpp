extern "C" {
#include <chatd/net/tcp/stream.h>
}

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
    while (!is_ready->load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    char *buff = NULL;
    size_t len;

    while (this->is_active->load()) {
        if (tcp_stream_readuntil(&this->m_stream, '\n', &buff, &len) != TCP_STREAM_OK) {
            *this->is_ready = false;
            break;
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
}

void Connection::clean() {
    delete this->is_ready;
    delete this->is_active;
    delete this->m_thread;
    delete this;
}
