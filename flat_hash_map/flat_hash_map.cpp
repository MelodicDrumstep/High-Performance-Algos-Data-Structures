#include <iostream>
#include "flat_hash_map.hpp"

using namespace hpds;

int main() {
    FlatHashMap<int32_t, int32_t> hashmap;
    hashmap.insert(std::pair<int32_t, int32_t>{1, 1});
    std::cout << hashmap.size() << std::endl;

    auto erase_res0 = hashmap.erase(1);
    auto erase_res1 = hashmap.erase(1);

    std::cout << "erase_res0 is " << erase_res0 << ", erase_res1 is " << erase_res1 << std::endl;
    return 0;
}