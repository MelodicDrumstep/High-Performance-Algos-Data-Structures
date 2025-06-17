#include <iostream>
#include <unordered_map>
#include <list>
#include "stable_vector.hpp"
#include "test_utils.hpp"

using namespace hpds;

constexpr static int32_t TestTimes = 1'000'000;
constexpr static int32_t HeapRandomizationTimes = 1000;
constexpr static std::size_t ChunkSize = 4 * 4096;

void WSS_StableVector() {
    StableVector<int32_t, ChunkSize> v;
    std::list<int32_t> l;
    // the list is used for heap randomization
    for(int32_t i = 0; i < TestTimes; i++) {
        for(int32_t j = 0; j < HeapRandomizationTimes; j++) {
            l.push_back(j);
            doNotOptimizeAway(l.back());
        }
        v.push_back(i);
    }
    int32_t sum = 0;
    for(int32_t i = 0; i < TestTimes; i++) {
        sum += v[i];
    }
    doNotOptimizeAway(sum);
    std::cout << "WSS_StableVector done, sum: " << sum << std::endl;
}

void WSS_UnorderedMap() {
    std::unordered_map<int32_t, int32_t> m;
    std::list<int32_t> l;
    // the list is used for heap randomization
    for(int32_t i = 0; i < TestTimes; i++) {
        for(int32_t j = 0; j < HeapRandomizationTimes; j++) {
            l.push_back(j);
            doNotOptimizeAway(l.back());
        }
        m[i] = i;
    }
    int32_t sum = 0;
    for(int32_t i = 0; i < TestTimes; i++) {
        sum += m[i];
    }
    doNotOptimizeAway(sum);
    std::cout << "WSS_UnorderedMap done, sum: " << sum << std::endl;
}

int32_t main(int32_t argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " SV / UM" << std::endl;
        return 1;
    }
    if(std::string(argv[1]) == "SV") {
        WSS_StableVector();
    } else if(std::string(argv[1]) == "UM") {
        WSS_UnorderedMap();
    } else {
        std::cerr << "Usage: " << argv[0] << " SV / UM" << std::endl;
        return 1;
    }
    return 0;
}