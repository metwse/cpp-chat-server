#include <chatd/protocol/protocol.hpp>

#include <cstdlib>


User::~User() {
    free(username);
    free(password);
}
