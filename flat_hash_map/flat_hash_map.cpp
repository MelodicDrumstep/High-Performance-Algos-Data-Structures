#include <iostream>
#include "flat_hash_map.hpp"

using namespace hpds;

int main() {
    FlatHashMap<int32_t, int32_t> hashmap;
    hashmap.insert(std::pair<int32_t, int32_t>{1, 1});
    std::cout << hashmap.size() << std::endl;
    return 0;
}