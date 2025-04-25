
# Matrix Multiplication Algorithm (Row Major)

## Optimizations



## Performance Test

### Test Environment

+ CPU: Intel(R) Xeon(R) Platinum 8358 CPU @ 2.60GHz

+ OS: Rocky Linux 8.9 (Green Obsidian)

+ Compiler Version: G++ 10.5.0

+ Compilation Flags: -O3

+ Execution Command: `taskset -c 0 ./matmul ../config.json`

### Test Results

<img src="../images/matmul_result.png" alt="Matmul" width="850" height="auto">

Test result data is located at [matmulresult.json](./matmul_result.json).

### Profiling Results

#### Cache miss rate

<img src="../images/matmul_profile_cache.png" alt="Matmul Profile" width="850" height="auto">

#### L1 Cache miss rate

<img src="../images/matmul_profile_L1_cache.png" alt="Matmul Profile" width="850" height="auto">

#### Branch miss rate

<img src="../images/matmul_profile_branch.png" alt="Matmul Profile" width="850" height="auto">

## References

1. [en.algorithmica.org:matrix multiplication](https://en.algorithmica.org/hpc/algorithms/matmul/)

2. [how-to-optimize-gemm](https://github.com/flame/how-to-optimize-gemm/wiki)

3. [Optimizing-DGEMM-on-Intel-CPUs-with-AVX512F](https://github.com/yzhaiustc/Optimizing-DGEMM-on-Intel-CPUs-with-AVX512F)

4. [matmul-cache-blocking-demo](https://jukkasuomela.fi/cache-blocking-demo/)