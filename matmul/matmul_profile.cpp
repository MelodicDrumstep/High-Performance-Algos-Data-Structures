#include <chrono>
#include <random>
#include <iostream>
#include <string>
#include <vector>

#include "matmul.hpp"
#include "test_utils.hpp"
#include "aligned_allocator.hpp"

constexpr int32_t WarmupTimes = 2000;
constexpr int32_t TestTimes = 10000;

template <typename Func>
__attribute__((noinline))
void warmup(Func&& func, const float* a, const float* b, float* c, int32_t n) {
    for (int32_t i = 0; i < WarmupTimes; ++i) {
        func(a, b, c, n);
    }
}

template <typename Func>
__attribute__((noinline))
void actualTest(Func&& func, const float* a, const float* b, float* c, int32_t n) {
    for (int32_t i = 0; i < TestTimes; ++i) {
        func(a, b, c, n);
    }
}

template <typename Func>
void testMatmul(Func && func, std::string_view func_name, int32_t n) {
    // Generate random matrices
    Vector a(n * n);
    Vector b(n * n);
    Vector c(n * n, 0.0f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int32_t i = 0; i < n * n; ++i) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }

    // Warmup phase
    warmup(func, a.data(), b.data(), c.data(), n);
    // Actual test phase
    actualTest(func, a.data(), b.data(), c.data(), n);
    // Print a sample result to prevent optimization
    std::cout << "Sample result: " << c[0] << std::endl;
}

void show_help() {
    std::cout << "Usage: ./matmul_profile <matrix_size> <implementation>\n"
              << "Available implementations:\n"
              << "  baseline\n"
              << "  baseline_restricted\n"
              << "  loop_interchange\n"
              << "  invariant\n"
              << "  register_reuse\n"
              << "  register_reuse2\n"
              << "  4x4\n"
              << "  blocking_4x4\n"
              << "  4x4_vectorization\n"
              << "  blocking_4x4_vectorization\n"
              << "  packing\n"
              << "  packing2\n"
              << "  transpose\n"
              << "  vectorization\n"
              << "  kernel_blocking\n"
              << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        show_help();
        return 1;
    }

    int32_t n;
    try {
        n = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Matrix size must be an integer" << std::endl;
        return 1;
    }

    std::string impl = argv[2];

    if (impl == "baseline") {
        testMatmul(matmul_baseline<false>, "baseline", n);
    } else if (impl == "baseline_restricted") {
        testMatmul(matmul_baseline<true>, "baseline_restricted", n);
    } else if (impl == "loop_interchange") {
        testMatmul(matmul_opt1_loop_interchange<true>, "loop_interchange", n);
    } else if (impl == "invariant") {
        testMatmul(matmul_opt2_invariant<true>, "invariant", n);
    } else if (impl == "register_reuse") {
        testMatmul(matmul_opt3_register_reuse<true>, "register_reuse", n);
    } else if (impl == "register_reuse2") {
        testMatmul(matmul_opt4_register_reuse2<true>, "register_reuse2", n);
    } else if (impl == "4x4") {
        testMatmul(matmul_opt5_4x4<true>, "4x4", n);
    } else if (impl == "blocking_4x4") {
        testMatmul(matmul_opt6_blocking_4x4<true>, "blocking_4x4", n);
    } else if (impl == "4x4_vectorization") {
        testMatmul(matmul_opt7_4x4_vectorization<true>, "4x4_vectorization", n);
    } else if (impl == "blocking_4x4_vectorization") {
        testMatmul(matmul_opt8_blocking_4x4_vectorization<true>, "blocking_4x4_vectorization", n);
    } else if (impl == "packing") {
        testMatmul(matmul_opt9_packing<true>, "packing", n);
    } else if (impl == "packing2") {
        testMatmul(matmul_opt10_packing2<true>, "packing2", n);
    } else if (impl == "transpose") {
        testMatmul(matmul_transpose<true>, "transpose", n);
    } else if (impl == "vectorization") {
        testMatmul(matmul_vectorization, "vectorization", n);
    } else if (impl == "kernel_blocking") {
        testMatmul(matmul_kernel_blocking, "kernel_blocking", n);
    } else {
        std::cerr << "Unknown implementation: " << impl << std::endl;
        show_help();
        return 1;
    }
    return 0;
}