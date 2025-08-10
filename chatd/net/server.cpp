extern "C" {
#include <chatd/net/tcp/listener.h>
#include <chatd/net/tcp/stream.h>
}

#include <cassert>
#include <cstdint>

#include <chatd/collections/vec.hpp>
#include <chatd/net/connection.hpp>
#include <chatd/net/server.hpp>


Server::~Server() {
    tcp_listener_destroy(&this->m_listener);
}

void Server::bind(const char *host, uint16_t port) {
    auto init_result = tcp_listener_init(&this->m_listener, host, port);

    if (init_result)
        throw init_result;
}

void Server::serve_forever() {
    struct tcp_stream stream;
    ConnectionPool pool;

#ifdef _DEBUG
    size_t conn_count = 0;

    this->connection_pool = &pool;
#endif

    while (!tcp_listener_accept(&this->m_listener, &stream)) {
        pool.push(stream);

#ifdef _DEBUG
        if (this->conn_limit && (++conn_count) == this->conn_limit)
            break;
#endif
    }
}
