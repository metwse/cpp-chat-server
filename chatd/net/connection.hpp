#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <atomic>
#include <mutex>
#include <thread>

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
    /**
     * Connection() - Constructor for Connection
     * @stream: TCP stream for the client connection
     * @pool: Pointer to the connection pool managing this connection
     *
     * Initializes a new connection with the provided TCP stream and associates
     * it with the given connection pool.
     */
    Connection(struct tcp_stream stream, ConnectionPool *pool);

    /**
     * operator()() - Starts the client connection handler loop
     *
     * Main processing loop for handling client messages and maintaining the
     * connection.
     */
    void operator()();

    /**
     * ready() - Mark connection as ready for processing
     *
     * Signals that the connection has been fully initialized and is ready to
     * begin processing client requests. Sets up any final connection state
     * before entering the main processing loop.
     */
    void ready();

    /**
     * terminate() - Gracefully terminate the connection
     *
     * Initiates a graceful shutdown of the client connection. Cleans up
     * resources, flushes any pending data, and signals the connection handler
     * loop to exit.
     */
    void terminate();

    /**
     * clean() - Clean up connection resources
     */
    void clean();

    std::atomic_bool *is_active;
    std::atomic_bool *is_ready;

private:
    friend ConnectionPool;

    /**
     * @m_stream: TCP stream handle for client communication
     *
     * @see chatd/net/tcp/stream.h
     */
    struct tcp_stream m_stream;

    std::thread *m_thread;

    /**
     * @m_pool: Pointer to the connection pool
     *
     * Reference to the ConnectionPool that manages this connection instance
     * for resource management and cleanup.
     */
    ConnectionPool *m_pool;
};

/**
 * class ConnectionPool - Manages multiple client connections
 *
 * Provides connection pooling functionality for managing multiple concurrent
 * client connections to the chatd server.
 */
class ConnectionPool {
public:
    /**
     * ConnectionPool() - Constructor for ConnectionPool
     *
     * Initializes an empty connection pool ready to accept and manage client
     * connections.
     */
    ConnectionPool();

    /**
     * terminate() - Terminate all connections and shutdown the pool
     *
     * Initiates shutdown of the entire connection pool by terminating all
     * active connections, stopping the garbage collector thread, and preparing
     * for cleanup. Called during server shutdown or pool destruction.
     */
    void terminate();

    /**
     * push() - Add a new connection to the pool
     * @stream: TCP stream for the new client connection
     *
     * Creates a new Connection object from the provided TCP stream and adds it
     * to the pool for management. The connection will be handled in a thread.
     */
    void push(struct tcp_stream stream);

private:
    friend Server;
    friend void test();

    /**
     * init_gc() - Initialize the garbage collection thread
     *
     * Sets up and starts the garbage collector thread responsible for cleaning
     * up terminated connections and reclaiming resources. Called during
     * `ConnectionPool` initialization.
     */
    void init_gc();

    std::atomic_bool m_running { true };

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
