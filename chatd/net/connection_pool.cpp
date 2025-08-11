extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

#include <chatd/net/connection.hpp>
#include <chatd/protocol/protocol.hpp>


ConnectionPool::ConnectionPool() {
    m_gc_thread = std::thread([this] () {
        bool terminated = false;

        // Deletes non-ready threads, which stopped receiving/sending and are
        // killed. If the server has terminated, then delete all threads.
        while (!terminated) {
            std::this_thread::sleep_for(std::chrono::milliseconds(64));

            if (!m_is_running)
                terminated = true;

            std::lock_guard<std::mutex> guard(m_conns_m);
            for (ssize_t i = 0; i < (ssize_t) m_conns.get_size(); i++) {
                auto conn = (std::shared_ptr<Connection> *) m_conns[i];

                if (terminated || !(*conn)->m_is_ready.load()) {
                    if (!terminated)
                        m_conns.remove(i--);

                    (*conn)->shutdown();
                    delete conn;
                }
            }
        };
    });
}

ConnectionPool::~ConnectionPool() {
    m_is_running = false;
    m_gc_thread.join();
}

void ConnectionPool::push(struct tcp_stream stream) {
    auto conn = new std::shared_ptr<Connection>(new Connection { stream, this });

    (*conn)->send_strliteral(WELCOME_MESSAGE);

    (*conn)->m_rx_thread = new std::thread(Connection::rx_thread, *conn);
    (*conn)->m_tx_thread = new std::thread(Connection::tx_thread, *conn);

    (*conn)->m_is_ready = true;
    {
        std::lock_guard<std::mutex> guard(m_conns_m);
        m_conns.push(conn);
    }
}
