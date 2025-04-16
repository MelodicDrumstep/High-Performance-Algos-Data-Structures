#include <chrono>
#include <random>
#include <iostream>

#include "division.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100000;

struct ElementsBlock {
    std::vector<uint32_t> elements_a;
    std::vector<uint32_t> elements_b;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testDivision(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements_a, elements_b] = input_param2elements.at(input_param);
    DivResult result;
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
    // std::cout << "result is {" << result.quotient << ", " << result.remainder << "}" << std::endl;
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
        std::uniform_int_distribution<uint32_t> dist(input_param / 2 + 1, input_param);
        std::vector<uint32_t> elements_a(TestTimes);
        std::vector<uint32_t> elements_b(TestTimes);
        for(int32_t i = 0; i < TestTimes; i++) {
            elements_a[i] = dist(gen);
            elements_b[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements_a), std::move(elements_b)});
    }

    test_manager.launchTest("division_baseline", [&input_param2elements](int32_t input_param) {
        return testDivision(division_baseline, input_param, input_param2elements);
    });
    test_manager.launchTest("division_baseline2", [&input_param2elements](int32_t input_param) {
        return testDivision(division_baseline2, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Barrett_reduction", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Barrett_reduction, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Lemire_reduction", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Lemire_reduction, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Lemire_reduction2", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Lemire_reduction2, input_param, input_param2elements);
    });
    test_manager.launchTest("division_libdivide_branchfull", [&input_param2elements](int32_t input_param) {
        return testDivision(division_libdivide_branchfull, input_param, input_param2elements);
    });
    test_manager.launchTest("division_libdivide_branchfree", [&input_param2elements](int32_t input_param) {
        return testDivision(division_libdivide_branchfree, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Barrett_reduction_precompute", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Barrett_reduction_precompute, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Lemire_reduction_precompute", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Lemire_reduction_precompute, input_param, input_param2elements);
    });
    test_manager.launchTest("division_Lemire_reduction_precompute2", [&input_param2elements](int32_t input_param) {
        return testDivision(division_Lemire_reduction_precompute2, input_param, input_param2elements);
    });
    test_manager.launchTest("division_libdivide_branchfull_precompute", [&input_param2elements](int32_t input_param) {
        return testDivision(division_libdivide_branchfull_precompute, input_param, input_param2elements);
    });
    test_manager.launchTest("division_libdivide_branchfree_precompute", [&input_param2elements](int32_t input_param) {
        return testDivision(division_libdivide_branchfree_precompute, input_param, input_param2elements);
    });
    test_manager.dump();
}