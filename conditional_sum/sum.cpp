#include <chrono>
#include <random>
#include <iostream>

#include "sum.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

struct ElementsBlock {
    std::vector<int32_t> elements;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testSum(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
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
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    return duration.count() * 1.0 / TestTimes;
    // std::cout << "result is " << result << std::endl;
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
    std::uniform_int_distribution<int32_t> dist(0, UpperBound * 2 - 1);

    for(int32_t input_param : input_params) {
        std::vector<int32_t> elements(input_param);
        for(int32_t i = 0; i < input_param; i++) {
            elements[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements)});
    }

    test_manager.launchTest("sum_baseline", [&input_param2elements](int32_t input_param) {
        return testSum(sum_baseline<UpperBound>, input_param, input_param2elements);
    });
    test_manager.launchTest("sum_predication", [&input_param2elements](int32_t input_param) {
        return testSum(sum_predication<UpperBound>, input_param, input_param2elements);
    });
    test_manager.launchTest("sum_predication_tenary", [&input_param2elements](int32_t input_param) {
        return testSum(sum_predication_tenary<UpperBound>, input_param, input_param2elements);
    });
    test_manager.launchTest("sum_predication_masking", [&input_param2elements](int32_t input_param) {
        return testSum(sum_predication_masking<UpperBound>, input_param, input_param2elements);
    });
    test_manager.dump();
}