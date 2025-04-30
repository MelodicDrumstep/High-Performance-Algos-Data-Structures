#!/usr/bin/env python3

import subprocess
import sys
import os
import json
import re

def run_perf(input_size, implementation, perf_events):
    """Run perf stat for a specific implementation and return the results."""
    cmd = [
        "perf", "stat", "-e", perf_events,
        "./binary_search_profile", str(input_size), implementation
    ]
    
    try:
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        return result.stderr  # perf output goes to stderr
    except subprocess.CalledProcessError as e:
        print(f"Error running perf for {implementation}: {e}")
        return None

def parse_perf_output(output, perf_events):
    """
    Parse perf stat output to extract the first percentage value (miss rate).
    This is robust to any number formatting, event names, or suffixes.
    It simply finds the first occurrence of 'xx.xxx %' in the output.
    """
    # Regex to match a percentage like '23.685 %'
    percent_pattern = re.compile(r'([0-9]+\.[0-9]+)\s*%')
    for line in output.split('\n'):
        match = percent_pattern.search(line)
        if match:
            try:
                return float(match.group(1))
            except ValueError:
                continue
    # If no percentage found, print a warning and return None
    print("Warning: Could not extract percentage from perf output.")
    return None

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
    perf_events = config.get('perf_events', 'branch-instructions,branch-misses')
    events = perf_events.split(',')
    if len(events) != 2:
        print("Error: perf_events must contain exactly 2 events (reference event,miss event)")
        sys.exit(1)

    implementations = [
        "binary_search_baseline",
        "binary_search_std",
        "binary_search_opt1_branchless",
        "binary_search_opt2_branchless2",
        "binary_search_opt3_branchless3",
        "binary_search_opt4_prefetch",
        "binary_search_opt5_eytzinger",
        "binary_search_opt6_eytzinger_branchless",
        "binary_search_opt7_eytzinger_prefetch1",
        "binary_search_opt8_eytzinger_prefetch2",
        "binary_search_opt9_branch_removal"
    ]

    results = {}
    print(f"Starting performance measurements...")
    print(f"Testing input sizes: {input_params}")
    print(f"Events: {events[0]} (reference), {events[1]} (misses)")

    for impl in implementations:
        print(f"Testing {impl}...")
        impl_results = []
        for size in input_params:
            print(f"  Size {size}...")
            perf_output = run_perf(size, impl, perf_events)
            if perf_output:
                miss_rate = parse_perf_output(perf_output, perf_events)
                if miss_rate is not None:
                    impl_results.append(miss_rate)
                    print(f"    ✓ {miss_rate:.3f}% {events[1]} rate")
                else:
                    print(f"    ✗ Failed to parse perf output")
            else:
                print(f"    ✗ Failed to run perf")
        results[impl] = impl_results

    # Create output JSON
    output = {
        "input_params": input_params,
        "result": results,
        "input_param_meaning": config.get("input_param_meaning", "size of input array"),
        "test_name": config.get("test_name", "Binary Search")
    }

    # Get output file path from config or use default
    output_file = config.get("output_file_path", "binary_search_profile_results.json")
    with open(output_file, 'w') as f:
        json.dump(output, f, indent=4)

    print(f"\nResults saved in: {output_file}")
    print("Done!")

if __name__ == "__main__":
    main()