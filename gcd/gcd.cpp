#include <chrono>
#include <random>
#include <iostream>

#include "gcd.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

struct ElementsBlock {
    std::vector<int32_t> elements_a;
    std::vector<int32_t> elements_b;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testGcd(Func && func, std::string_view func_name, int32_t input_param, 
        const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements_a, elements_b] = input_param2elements.at(input_param);
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
    // std::cout << "Input_param '" << input_param << "' took " << duration.count() << " ns to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
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

    for(int32_t input_param : input_params) {
        std::uniform_int_distribution<int32_t> dist(input_param / 2 + 1, input_param);
        std::vector<int32_t> elements_a(TestTimes);
        std::vector<int32_t> elements_b(TestTimes);
        for(int32_t i = 0; i < TestTimes; i++) {
            elements_a[i] = dist(gen);
            elements_b[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements_a), std::move(elements_b)});
    }

    #define launchFuncTest(func_name) \
        test_manager.launchTest(#func_name, [&input_param2elements](int32_t input_param) {   \
            return testGcd(func_name, #func_name, input_param, input_param2elements);    \
        });

    launchFuncTest(gcd_baseline_recursion);
    launchFuncTest(gcd_baseline_loop);
    launchFuncTest(gcd_binary);
    launchFuncTest(gcd_binary_opt1);
    launchFuncTest(gcd_binary_opt2);

    test_manager.dump();
}