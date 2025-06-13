#include <cstdint>
#include <iostream>
#include <limits>

struct ElementTA {
    uint32_t a : 1;
    uint32_t b : 15;
};

struct ElementTB {
    uint16_t a : 1;
    uint16_t b : 15;
};

struct ElementTC {
    uint16_t a : 1;
    uint32_t b : 15;
};

struct ElementTD {
    uint32_t a : 1;
    uint16_t b : 15;
};

struct ElementTE {
    uint16_t a : 1;
    uint16_t b : 31;
};

struct ElementTF {
    uint32_t a : 1;
    uint16_t b : 31;
};

struct ElementTG {
    uint16_t a : 1;
    uint32_t b : 31;
};

int main() {
    std::cout << "sizeof(ElementTA) is " << sizeof(ElementTA) << "\n";
    std::cout << "sizeof(ElementTB) is " << sizeof(ElementTB) << "\n";
    std::cout << "sizeof(ElementTC) is " << sizeof(ElementTC) << "\n";
    std::cout << "sizeof(ElementTD) is " << sizeof(ElementTD) << "\n";
    std::cout << "sizeof(ElementTE) is " << sizeof(ElementTE) << "\n";
    std::cout << "sizeof(ElementTF) is " << sizeof(ElementTF) << "\n";
    std::cout << "sizeof(ElementTG) is " << sizeof(ElementTG) << "\n";

    uint32_t a = std::numeric_limits<uint32_t>::max() - 1;
    uint16_t b = a;
    std::cout << "a : " << a << std::endl;
    std::cout << "b : " << b << std::endl;
    std::cout << "a == b : " << (a == b) << std::endl;
    std::cout << "static_cast<uint16_t>(a) == b : " << (static_cast<uint16_t>(a) == b) << std::endl;
    return 0;
}