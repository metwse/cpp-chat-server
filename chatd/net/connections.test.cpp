extern "C" {
#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>
}

#include <ctime>
#include <thread>
#include <cassert>
#include <cstdlib>

#include <chatd/net/server.hpp>


#define TEST_HOST "127.0.0.1"
#define TEST_PORT 15535


void connection_instantexit()
{
    struct tcp_stream conn;
    assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

    tcp_stream_destroy(&conn);
}

void connection()
{
    struct tcp_stream conn;
    assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 32));

    tcp_stream_destroy(&conn);
}

void test() {
    srand(time(NULL));

    for (int _fuzz = 0; _fuzz < 16; _fuzz++) {
        Server srv;
        srv.bind(TEST_HOST, TEST_PORT);

        auto srv_thread = std::thread([&srv] () {
            srv.serve_forever();
        });

        auto client_count = (rand() % 10) + 1;
        srv.conn_limit = client_count;

        std::thread *client_threads = new std::thread[client_count];

        for (int i = 0; i < client_count / 2; i++)
            client_threads[i] = std::thread(connection_instantexit);

        for (int i = client_count / 2; i < client_count; i++)
            client_threads[i] = std::thread(connection);

        for (int i = 0; i < client_count; i++)
            client_threads[i].join();

        assert(!tcp_listener_destroy(&srv.m_listener));

        delete[] client_threads;
        srv_thread.join();
    }
}

int main() {
    test();
}
