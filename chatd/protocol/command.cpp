#include <chatd/protocol/protocol.hpp>


void cmd::Subscribe::operator()(Connection *, ConnectionPool *) {}

void cmd::Unsubscribe::operator()(Connection *, ConnectionPool *) {}

void cmd::Delete::operator()(Connection *, ConnectionPool *) {}

void cmd::Logout::operator()(Connection *, ConnectionPool *) {}

void cmd::ListUsers::operator()(Connection *, ConnectionPool *) {}
