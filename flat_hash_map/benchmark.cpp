#include <iostream>
#include <chrono>
#include <random>
#include "flat_hash_map_v0.hpp"
#include "flat_hash_map_v1.hpp"

#define ChosenFlatHashMap FlatHashMapV1a

// TODO: Use TestManager to rewrite it.

constexpr int32_t WarmupTimes = 50;
constexpr int32_t TestTimes = 50;

using namespace hpds;
using Clock = std::chrono::high_resolution_clock;
using Duration = std::chrono::duration<double, std::micro>; // microseconds

// Generates random integers in the range [min, max]
std::vector<int> generate_random_ints(std::size_t count, int min = 0, int max = 1000000) {
    std::mt19937 rng(42); // fixed seed for repeatability
    std::uniform_int_distribution<int> dist(min, max);
    std::vector<int> data(count);
    for (auto &val : data) val = dist(rng);
    return data;
}

// Measures execution time of a function
template <typename Func>
double measure_time_us(Func func) {
    for (int32_t i = 0; i < WarmupTimes; ++i) {
        func();
    }
    auto start = Clock::now();
    for (int32_t i = 0; i < TestTimes; ++i) {
        func();
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<Duration>(end - start).count();
}

void test_sequential_insert() {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 100000;

    double time = measure_time_us([&]() {
        for (int i = 0; i < N; ++i) {
            map.insert({i, i});
        }
    });

    std::cout << "[Sequential Insert] Time: " << time << " us\n";
}

void test_random_insert() {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 100000;
    auto keys = generate_random_ints(N);

    double time = measure_time_us([&]() {
        for (int i = 0; i < N; ++i) {
            map.insert({keys[i], i});
        }
    });

    std::cout << "[Random Insert] Time: " << time << " us\n";
}

void test_high_collision() {
    struct HighCollisionHash {
        std::size_t operator()(int key) const {
            return 42;
        }
    };

    ChosenFlatHashMap<int, int, 256, HighCollisionHash> map;
    constexpr int N = 1000;

    double time = measure_time_us([&]() {
        for (int i = 0; i < N; ++i) {
            map.insert({i, i});
        }
    });

    std::cout << "[High Collision Insert] Time: " << time << " us\n";
}

void test_erase_half() {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 100000;
    for (int i = 0; i < N; ++i) {
        map.insert({i, i});
    }

    double time = measure_time_us([&]() {
        for (int i = 0; i < N; i += 2) {
            map.erase(i);
        }
    });

    std::cout << "[Erase Half Elements] Time: " << time << " us\n";
}

void test_erase_and_reinsert() {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 100000;
    for (int i = 0; i < N; ++i) {
        map.insert({i, i});
    }
    for (int i = 0; i < N; ++i) {
        map.erase(i);
    }

    double time = measure_time_us([&]() {
        for (int i = 0; i < N; ++i) {
            map.insert({i, i});
        }
    });

    std::cout << "[Reinsert After Full Erase] Time: " << time << " us\n";
}

void test_mixed_find() {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 10000;
    for (int i = 0; i < N; ++i) {
        map.insert({i, i});
    }
    auto queries = generate_random_ints(N, 0, 20000);  // 50% hit, 50% miss

    double time = measure_time_us([&]() {
        volatile int sum = 0;
        for (int q : queries) {
            auto it = map.find(q);
            if (it != map.end()) {
                sum += it->second;
            }
        }
    });

    std::cout << "[Mixed Find (hit+miss)] Time: " << time << " us\n";
}

int main() {
    test_sequential_insert();
    test_random_insert();
    test_high_collision();
    test_erase_half();
    test_erase_and_reinsert();
    test_mixed_find();
    return 0;
}
