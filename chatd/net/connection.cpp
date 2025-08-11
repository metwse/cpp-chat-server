extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <mutex>
#include <cctype>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <cstdlib>
#include <cstring>

#include <chatd/net/connection.hpp>
#include <chatd/protocol/protocol.hpp>


struct outgoing_payload {
    union {
        char *buff;
        const char *cstr;
    };
    size_t len;
    bool string_literal { false };
};

Connection::Connection(struct tcp_stream stream, ConnectionPool *pool) :
    m_stream { stream }
{}

Connection::~Connection() {
    delete m_rx_thread;
    delete m_tx_thread;

    for (size_t i = 0; i < m_tx_queue.get_size(); i++) {
        auto payload = (struct outgoing_payload *) m_tx_queue.pop_back();
        if (!payload->string_literal)
            free(payload->buff);
        delete payload;
    }
}

void Connection::rx_thread(std::shared_ptr<Connection> self) {
    while (!self->m_is_ready.load()) {
        if (self->m_is_killed)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    char *buff = NULL;
    size_t len;

    char *username = NULL, *password = NULL;
    bool authenticated = false;

    while (!self->m_is_killed.load()) {
        if (tcp_stream_readuntil(&self->m_stream, '\n', &buff, &len)) {
            self->m_is_ready = false;
            break;
        }

        if (authenticated) {
            auto payload = Payload::parse(buff, len);

            if (!payload)
                continue;

            if (payload->kind() == Payload::Kind::Command) {
                // auto _ = dynamic_cast<cmd::Command *>(payload);
            } else if (payload->kind() == Payload::Kind::Message) {
                // auto _ = dynamic_cast<msg::Message *>(payload);
            }

            if (payload)
                delete payload;
        } else {
            if (buff[len - 1] == '\r') {
                buff[len - 1] = '\0';
                len--;
            }

            if (username == NULL) {
                ssize_t i = 0;

                while (buff[i] != '\0') {
                    if (!(isalnum(buff[i]) || buff[i] == '_')) {
                        self->send_strliteral("ERR: Username contains illegal "
                                              "character(s).\nERR: Try again: ");
                        free(buff);
                        buff = NULL;
                        break;
                    }
                    i++;
                }

                if (buff) {
                    if (i < 2 || i > 20) {
                        self->send_strliteral("ERR: Username should between 2 "
                                              "and 20 characters length.\n"
                                              "ERR: Try again: ");
                        free(buff);
                    } else {
                        auto username_cpy = (char *) malloc(sizeof(char) *
                                                            (len + 1));
                        strcpy(username_cpy, buff);
                        self->send_strliteral("SRV: Hello @");
                        self->send(username_cpy, len);
                        self->send_strliteral("\nSRV: Please enter your "
                                              "password: ");
                        username = buff;
                    }
                }
            } else {
                if (strlen(buff) != len) {
                    self->send_strliteral("ERR: Password contains null-byte.");

                    free(buff);
                } else if (len < 8 || len > 64) {
                    self->send_strliteral("ERR: Password should between 8 and "
                                          "64 characters length.\nERR: Try "
                                          "again: ");

                    free(buff);
                } else {
                    password = buff;
                }
            }

            if (username && password) {
                // TODO: authentication
            }
        }
    }
}

void Connection::tx_thread(std::shared_ptr<Connection> self) {
    while (!self->m_is_ready.load())
        if (self->m_is_killed)
            return;

    bool initial_message = true;
    bool end = false;

    while (!self->m_is_killed.load()) {
        if (initial_message) {
            std::lock_guard<std::mutex> guard(self->m_tx_queue_mutex);
            initial_message = false;
        } else {
            std::unique_lock<std::mutex> lk(self->m_tx_queue_mutex);
            self->m_tx_condvar.wait(lk);
        }

        while (self->m_tx_queue.get_size() && !end) {
            auto payload = (struct outgoing_payload *)
                self->m_tx_queue.pop_back();

            if (tcp_stream_write(&self->m_stream,
                                 payload->string_literal ?
                                     payload->cstr : payload->buff,
                                 payload->len)) {
                self->m_is_ready = false;
                end = true;
            }

            if (!payload->string_literal)
                free(payload->buff);
            delete payload;
        }

        if (end)
            break;
    }
}

void Connection::send(char *buff, size_t len) {
    auto payload = new struct outgoing_payload;
    payload->buff = buff;
    payload->len = len;
    payload->string_literal = false;

    {
        std::lock_guard<std::mutex> guard(m_tx_queue_mutex);
        m_tx_queue.push_front(payload);
    }
    m_tx_condvar.notify_one();
}

void Connection::send_strliteral(const char *cstr) {
    auto payload = new struct outgoing_payload;
    payload->cstr = cstr;
    payload->len = strlen(cstr);
    payload->string_literal = true;

    {
        std::lock_guard<std::mutex> guard(m_tx_queue_mutex);
        m_tx_queue.push_front(payload);
    }
    m_tx_condvar.notify_one();
}

void Connection::terminate() {
    if (!m_is_killed.exchange(true, std::memory_order_seq_cst)) {
        tcp_stream_destroy(&m_stream);
        m_tx_condvar.notify_one();
    }
}

void Connection::shutdown() {
    terminate();
    m_rx_thread->join();
    m_tx_thread->join();
}
