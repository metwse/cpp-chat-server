#include <chatd/protocol/protocol.hpp>


void msg::DirectMessage::send(Connection &) {}

void msg::GroupMessage::send(Connection &) {}

void msg::GlobalMessage::send(Connection &) {}
