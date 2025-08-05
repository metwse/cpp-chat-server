#include <cstdlib>
extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <mutex>

#include <chatd/net/server.hpp>
#include <chatd/net/connection.hpp>


Connection::~Connection() {
    tcp_stream_destroy(&m_stream);
}

void Connection::start_thread() {
}

void Connection::operator()(Server *server, std::thread **conn_thread_ptr) {
    {
        std::lock_guard<std::mutex> lock(*this->m_thread_ready);
    }

    delete conn_thread_ptr;
    delete this->m_thread_ready;
}
