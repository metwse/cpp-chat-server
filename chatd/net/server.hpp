#ifndef SERVER_HPP
#define SERVER_HPP

extern "C" {
#include <chatd/net/tcp/listener.h>
}

#include <mutex>
#include <thread>
#include <cstdint>

#include <chatd/protocol/protocol.hpp>
#include <chatd/collections/vec.hpp>


class Connection;
class ConnectionPool;

/**
 * class Server - chatd server
 */
class Server {
public:
    Server() :
        m_listener { .sockfd = -1 }
    {}

    /**
     * ~Server() - Cleans up any associated TCP listener resources
     */
    ~Server();

    /**
     * bind() - Bind the server to a given host and port
     * @host: IP address to bind.
     * @port: TCP port number to listen on.
     */
    void bind(const char *host, uint16_t port);

    /**
     * serve_forever() - Start accepting and handling connections
     */
    void serve_forever();

#ifdef _DEBUG
    size_t conn_limit{};

    ConnectionPool *connection_pool { nullptr };
#endif

private:
#ifdef _DEBUG
    friend void test();
#endif

    /**
     * @m_listener: Underlying TCP listener
     *
     * @see chatd/net/tcp/listener.h
     */
    tcp_listener m_listener;
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

    template<typename T>
    std::shared_ptr<T> get(const char *name);

    template<typename T>
    bool push(std::shared_ptr<T> *);

    template<typename T>
    bool remove(const char *name);


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
     * @m_users: Container holding users
     */
    Vec m_users;
    std::mutex m_users_m;

    /**
     * @m_channels: Container holding channels
     */
    Vec m_channels;
    std::mutex m_channels_m;

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


#include <chatd/net/server/connection_pool.ipp>

#endif
