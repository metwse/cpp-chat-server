#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <atomic>
#include <mutex>
#include <thread>

#include <chatd/collections/ringbuffer.hpp>
#include <chatd/net/server.hpp>


class ConnectionPool;

/**
 * class Connection - chatd client connection
 *
 * Manages individual client connections to the chatd server. Handles the
 * connection lifecycle and message processing loop.
 */
class Connection {
public:
    Connection(Connection &) = delete;

private:
    friend ConnectionPool;

    /**
     * Connection() - Constructor for Connection
     * @stream: TCP stream for the client connection
     * @pool: Pointer to the connection pool managing this connection
     *
     * Initializes a new connection with the provided TCP stream and associates
     * it with the given connection pool.
     */
    Connection(struct tcp_stream stream, ConnectionPool *pool);

    ~Connection();

    static void receiver_thread(Connection *self);

    /**
     * terminate() - Gracefully terminate the connection
     *
     * Initiates a graceful shutdown of the client connection. Cleans up
     * resources, flushes any pending data, and signals the connection handler
     * loop to exit.
     */
    void terminate();

    std::atomic_bool m_is_killed { false };
    std::atomic_bool m_is_ready { false };

    struct tcp_stream m_stream;

    std::thread *m_receiver_thread;
};

/**
 * class ConnectionPool - Manages multiple client connections
 *
 * Provides connection pooling functionality for managing multiple concurrent
 * client connections to the chatd server.
 */
class ConnectionPool {
public:
    ConnectionPool(ConnectionPool &) = delete;

private:
    friend Server;
    friend void test();


    /**
     * ConnectionPool() - Constructor for ConnectionPool
     *
     * Initializes an empty connection pool ready to accept and manage client
     * connections.
     */
    ConnectionPool();

    ~ConnectionPool();

    /**
     * push() - Add a new connection to the pool
     * @stream: TCP stream for the new client connection
     *
     * Creates a new Connection object from the provided TCP stream and adds it
     * to the pool for management. The connection will be handled in a thread.
     */
    void push(struct tcp_stream stream);

    bool m_running { true };

    /**
     * @m_conns: Container holding active connections
     *
     * Stores and manages all active Connection objects in the pool.
     */
    Vec m_conns;

    /**
     * @m_mutex: Mutex for thread-safe access to connection pool
     */
    std::mutex m_mutex;

    /**
     * @m_gc_thread: Garbage collector thread handle
     *
     * Thread responsible for periodic cleanup of terminated connections and
     * resource management. Runs independently to maintain pool health without
     * blocking connection operations.
     */
    std::thread m_gc_thread;
};

#endif
