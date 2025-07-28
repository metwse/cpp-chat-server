extern "C" {
#include <chatd/net/tcp_listener.h>
#include <chatd/net/tcp_stream.h>
}

#include <chatd/server.hpp>

#include <stdint.h>


void Server::bind(const char *host, uint16_t port) {
    enum tcp_listener_result result = tcp_listener_init(&this->m_listener,
                                                        host, port);

    if (result)
        throw result;
}

Server::~Server() {
    tcp_listener_destroy(&this->m_listener);
}

class ServerConnection Server::accept() {
    ServerConnection server_connection;

    enum tcp_listener_result result = tcp_listener_accept(&this->m_listener,
                                                          &server_connection.m_connection);

    if (result)
        throw result;

    return server_connection;
}

ServerConnection::~ServerConnection() {
    tcp_stream_destroy(&this->m_connection);
}

ServerConnection::ServerConnection() {}
