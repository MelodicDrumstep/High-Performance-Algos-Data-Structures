#include <chrono>
#include <random>
#include <iostream>

#include "factorization.hpp"
#include "test_utils.hpp"

struct ElementsBlock {
    std::vector<uint64_t> elements;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

// Sieve of Eratosthenes to generate prime numbers up to a given limit
std::vector<int32_t> generatePrimes(int32_t limit) {
    std::vector<bool> sieve(limit + 1, true);
    sieve[0] = sieve[1] = false;  // 0 and 1 are not prime

    for (int32_t i = 2; i * i <= limit; ++i) {
        if (sieve[i]) {
            for (int32_t j = i * i; j <= limit; j += i) {
                sieve[j] = false;
            }
        }
    }

    std::vector<int32_t> primes;
    for (int32_t i = 2; i <= limit; ++i) {
        if (sieve[i]) {
            primes.push_back(i);
        }
    }

    return primes;
}

// Function to generate a product of two prime numbers within a given range,
// and ensures one of the primes is not 2, 3, or 5
int32_t generateProductOfTwoPrimes(int32_t lower_bound, int32_t upper_bound) {
    // Generate a list of primes up to the upper bound limit
    int32_t limit = upper_bound;  // Set prime number range limit
    std::vector<int32_t> primes = generatePrimes(limit);

    // Filter out 2, 3, and 5 from the primes list
    std::vector<int32_t> primes_without_235;
    for (int32_t prime : primes) {
        if ((upper_bound < 50) || (prime != 2 && prime != 3 && prime != 5)) {
            primes_without_235.push_back(prime);
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, primes.size() - 1);

    int32_t product = 0;
    while (true) {
        int32_t prime1 = primes_without_235[dis(gen) % primes_without_235.size()];
        int32_t prime2 = primes_without_235[dis(gen) % primes_without_235.size()];

        product = prime1 * prime2;

        // If the product is within the specified range, return it
        if (product >= lower_bound && product <= upper_bound) {
            return product;
        }
    }
}

template <typename Func>
double testFactorization(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements] = input_param2elements.at(input_param);
    int32_t result;
    for(int32_t i = 0; i < WarmupTimes; i++) {
        result = func(elements[i]);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements[i]);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
    return duration.count() * 1.0 / TestTimes;
}

int32_t main(int32_t argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    InputParam2ElementBlockMap input_param2elements;
    auto & input_params = test_manager.getInputParams();

    std::random_device rd;
    std::mt19937 gen(rd());

    for(int32_t input_param : input_params) {
        std::vector<uint64_t> elements(TestTimes);
        for(int32_t i = 0; i < TestTimes; i++) {
            // use the product of two prime numbers to test the functions
            elements[i] = generateProductOfTwoPrimes(input_param / 2 + 1, input_param);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements)});
    }

    test_manager.launchTest("find_factor_baseline", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_baseline, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_brute_pruning", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_brute_pruning, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_lookup_table", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_lookup_table, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_wheel", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_wheel, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_wheel2", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_wheel2, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_prime_table", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_prime_table, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_prime_table_lemire", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_prime_table_lemire, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_Pollard_Pho", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_Pollard_Pho, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_Pollard_Brent", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_Pollard_Brent, input_param, input_param2elements);
    });
    test_manager.launchTest("find_factor_Pollard_Brent_batch_opt", [&input_param2elements](int32_t input_param) {
        return testFactorization(find_factor_Pollard_Brent_batch_opt, input_param, input_param2elements);
    });
    test_manager.dump();
}