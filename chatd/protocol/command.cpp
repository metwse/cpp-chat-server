#include <mutex>

#include <chatd/protocol/protocol.hpp>


void cmd::Subscribe::operator()(Connection *c, ConnectionPool *pool) {
    auto name = (char *) args[0];
    auto password = (char *) args[1];
}

void cmd::Unsubscribe::operator()(Connection *, ConnectionPool *) {}

void cmd::Delete::operator()(Connection *, ConnectionPool *) {}

void cmd::Logout::operator()(Connection *, ConnectionPool *) {}

void cmd::ListUsers::operator()(Connection *, ConnectionPool *) {}
