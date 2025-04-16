#include <chrono>
#include <random>
#include <iostream>

#include "division.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100000;

template <typename Func>
double testDivision(Func && func, int32_t input_param) {
    std::vector<uint32_t> elements_a(TestTimes);
    std::vector<uint32_t> elements_b(TestTimes);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(input_param / 2 + 1, input_param);
    for (int32_t i = 0; i < TestTimes; i++) {
        elements_a[i] = dist(gen);
        elements_b[i] = dist(gen);
    }    

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

    test_manager.launchTest("gcd_baseline_recursion", [](int32_t input_param) {
        return testDivision(gcd_baseline_recursion, input_param);
    });
    test_manager.launchTest("gcd_baseline_loop", [](int32_t input_param) {
        return testDivision(gcd_baseline_loop, input_param);
    });
    test_manager.launchTest("gcd_binary", [](int32_t input_param) {
        return testDivision(gcd_binary, input_param);
    });
    test_manager.launchTest("gcd_binary_opt1", [](int32_t input_param) {
        return testDivision(gcd_binary_opt1, input_param);
    });
    test_manager.launchTest("gcd_binary_opt2", [](int32_t input_param) {
        return testDivision(gcd_binary_opt2, input_param);
    });
    test_manager.dump();
}

int main() {

    testDivision(division_baseline, "division_baseline", elements_a, elements_b);
    testDivision(division_baseline2, "division_baseline2", elements_a, elements_b);
    testDivision(division_Barrett_reduction, "division_Barrett_reduction", elements_a, elements_b);
    testDivision(division_Lemire_reduction, "division_Lemire_reduction", elements_a, elements_b);
    testDivision(division_Lemire_reduction2, "division_Lemire_reduction2", elements_a, elements_b);
    testDivision(division_libdivide_branchfull, "division_libdivide_branchfull", elements_a, elements_b);
    testDivision(division_libdivide_branchfree, "division_libdivide_branchfree", elements_a, elements_b);

    testDivisionPrecompute(division_baseline, "division_baseline", elements_a, elements_b[0]);
    testDivisionPrecompute(division_baseline2, "division_baseline2", elements_a, elements_b[0]);
    testDivisionPrecompute(division_Barrett_reduction_precompute, "division_Barrett_reduction", elements_a, elements_b[0]);
    testDivisionPrecompute(division_Lemire_reduction_precompute, "division_Lemire_reduction", elements_a, elements_b[0]);
    testDivisionPrecompute(division_Lemire_reduction_precompute2, "division_Lemire_reduction2", elements_a, elements_b[0]);
    testDivisionPrecompute(division_libdivide_branchfull_precompute, "division_libdivide_branchfull", elements_a, elements_b[0]);
    testDivisionPrecompute(division_libdivide_branchfree_precompute, "division_libdivide_branchfree", elements_a, elements_b[0]);
}