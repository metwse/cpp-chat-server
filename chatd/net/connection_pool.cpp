extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <thread>
#include <chrono>

#include <chatd/net/connection.hpp>


ConnectionPool::ConnectionPool() {
    m_gc_thread = std::thread([this] () {
        while (m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(64));
            std::lock_guard<std::mutex> lock(m_mutex);

            for (size_t i = 0; i < m_conns.get_size(); i++) {
                Connection *conn = (Connection *) m_conns[i];

                if (!conn->m_is_ready.load()) {
                    m_conns.remove(i--);
                    delete conn;
                }
            }
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        for (size_t i = 0; i < m_conns.get_size(); i++)
            delete (Connection *) m_conns[i];
    });
}

ConnectionPool::~ConnectionPool() {
    m_running = false;
    m_gc_thread.join();
}

void ConnectionPool::push(struct tcp_stream stream) {
    auto *conn = new Connection { stream, this };

    std::thread *conn_thread = new std::thread(Connection::receiver_thread, conn);
    conn->m_receiver_thread = conn_thread;
    conn->m_is_ready = true;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_conns.push(conn);
    }
}
