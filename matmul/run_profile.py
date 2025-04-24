#!/usr/bin/env python3

import subprocess
import sys
import os
from datetime import datetime
import json
import re

def run_perf(config_file, implementation):
    """Run perf stat for a specific implementation and return the results."""
    cmd = [
        "perf", "stat", "-e", "cache-references,cache-misses",
        "./matmul_profile", config_file, implementation
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.stderr  # perf output goes to stderr
    except subprocess.CalledProcessError as e:
        print(f"Error running perf for {implementation}: {e}")
        return None

def parse_perf_output(output):
    """Parse perf stat output to extract cache references and misses."""
    cache_refs = None
    cache_misses = None
    
    for line in output.split('\n'):
        if 'cache-references' in line:
            cache_refs = int(re.search(r'(\d+,?\d*)', line).group(1).replace(',', ''))
        elif 'cache-misses' in line:
            cache_misses = int(re.search(r'(\d+,?\d*)', line).group(1).replace(',', ''))
    
    if cache_refs is None or cache_misses is None:
        return None
    
    miss_rate = (cache_misses / cache_refs) * 100 if cache_refs > 0 else 0
    return {
        "cache_references": cache_refs,
        "cache_misses": cache_misses,
        "miss_rate": miss_rate
    }

def main():
    if len(sys.argv) != 2:
        print("Usage: python run_profile.py <config_file>")
        sys.exit(1)

    config_file = sys.argv[1]
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = f"cache_results_{timestamp}"
    os.makedirs(output_dir, exist_ok=True)

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
    print("Starting cache performance measurements...")
    print(f"Results will be saved in: {output_dir}")

    for impl in implementations:
        print(f"Testing {impl}...")
        perf_output = run_perf(config_file, impl)
        
        if perf_output:
            cache_stats = parse_perf_output(perf_output)
            if cache_stats:
                results[impl] = cache_stats
                print(f"  ✓ {impl}: {cache_stats['miss_rate']:.2f}% miss rate")
            else:
                print(f"  ✗ {impl}: Failed to parse perf output")
        else:
            print(f"  ✗ {impl}: Failed to run perf")

    # Save detailed results
    detailed_file = os.path.join(output_dir, "detailed_results.json")
    with open(detailed_file, 'w') as f:
        json.dump(results, f, indent=2)

    # Create summary
    summary_file = os.path.join(output_dir, "summary.txt")
    with open(summary_file, 'w') as f:
        f.write("Cache Performance Summary\n")
        f.write(f"Generated on {datetime.now()}\n")
        f.write("=" * 50 + "\n\n")
        
        for impl, stats in sorted(results.items(), key=lambda x: x[1]['miss_rate']):
            f.write(f"{impl}:\n")
            f.write(f"  Cache References: {stats['cache_references']:,}\n")
            f.write(f"  Cache Misses: {stats['cache_misses']:,}\n")
            f.write(f"  Miss Rate: {stats['miss_rate']:.2f}%\n\n")

    print(f"\nDetailed results saved in: {detailed_file}")
    print(f"Summary saved in: {summary_file}")
    print("Done!")

if __name__ == "__main__":
    main() 