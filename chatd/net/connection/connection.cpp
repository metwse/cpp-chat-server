extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <mutex>
#include <cctype>
#include <thread>
#include <chrono>
#include <memory>
#include <cstdlib>
#include <cstring>

#include <chatd/net/connection.hpp>
#include <chatd/protocol/protocol.hpp>


enum class PayloadKind {
    StringLiteral,
    Buff,
    Message,
};

class Connection::OutgoingPayload {
public:
    union {
        char *buff;
        const char *cstr;
        std::shared_ptr<msg::Message> *msg;
    };
    size_t len;
    enum PayloadKind kind;
};

Connection::Connection(struct tcp_stream stream,
                       ConnectionPool *pool) :
    m_pool { pool }, m_stream { stream }
{}

Connection::~Connection() {
    delete m_rx_thread;
    delete m_tx_thread;

    for (size_t i = 0; i < m_tx_queue.get_size(); i++) {
        auto payload = (OutgoingPayload *) m_tx_queue[i];

        if (payload->kind == PayloadKind::Buff)
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

    while (!self->m_is_killed.load()) {
        if (tcp_stream_readuntil(&self->m_stream, '\n', &buff, &len)) {
            self->m_is_ready = false;
            break;
        }

        self->handle_payload(buff, len);
    }

    self->handle_payload(NULL, 0);
}

void Connection::tx_thread(std::shared_ptr<Connection> self) {
    while (!self->m_is_ready.load()) {
        if (self->m_is_killed)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    bool initial_message = true;
    bool terminated = false;

    while (!self->m_is_killed.load() && !terminated) {
        if (initial_message) {
            std::lock_guard<std::mutex> guard(self->m_tx_queue_m);
            initial_message = false;
        } else {
            std::unique_lock<std::mutex> lk(self->m_tx_queue_m);
            self->m_tx_queue_cv.wait(lk);
        }

        while (self->m_tx_queue.get_size() && !terminated) {
            auto payload = (OutgoingPayload *)
                self->m_tx_queue.pop_back();

            if ((
                    payload->kind == PayloadKind::Message &&
                    !(*payload->msg)->send(self->m_stream)
                ) || tcp_stream_write(&self->m_stream,
                                 payload->kind == PayloadKind::StringLiteral ?
                                     payload->cstr : payload->buff,
                                 payload->len)) {
                self->m_is_ready = false;
                terminated = true;
            }

            if (payload->kind == PayloadKind::Buff)
                free(payload->buff);

            delete payload;
        }

        if (self->m_is_gracefully_terminated.load())
            self->terminate();

        if (terminated)
            break;
    }
}

void Connection::push_payload(OutgoingPayload *payload) {
    {
        std::lock_guard<std::mutex> guard(m_tx_queue_m);
        m_tx_queue.push_front(payload);
    }
    m_tx_queue_cv.notify_one();
}

void Connection::send(char *buff, size_t len) {
    if (m_is_gracefully_terminated) {
        free(buff);
        return;
    }

    auto payload = new OutgoingPayload;
    payload->buff = buff;
    payload->len = len;
    payload->kind = PayloadKind::Buff;

    push_payload(payload);
}

void Connection::send_strliteral(const char *cstr) {
    auto payload = new OutgoingPayload;
    payload->cstr = cstr;
    payload->len = strlen(cstr);
    payload->kind = PayloadKind::StringLiteral;

    push_payload(payload);
}

void Connection::send_message(std::shared_ptr<msg::Message> msg) {
    auto payload = new OutgoingPayload;
    auto shared_ptr = new std::shared_ptr<msg::Message>;
    *shared_ptr = msg;
    payload->msg = shared_ptr;
    payload->kind = PayloadKind::Message;

    push_payload(payload);
}

void Connection::terminate() {
    if (!m_is_killed.exchange(true, std::memory_order_seq_cst)) {
        tcp_stream_destroy(&m_stream);
        m_tx_queue_cv.notify_one();
    }
}

void Connection::gracefully_terminate() {
    if (!m_is_gracefully_terminated.exchange(true, std::memory_order_seq_cst))
        m_tx_queue_cv.notify_one();
}

void Connection::shutdown() {
    terminate();
    m_rx_thread->join();
    m_tx_thread->join();
}
