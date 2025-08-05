#ifndef SERVER_HPP
#define SERVER_HPP

extern "C" {
#include <chatd/net/tcp/listener.h>
}

#include <chatd/collections/vec.hpp>

#include <cstdint>
#include <mutex>


class Connection;
class ConnectionTerminateCallback;

/**
 * class Server - chatd server
 */
class Server {
public:
    Server() :
        m_listener { .sockfd = -1 },
        m_conn_thread_pool {},
        m_conn_thread_pool_mutex { new std::mutex }
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

private:
    friend Connection;

    /**
     * @m_listener: Underlying TCP listener
     *
     * @see chatd/net/tcp/listener.h
     */
    tcp_listener m_listener;

    /**
     * @conn_pool: Pool of active client connections.
     * @conn_pool_mutex: Mutex to protect conn_pool access.
     */
    Vec m_conn_thread_pool;
    std::mutex *m_conn_thread_pool_mutex;
};

#endif
