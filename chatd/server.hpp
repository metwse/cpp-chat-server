#ifndef SERVER_HPP
#define SERVER_HPP

extern "C" {
#include <chatd/net/tcp_listener.h>
#include <chatd/net/tcp_stream.h>
}

#include <stdint.h>


class Server {
public:
    ~Server();

    void bind(const char *host, uint16_t port);

    class ServerConnection *accept();

private:
    struct tcp_listener m_listener = { .sockfd = -1 };
};


class ServerConnection {
public:
    ~ServerConnection();

private:
    ServerConnection();
    struct tcp_stream m_connection;

    friend Server;
};

#endif
