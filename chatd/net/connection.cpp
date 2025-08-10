extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <atomic>
#include <thread>

#include <chatd/net/connection.hpp>
#include <chatd/protocol/protocol.hpp>


Connection::Connection(struct tcp_stream stream, ConnectionPool *pool) :
    m_stream { stream }
{
}

Connection::~Connection() {
    terminate();

    m_receiver_thread->join();
    delete this->m_receiver_thread;
}

void Connection::receiver_thread(Connection *self) {
    while (!self->m_is_ready.load())
        ;

    char *buff = NULL;
    size_t len;

    while (!self->m_is_killed.load()) {
        if (tcp_stream_readuntil(&self->m_stream, '\n', &buff, &len) !=
            TCP_STREAM_OK) {
            self->m_is_ready = false;
            break;
        }

        auto payload = Payload::parse(buff, len);

        if (payload)
            delete payload;
    }
}


void Connection::terminate() {
    if (!this->m_is_killed.exchange(true))
        tcp_stream_destroy(&this->m_stream);
}
