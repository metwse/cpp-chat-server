#ifndef SERVER_HPP
#define SERVER_HPP

extern "C" {
#include <chatd/net/tcp/listener.h>
}

#include <chatd/collections/vec.hpp>

#include <cstdint>


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

    ConnectionPool *connection_pool{NULL};
#endif

private:
    friend Connection;

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

#endif
