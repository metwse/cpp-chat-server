extern "C" {
#include <chatd/net/server.h>
}

#include <chatd/server.hpp>

#include <stdint.h>


Server::Server(const char *host, uint16_t port) {
    this->srv_status = server_init(&this->m_srv, host, port);
}

Server::~Server() {
    server_destroy(&this->m_srv);
}
