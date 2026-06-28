#include <cassert>
#include <iostream>

int main() {
    std::cout << "Running simple integration test...\n";
    // Basic arithmetic check
    assert(1 + 1 == 2);
    std::cout << "Test passed.\n";
    return 0;
}
