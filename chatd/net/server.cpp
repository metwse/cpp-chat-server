extern "C" {
#include <chatd/net/tcp/listener.h>
#include <chatd/net/tcp/stream.h>
}

#include <chatd/collections/vec.hpp>

#include <cstdint>
#include <thread>

#include <chatd/net/connection.hpp>
#include <chatd/net/server.hpp>


Server::~Server() {
    tcp_listener_destroy(&this->m_listener);
    delete this->m_conn_thread_pool_mutex;
}

void Server::bind(const char *host, uint16_t port) {
    auto init_result = tcp_listener_init(&this->m_listener, host, port);

    if (init_result)
        throw init_result;
}

void Server::serve_forever() {
    struct tcp_stream stream;

    while (!tcp_listener_accept(&this->m_listener, &stream)) {
        auto conn = Connection(stream);
        std::lock_guard<std::mutex> thread_lock(*conn.m_thread_ready);

        std::thread **conn_thread = new std::thread *;
        *conn_thread = new std::thread(conn, this, conn_thread);

        this->m_conn_thread_pool.push(*conn_thread);
    }

    for (size_t i = 0; i < this->m_conn_thread_pool.get_size(); i++) {
        std::thread *thread = (std::thread *) this->m_conn_thread_pool[i];
        thread->join();
        delete thread;
    }
}
