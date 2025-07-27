#include <chatd/server.hpp>

#include <stdlib.h>
#include <iostream>


int main() {
    Server server = Server("0.0.0.0", 3001);

    if (server.srv_status != 0) {
        std::cerr << "Failed to start server. enum server_result: " <<
            server.srv_status << std::endl;
        return EXIT_FAILURE;
    }
}
