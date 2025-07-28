#include <chatd/net/tcp_listener.h>
#include <chatd/server.hpp>

#include <stdlib.h>
#include <iostream>


int main() {
    Server server;

    try {
        server.bind("0.0.0.0", 3001);
    } catch (enum tcp_listener_result r) {
        std::cerr << "Failed to start server. enum server_result: " <<
            r << std::endl;
        return EXIT_FAILURE;
    };

    try {
        ServerConnection connection = server.accept();
    } catch (enum tcp_listener_result r) {
        return EXIT_FAILURE;
    };
}
