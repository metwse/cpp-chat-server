extern "C" {
#include <chatd/c_main.h>
}

#include <iostream>


int main() {
    std::cout << "Hello, world!" << std::endl;

    return c_main();
}
