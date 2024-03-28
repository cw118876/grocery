#include <iostream>
#include "any/any.hpp"

void processAny(const mystd::any& value) {
    if (value.has_value()) {
        std::cout << "Type of stored value: " << value.type().name() << std::endl;

        if (value.type() == typeid(int)) {
            std::cout << "Value: " << mystd::any_cast<int>(value) << std::endl;
        } else if (value.type() == typeid(double)) {
            std::cout << "Value: " << mystd::any_cast<double>(value) << std::endl;
        } else if (value.type() == typeid(std::string)) {
            std::cout << "Value: " << mystd::any_cast<std::string>(value) << std::endl;
        } else {
            std::cout << "Unsupported type!" << std::endl;
        }
    } else {
        std::cout << "No value stored in std::any." << std::endl;
    }
}

int main() {
    processAny(42);
    processAny(3.14);
    processAny(std::string("Hello, std::any!"));
    processAny(4.5f); // Unsupported type

    return 0;
}