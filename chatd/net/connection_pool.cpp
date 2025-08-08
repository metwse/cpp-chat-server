extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <thread>
#include <chrono>

#include <chatd/net/connection.hpp>


ConnectionPool::ConnectionPool() {
    this->init_gc();
}

void ConnectionPool::terminate() {
    this->m_running = false;
    this->m_gc_thread.join();
}

void ConnectionPool::init_gc() {
    this->m_gc_thread = std::thread([this] () {
        while (this->m_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(64));
            std::lock_guard<std::mutex> lock(this->m_mutex);

            for (size_t i = 0; i < this->m_conns.get_size(); i++) {
                Connection *conn = (Connection *) this->m_conns[i];
                if (!conn->is_active->load() &&
                    conn->is_ready->load()) {
                    this->m_conns.remove(i);
                    conn->terminate();
                    conn->clean();
                    i--;
                }
            }
        }

        for (size_t i = 0; i < this->m_conns.get_size(); i++) {
            Connection *conn = (Connection *) this->m_conns[i];
            if (conn->is_active->load())
                conn->terminate();
            if (conn->is_ready->load())
                conn->clean();
        }
    });
}

void ConnectionPool::push(struct tcp_stream stream) {
    auto *conn = new Connection { stream, this };
    {
        std::lock_guard<std::mutex> lock(this->m_mutex);
        std::thread *conn_thread = new std::thread(*conn);
        conn->m_thread = conn_thread;
        this->m_conns.push(conn);
    }
    conn->ready();
}
