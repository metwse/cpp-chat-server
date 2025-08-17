extern "C" {
#include <chatd/net/tcp/stream.h>
}

#include <cstring>

#include <chatd/protocol/protocol.hpp>


#define UNWRAP(e) \
    if (e) \
        return false;

bool msg::DirectMessage::send(struct tcp_stream &s) {
    UNWRAP(tcp_stream_write(&s, "<DM> @", 6));
    UNWRAP(tcp_stream_write(&s, user->name, strlen(user->name)));
    UNWRAP(tcp_stream_write(&s, ": ", 2));
    UNWRAP(tcp_stream_write(&s, content, strlen(content)));
    UNWRAP(tcp_stream_write(&s, "\n", 1));
    return true;
}

bool msg::GroupMessage::send(struct tcp_stream &s) {
    UNWRAP(tcp_stream_write(&s, "#", 1));
    UNWRAP(tcp_stream_write(&s, to, strlen(to)));
    UNWRAP(tcp_stream_write(&s, " @", 2));
    UNWRAP(tcp_stream_write(&s, user->name, strlen(user->name)));
    UNWRAP(tcp_stream_write(&s, ": ", 2));
    UNWRAP(tcp_stream_write(&s, content, strlen(content)));
    UNWRAP(tcp_stream_write(&s, "\n", 1));
    return true;
}

bool msg::GlobalMessage::send(struct tcp_stream &s) {
    UNWRAP(tcp_stream_write(&s, "<Global> @", 10));
    UNWRAP(tcp_stream_write(&s, user->name, strlen(user->name)));
    UNWRAP(tcp_stream_write(&s, ": ", 2));
    UNWRAP(tcp_stream_write(&s, content, strlen(content)));
    UNWRAP(tcp_stream_write(&s, "\n", 1));
    return true;
}
