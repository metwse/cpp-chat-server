extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <ctime>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <chatd/protocol/protocol.hpp>
#include <chatd/net/server.hpp>
#include <chatd/net/connection.hpp>


#define TEST_HOST "127.0.0.1"
#define TEST_PORT 15534

#define CONNECTION_COUNT 16


void stream_thread(size_t id);

void test() {
    Server srv;
    srv.bind(TEST_HOST, TEST_PORT);

    auto srv_thread = std::thread([&srv] () {
        srv.serve_forever();
    });

    while (srv.connection_pool == nullptr)
        ;
    srv.conn_limit = CONNECTION_COUNT + 1;

    std::thread *client_threads = new std::thread[CONNECTION_COUNT];

    for (size_t i = 0; i < CONNECTION_COUNT; i++) {
        client_threads[i] = std::thread(stream_thread, i);
    }

    for (size_t i = 0; i < CONNECTION_COUNT; i++)
        client_threads[i].join();

    delete[] client_threads;

    struct tcp_stream conn;
    assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));
    tcp_stream_destroy(&conn);

    srv_thread.join();
}

int main() {
    srand(time(NULL));

    test();
}

void stream_thread(size_t i) {
    struct tcp_stream conn;
    assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

    char *buff;
    size_t len;

    // Ignore the welcome message sent by server.
    assert(!tcp_stream_readuntil(&conn, ':', &buff, &len));
    assert(!memcmp(buff, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE) - 2));
    free(buff);
    tcp_stream_readuntil(&conn, ' ', &buff, &len);
    free(buff);

    tcp_stream_write(&conn, "test_user\n", 10);
    tcp_stream_write(&conn, "test_password\n", 10);

    switch (rand() % 3) {
    case 0:
        tcp_stream_write(&conn, "y\n", 2);
        break;
    case 1:
        tcp_stream_write(&conn, "n\n", 2);
        break;
    default:
        break;
    }

    tcp_stream_destroy(&conn);
}
