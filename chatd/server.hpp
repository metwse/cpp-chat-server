#ifndef SERVER_HPP
#define SERVER_HPP

extern "C" {
#include <chatd/net/server.h>
}

#include <stdint.h>


class Server {
public:
    Server() = delete;
    ~Server();
    Server(const char *host, uint16_t port);

    enum server_result srv_status;

private:
    struct server m_srv;
};

#endif
