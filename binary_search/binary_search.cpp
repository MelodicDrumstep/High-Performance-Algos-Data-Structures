#include <chrono>
#include <random>
#include <iostream>

#include "binary_search.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

struct ElementsBlock {
    std::vector<int32_t> elements;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

// take the func_name as a parameter for debugging and correctness testing
template <typename Func>
double testBinarySearch(Func && func, std::string_view func_name, int32_t input_param, 
        const InputParam2ElementBlockMap & input_param2elements) {
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
    // std::cout << "Function '" << func_name << "' took " << duration.count() << " µs to complete." << std::endl;
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

    #define launchFuncTest(system_func_name, output_func_name) \
        test_manager.launchTest(#output_func_name, [&input_param2elements](int32_t input_param) {   \
            return testBinarySearch(system_func_name, #output_func_name, input_param, input_param2elements);    \
        });

    // launchFuncTest(sum_baseline<UpperBound>, sum_baseline);
    // launchFuncTest(sum_predication<UpperBound>, sum_predication);
    // launchFuncTest(sum_predication_tenary<UpperBound>, sum_predication_tenary);
    // launchFuncTest(sum_predication_masking<UpperBound>, sum_predication_masking);

    test_manager.dump();
}