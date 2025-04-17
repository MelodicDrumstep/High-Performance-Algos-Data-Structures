#include <chrono>
#include <random>
#include <iostream>

#include "argmin.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100000;

struct ElementsBlock {
    Vector elements;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testArgmin(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements] = input_param2elements.at(input_param);
    int32_t result;
    for(int32_t i = 0; i < WarmupTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "input_param is " << input_param << "\n";
    // std::cout << "result is " << result << "\n";
    // std::cout << "min value is " << elements[result] << "\n";
    return duration.count() * 1.0 / TestTimes;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    InputParam2ElementBlockMap input_param2elements;
    auto & input_params = test_manager.getInputParams();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(UpperBound / 2, UpperBound);

    for(int32_t input_param : input_params) {
        Vector elements(input_param);
        for(int32_t i = 0; i < input_param; i++) {
            elements[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements)});
    }

    test_manager.launchTest("argmin_baseline", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_baseline, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_baseline_with_hint", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_baseline_with_hint, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_std", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_std, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_vectorize", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_vectorize, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_vectorize2", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_vectorize2, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_vectorize2_with_hint", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_vectorize2_with_hint, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_vectorize2_unroll2", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_vectorize2_unroll2, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_vectorize2_unroll4", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_vectorize2_unroll4, input_param, input_param2elements);
    });
    test_manager.launchTest("argmin_blocking_breakdown", [&input_param2elements](int32_t input_param) {
        return testArgmin(argmin_blocking_breakdown, input_param, input_param2elements);
    });
    test_manager.dump();
}