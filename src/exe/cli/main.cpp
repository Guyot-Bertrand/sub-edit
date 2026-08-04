#include <subedit/core/version.hpp>

#include <iostream>

int main() {
    std::cout << "subedit " << subedit::core::versionString() << '\n';
    return 0;
}
