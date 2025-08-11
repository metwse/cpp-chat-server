extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

#include <chatd/net/connection.hpp>


ConnectionPool::ConnectionPool() {
    m_gc_thread = std::thread([this] () {
        std::unique_lock<std::mutex> lk(m_mutex, std::defer_lock);

        while (m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(64));

            lk.lock();
            bool removed = false;
            for (size_t i = 0; i < m_conns.get_size(); i++) {
                if (removed) {
                    i--;
                    removed = false;
                }

                auto conn = (std::shared_ptr<Connection> *) m_conns[i];

                if (!(*conn)->m_is_ready.load()) {
                    m_conns.remove(i);
                    removed = true;

                    (*conn)->shutdown();
                    delete conn;
                }
            }
            lk.unlock();
        }

        lk.lock();
        for (size_t i = 0; i < m_conns.get_size(); i++) {
            auto conn = (std::shared_ptr<Connection> *) m_conns[i];

            (*conn)->shutdown();
            delete conn;
        }
    });
}

ConnectionPool::~ConnectionPool() {
    m_running = false;
    m_gc_thread.join();
}

void ConnectionPool::push(struct tcp_stream stream) {
    auto conn = new std::shared_ptr<Connection>(new Connection { stream, this });

    (*conn)->m_rx_thread = new std::thread(Connection::rx_thread, *conn);
    (*conn)->m_tx_thread = new std::thread(Connection::tx_thread, *conn);

    (*conn)->m_is_ready = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_conns.push(conn);
    }
}
