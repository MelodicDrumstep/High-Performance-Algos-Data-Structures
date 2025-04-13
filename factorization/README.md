# Integer Factorization algorithm

## Performance Tests

### Testing Environment

+ CPU: Intel(R) Xeon(R) Platinum 8358 CPU @ 2.60GHz

+ OS: Rocky Linux 8.9 (Green Obsidian)

+ Compiler Version: G++ 10.5.0

+ Compilation Flags: -O3

+ Execution Command: taskset -c 0 ./gcd.

### Testing Result

```
Function 'find_factor_baseline' took 36 µs to complete.
Function 'find_factor_brute_pruning' took 12 µs to complete.
Function 'find_factor_lookup_table' took 1 µs to complete.
Function 'find_factor_wheel' took 9 µs to complete.
Function 'find_factor_wheel2' took 9 µs to complete.
Function 'find_factor_prime_table' took 224 µs to complete.
Function 'find_factor_prime_table_lemire' took 52 µs to complete.
Function 'find_factor_Pollard_Pho' took 4455 µs to complete.
Function 'find_factor_Pollard_Brent' took 79450 µs to complete.
Function 'find_factor_Pollard_Brent_batch_opt' took 61089 µs to complete.
```

## References

1. [en.algorithmica.org:factorization](https://en.algorithmica.org/hpc/algorithms/factorization/)