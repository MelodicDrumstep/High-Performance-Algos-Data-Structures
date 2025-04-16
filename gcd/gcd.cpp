#include <chrono>
#include <random>
#include <iostream>

#include "gcd.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

template <typename Func>
double testGcd(Func && func, int32_t input_param) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(input_param / 2 + 1, input_param);

    std::vector<int32_t> elements_a(TestTimes);
    std::vector<int32_t> elements_b(TestTimes);
    for(int32_t i = 0; i < TestTimes; i++) {
        elements_a[i] = dist(gen);
        elements_b[i] = dist(gen);
    }

    int32_t result;
    for(int32_t i = 0; i < WarmupTimes; i++) {
        result = func(elements_a[i], elements_b[i]);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements_a[i], elements_b[i]);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    return duration.count() * 1.0 / TestTimes;
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    struct ElementsBlock {
        std::vector<int32_t> elements_a;
        std::vector<int32_t> elements_b;
    };
    std::unordered_map<int32_t, ElementsBlock> input_param2elements;

    test_manager.launchTest("gcd_baseline_recursion", [](int32_t input_param) {
        return testGcd(gcd_baseline_recursion, input_param);
    });
    test_manager.launchTest("gcd_baseline_loop", [](int32_t input_param) {
        return testGcd(gcd_baseline_loop, input_param);
    });
    test_manager.launchTest("gcd_binary", [](int32_t input_param) {
        return testGcd(gcd_binary, input_param);
    });
    test_manager.launchTest("gcd_binary_opt1", [](int32_t input_param) {
        return testGcd(gcd_binary_opt1, input_param);
    });
    test_manager.launchTest("gcd_binary_opt2", [](int32_t input_param) {
        return testGcd(gcd_binary_opt2, input_param);
    });
    test_manager.dump();
}