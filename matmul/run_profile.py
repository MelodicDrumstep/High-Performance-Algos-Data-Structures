#!/usr/bin/env python3

import subprocess
import sys
import os
import json
import re

def run_perf(matrix_size, implementation, perf_events):
    """Run perf stat for a specific implementation and return the results."""
    cmd = [
        "perf", "stat", "-e", perf_events,
        "./matmul_profile", str(matrix_size), implementation
    ]
    
    try:
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        return result.stderr  # perf output goes to stderr
    except subprocess.CalledProcessError as e:
        print(f"Error running perf for {implementation}: {e}")
        return None

def parse_perf_output(output):
    """Parse perf stat output to extract cache references, misses, and miss rate."""
    cache_refs = None
    cache_misses = None
    miss_rate = None
    
    for line in output.split('\n'):
        if 'cache-references' in line:
            cache_refs = int(re.search(r'(\d+,?\d*)', line).group(1).replace(',', ''))
        elif 'cache-misses' in line and '%' in line:
            # Extract the percentage from lines like "7,885      cache-misses:u            #   39.526 % of all cache refs"
            match = re.search(r'#\s*(\d+\.\d+)\s*%', line)
            if match:
                miss_rate = float(match.group(1))
            # Also get the raw cache misses count
            cache_misses = int(re.search(r'(\d+,?\d*)', line).group(1).replace(',', ''))
    
    if cache_refs is None or cache_misses is None or miss_rate is None:
        return None
    
    return miss_rate

def main():
    if len(sys.argv) != 2:
        print("Usage: python run_profile.py <config_file>")
        print("Example: python run_profile.py config.json")
        sys.exit(1)

    config_file = sys.argv[1]
    try:
        with open(config_file, 'r') as f:
            config = json.load(f)
    except FileNotFoundError:
        print(f"Error: Config file {config_file} not found")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON in config file {config_file}")
        sys.exit(1)

    if 'input_params' not in config:
        print("Error: Config file must contain 'input_params' field")
        sys.exit(1)

    input_params = config['input_params']
    if not isinstance(input_params, list):
        print("Error: 'input_params' must be a list")
        sys.exit(1)

    # Get perf events from config or use default
    perf_events = config.get('perf_events', 'cache-references,cache-misses')

    implementations = [
        "baseline",
        "baseline_restricted",
        "loop_interchange",
        "invariant",
        "register_reuse",
        "register_reuse2",
        "4x4",
        "blocking_4x4",
        "4x4_vectorization",
        "blocking_4x4_vectorization",
        "packing",
        "packing2",
        "transpose",
        "vectorization",
        "kernel_blocking"
    ]

    results = {}
    print(f"Starting cache performance measurements...")
    print(f"Testing matrix sizes: {input_params}")

    for impl in implementations:
        print(f"Testing {impl}...")
        impl_results = []
        for size in input_params:
            print(f"  Size {size}x{size}...")
            perf_output = run_perf(size, impl, perf_events)
            if perf_output:
                miss_rate = parse_perf_output(perf_output)
                if miss_rate is not None:
                    impl_results.append(miss_rate)
                    print(f"    ✓ {miss_rate:.3f}% miss rate")
                else:
                    print(f"    ✗ Failed to parse perf output")
            else:
                print(f"    ✗ Failed to run perf")
        results[impl] = impl_results

    # Create output JSON
    output = {
        "cache miss rate": {
            "input_params": input_params,
            "result": results
        },
        "input_param_meaning": "value of N (we use NxN matrices)",
        "test_name": "Matrix Multiplication"
    }

    # Save results
    output_file = "profile_results.json"
    with open(output_file, 'w') as f:
        json.dump(output, f, indent=4)

    print(f"\nResults saved in: {output_file}")
    print("Done!")

if __name__ == "__main__":
    main()