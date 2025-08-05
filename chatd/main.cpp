#include <chatd/net/tcp/listener.h>
#include <chatd/net/server.hpp>

#include <stdlib.h>
#include <iostream>


int main() {
    Server server;

    try {
        server.bind("0.0.0.0", 3001);
        server.serve_forever();
    } catch (enum tcp_listener_result r) {
        std::cerr << "Failed to start server. Error code: " << r << std::endl;
        return EXIT_FAILURE;
    };
}
