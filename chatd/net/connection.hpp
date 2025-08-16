#ifndef CONNECTION_HPP
#define CONNECTION_HPP

extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>

#include <chatd/protocol/protocol.hpp>
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
     * terminate() - Instantly terminates the connection and discards the
     *               message queue.
     */
    void terminate();
    /**
     * gracefully_terminate() - Gracefully terminates the connection.
     *
     * Initiates a graceful shutdown of the client connection, signals the
     * connection handler loop to exit.
     */
    void gracefully_terminate();

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

    /**
     * send_message() - Send a message to the client.
     */
    void send_message(std::shared_ptr<msg::Message> msg);

    std::shared_ptr<User> user { nullptr };

private:
    friend ConnectionPool;

    class OutgoingPayload;

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

    void push_payload(OutgoingPayload *);

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
    /*
     * @m_is_gracefully_terminated: Atomic flag the indicating connection is
     *                             currently shutting down.
     */
    std::atomic_bool m_is_gracefully_terminated { false };

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

#endif
