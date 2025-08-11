#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <chatd/protocol/protocol.hpp>
#include <chatd/net/server.hpp>
#include <chatd/collections/ringbuffer.hpp>


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

    ~Connection();

    /**
     * terminate() - Gracefully terminate the connection.
     *
     * Initiates a graceful shutdown of the client connection, signals the
     * connection handler loop to exit.
     */
    void terminate();

    /**
     * send() - Send data to the connected client.
     *
     * Queues the specified data for transmission to the client. The data is
     * added to the transmission queue and will be sent asynchronously by the
     * TX thread.
     */
    void send(char *buff, size_t len);

    /**
     * send_strliteral() - Send a string literal to the client.
     * @cstring: Null-terminated string to send
     *
     * Convenience function for sending string literals to the client.
     * Automatically calculates the string length and queues it for
     * transmission via the TX thread.
     */
    void send_strliteral(const char *cstring);

    std::shared_ptr<User> user { nullptr };

private:
    friend ConnectionPool;

#ifdef _DEBUG
    friend void test();
#endif

    /**
     * Connection() - Constructor for Connection.
     * @stream: TCP stream for the client connection
     * @pool: Pointer to the connection pool managing this connection
     *
     * Initializes a new connection with the provided TCP stream and associates
     * it with the given connection pool.
     */
    Connection(struct tcp_stream stream, ConnectionPool *pool);

    /**
     * rx_thread() - Reception thread function.
     * @self: Shared pointer to the Connection instance
     *
     * Static thread functions that handle incoming/outgoing data. Runs in a
     * separate thread to receive and process messages or send messages without
     * blocking other connection operations.
     */
    static void rx_thread(std::shared_ptr<Connection> self);
    /**
     * tx_thread() - Transmission thread function.
     *
     * @see Connection::rx_thread()
     */
    static void tx_thread(std::shared_ptr<Connection> self);
    std::thread *m_rx_thread;
    std::thread *m_tx_thread;

    /**
     * @m_is_killed: Atomic flag indicating connection termination.
     */
    std::atomic_bool m_is_killed { false };
    /*
     * @m_is_ready: Atomic flag indicating connection readiness.
     */
    std::atomic_bool m_is_ready { false };

    /**
     * shutdown() - Terminate the connection and cleanup threads.
     *
     * Cleans up resources, flushes any pending data.
     */
    void shutdown();

    /**
     * handle_payload() - Process received payload data.
     *
     * Parses and processes payload data received from the client, determining
     * the payload type and dispatching to appropriate handlers for commands
     * or messages.
     */
    void handle_payload(char *buff, size_t len);

    /**
     * @m_pool: Pointer to the ConnectionPool instance that manages this
     *          connection.
     */
    ConnectionPool *m_pool;

    /**
     * @m_stream: The underlying C TCP stream handle.
     */
    struct tcp_stream m_stream;

    /**
     * @m_tx_queue: Transmission queue buffer.
     *
     * Ring buffer that stores outgoing messages waiting to be sent to the
     * client. Provides efficient FIFO queuing for the transmission thread.
     */
    Ringbuffer m_tx_queue;
    std::mutex m_tx_queue_m;
    std::condition_variable m_tx_queue_cv;
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
    friend Connection;

#ifdef _DEBUG
    friend void test();
#endif

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

    /**
     * @m_conns: Container holding active connections
     *
     * Stores and manages all active Connection objects in the pool.
     */
    Vec m_conns;
    std::mutex m_conns_m;


    /**
     * @m_gc_thread: Garbage collector thread handle
     *
     * Thread responsible for periodic cleanup of terminated connections and
     * resource management. Runs independently to maintain pool health without
     * blocking connection operations.
     */
    std::thread m_gc_thread;
    /**
     * @m_is_running: Flag controlling garbage collector thread state.
     */
    bool m_is_running { true };
};

#endif
