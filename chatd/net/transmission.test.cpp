extern "C" {
#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>
}

#include <ctime>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdlib>

#include <chatd/net/server.hpp>
#include <chatd/net/connection.hpp>


#define TEST_HOST "127.0.0.1"
#define TEST_PORT 15536

#define CONNECTION_COUNT 2

static bool end = false;

static char *payloads_sent[CONNECTION_COUNT][16];
static char *payloads_recv[CONNECTION_COUNT][16];


char *malloc_string(const char *);
void stream_thread(size_t id);

void test() {
    Server srv;
    srv.bind(TEST_HOST, TEST_PORT);

    auto srv_thread = std::thread([&srv] () {
        srv.serve_forever();
    });

    while (srv.connection_pool == NULL)
        ;
    auto pool = srv.connection_pool;

    srv.conn_limit = CONNECTION_COUNT;

    std::thread *client_threads = new std::thread[CONNECTION_COUNT];

    for (size_t i = 0; i < CONNECTION_COUNT; i++) {
        client_threads[i] = std::thread(stream_thread, i);

        while (true) {
            std::lock_guard<std::mutex> guard(srv.connection_pool->m_mutex);
            if (pool->m_conns.get_size() == i + 1)
                break;
        }
    }

    for (size_t i = 0; i < CONNECTION_COUNT; i++) {
        std::shared_ptr<Connection> conn = *(std::shared_ptr<Connection> *)
            pool->m_conns[i];

        for (size_t j = 0; j < 16; j++)
            conn->send(payloads_sent[i][j], 16);
    }

    end = true;

    for (size_t i = 0; i < CONNECTION_COUNT; i++)
        client_threads[i].join();

    assert(!tcp_listener_destroy(&srv.m_listener));

    delete[] client_threads;
    srv_thread.join();
}

int main() {
    srand(time(NULL));

    for (size_t i = 0; i < CONNECTION_COUNT; i++) {
        for (size_t j = 0; j < 16; j++) {
            auto str = (char *) malloc(sizeof(char) * 16);

            for (size_t k = 0; k < 15; k++) {
                char c = rand() % 10 + '0';
                if (c == '\n')
                    k--;
                else
                    str[k] = c;
            }

            str[15] = '\n';
            payloads_sent[i][j] = str;
            payloads_recv[i][j] = (char *) malloc(sizeof(char) * 16);
            memcpy(payloads_recv[i][j], str, 16);
        }
    }

    test();
}

char *malloc_string(const char *str_literal){
    char *str = (char *) malloc(sizeof(char *) * (strlen(str_literal) + 1));
    memcpy(str, str_literal, strlen(str_literal) + 1);

    return str;
}

void stream_thread(size_t i) {
    struct tcp_stream conn;
    assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

    char *buff;
    size_t len;

    for (size_t j = 0; j < 16; j++) {
        assert(!tcp_stream_readuntil(&conn, '\n', &buff, &len));

        assert(!memcmp(buff, payloads_recv[i][j], 15));
        free(payloads_recv[i][j]);
        free(buff);
    }

    while (!end)
        ;

    tcp_stream_destroy(&conn);
}
