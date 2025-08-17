#include <chatd/protocol/protocol.hpp>

#include <cstdlib>


User::~User() {
    free(name);
    free(password);
}

Channel::~Channel() {
    delete[] name;
    delete[] password;
}
