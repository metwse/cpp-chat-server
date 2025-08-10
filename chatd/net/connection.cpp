extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

#include <chatd/net/connection.hpp>
#include <chatd/protocol/protocol.hpp>


Connection::Connection(struct tcp_stream stream, ConnectionPool *pool) :
    m_stream { stream }
{
}

Connection::~Connection() {
    terminate();

    m_rx_thread->join();
    m_tx_thread->join();

    delete this->m_rx_thread;
    delete this->m_tx_thread;
}

void Connection::rx_thread(std::shared_ptr<Connection> self) {
    while (!self->m_is_ready.load()) {
        if (self->m_is_killed)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    char *buff = NULL;
    size_t len;

    while (!self->m_is_killed.load()) {
        if (tcp_stream_readuntil(&self->m_stream, '\n', &buff, &len)) {
            self->m_is_ready = false;
            break;
        }

        auto payload = Payload::parse(buff, len);

        if (payload)
            delete payload;
    }
}

void Connection::tx_thread(std::shared_ptr<Connection> self) {
    while (!self->m_is_ready.load())
        if (self->m_is_killed)
            return;

    // while (!self->m_is_killed.load())
}


void Connection::terminate() {
    if (!this->m_is_killed.exchange(true, std::memory_order_seq_cst))
        tcp_stream_destroy(&this->m_stream);
}
