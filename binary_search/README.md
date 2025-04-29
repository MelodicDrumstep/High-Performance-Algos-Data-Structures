# Binary Search Optimizations

## Testing

### Test Environment

+ CPU: Intel(R) Xeon(R) Platinum 8358 CPU @ 2.60GHz

+ OS: Rocky Linux 8.9 (Green Obsidian)

+ Compiler Version: G++ 10.5.0

+ Compilation Flags: -O3 -g

+ Execution Command: `taskset -c 0 ./binary_search ../config.json`

### Test Results

<img src="../images/binary_search_result.png" alt="BS Result" width="850" height="auto">

Test result data is located at [binary_search_result.json](./binary_search_result.json).

## References

1. [Optimizing Binary Search - Sergey Slotin - CppCon 2022](https://www.youtube.com/watch?v=1RIPMQQRBWk)

2. [en.algorithmica.org:binary_search](https://en.algorithmica.org/hpc/data-structures/binary-search/)

3. [qayamd/eytzinger](https://github.com/qayamd/eytzinger)