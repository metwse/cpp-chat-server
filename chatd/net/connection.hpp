#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <thread>

#include <chatd/net/server.hpp>


/**
 * class Connection - chatd client connection
 */
class Connection {
public:
    Connection() = delete;
    Connection(struct tcp_stream stream) :
        m_stream { stream },
        m_thread_ready { new std::mutex }
    {}

    ~Connection();

    /**
     * overload () - Starts the client connection handler loop.
     */
    void operator()(Server *, std::thread **);

    /**
     * start_thread() - Signals the thread to begin execution.
     *
     * Used after synchronization to notify that the thread can safely enter
     * the connection handler logic.
     */
    void start_thread();

private:
    friend Server;

    /**
     * @see chatd/net/tcp/stream.h
     */
    tcp_stream m_stream;

    /**
     * @m_thread_ready: Locks the thread until it can start listening
     */
    std::mutex *m_thread_ready;
};

#endif
