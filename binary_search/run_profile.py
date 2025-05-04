#!/usr/bin/env python3

import subprocess
import sys
import os
import json
import re
from collections import defaultdict
import tempfile


def run_perf_record(config_path, perf_events, binary_path, id_str):
    """Run perf record for a given input parameter and return the perf.data path."""
    cmd = [
        "perf", "record", "-e", perf_events,
        "--", binary_path, config_path
    ]
    print(f"Running perf record command: {' '.join(cmd)}")  # Debug print
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"Error running perf record.")
        return None
    if not os.path.exists("perf.data"):
        print(f"perf.data was not created for input {id_str}. The program may have crashed.")
        return None
    return "perf.data"

def run_perf_report(perf_data_path):
    """Run perf report --stdio on the given perf.data file and return the output."""
    cmd = ["perf", "report", "--stdio", "-i", perf_data_path]
    print(f"Running perf report command: {' '.join(cmd)}")  # Debug print
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if result.returncode != 0:
        print(f"Error running perf report: {result.stderr}")
        return None
    return result.stdout

def parse_perf_report_miss_rate(report, event_names, function_patterns, input_param):
    """
    Parse perf report output and compute per-function miss rate.
    Returns a dict: {function_name: miss_rate}
    """
    lines = report.splitlines()
    event_sections = []
    # Find all event section headers and their indices
    for i, line in enumerate(lines):
        m = re.match(r"# Samples: .* of event '([^']+)'", line)
        if m:
            event_sections.append((i, m.group(1)))
    if len(event_sections) < 2:
        print("Warning: Not enough event sections found in report.")
        return {}

    # Map event name to (section index, total count)
    event_info = {}
    for idx, event in event_sections:
        # Find total count
        total = None
        for j in range(idx, min(idx+5, len(lines))):
            m = re.match(r"# Event count \(approx\.\): ([0-9]+)", lines[j])
            if m:
                total = int(m.group(1))
                break
        event_info[event] = (idx, total)

    # Find the two events that match event_names (allow for :u or :k suffix)
    def match_event(target, candidates):
        for c in candidates:
            if c.startswith(target):
                return c
        return None

    event1 = match_event(event_names[0], event_info.keys())
    event2 = match_event(event_names[1], event_info.keys())
    if not event1 or not event2:
        print("Warning: Could not match both events in report.")
        return {}

    # Parse per-function percentages for each event
    def parse_table(idx, event_name, input_param):
        # print(f"DEBUG: Parsing table for event {event_name} with input_param {input_param}")
        table = {}
        found_funcs = []
        # Find the separator line of dots
        table_start = None
        for i in range(idx, len(lines)):
            if re.match(r"^#? ?\.{2,}", lines[i]):
                table_start = i + 1
                break
        if table_start is None:
            print("DEBUG: No table found for event section starting at index", idx)
            return table
        # Only consider lines until the next blank or comment line
        table_lines = [line for line in lines[table_start:] if not line.strip().startswith("#")]
        for line in lines[table_start:]:
            if not line.strip() or line.strip().startswith("#"):
                break
            table_lines.append(line)
        # Write table_lines to a uniquely named file for debugging
        grep_table_path = f'grep_table_{event_name}_{input_param}.txt'
        with open(grep_table_path, 'w') as tmpf:
            for l in table_lines:
                tmpf.write(l + '\n')
        # For each function, grep for the line and extract the first float
        for pattern, canonical in function_patterns.items():
            try:
                result = subprocess.run(['grep', pattern, grep_table_path], stdout=subprocess.PIPE, universal_newlines=True)
                if result.stdout:
                    line = result.stdout.splitlines()[0]
                    m = re.search(r"([0-9]+(?:\.[0-9]+)?)", line)
                    if m:
                        percent = float(m.group(1))
                        table[canonical] = percent
                        found_funcs.append(pattern)
                else:
                    # print(f"DEBUG: [GREP] No match found for pattern '{pattern}'")
                    pass
            except Exception as e:
                # print(f"DEBUG: [GREP] Error for pattern '{pattern}': {e}")
                pass
        # print(f"DEBUG: All function names found in table: {found_funcs}")
        return table

    table1 = parse_table(event_info[event1][0], event1, input_param)
    table2 = parse_table(event_info[event2][0], event2, input_param)
    total1 = event_info[event1][1]
    total2 = event_info[event2][1]

    # print(f"Matched event1: {event1} (total={total1}), event2: {event2} (total={total2})")
    # print(f"Per-function {event1} percentages: {table1}")
    # print(f"Per-function {event2} percentages: {table2}")

    # Compute miss rate for each function
    miss_rates = {}
    for func in function_patterns.values():
        if func in table1 and func in table2 and total1 and total2:
            # print(f"RAW LOAD RATIO for {func}: {table1[func]}%")
            # print(f"RAW MISS RATIO for {func}: {table2[func]}%")
            # print(f"RAW EVENT RATIO for {func}: misses_f={misses_f}, loads_f={loads_f}, ratio={misses_f / loads_f if loads_f > 0 else 'inf'}")
            loads_f = table1[func] / 100 * total1
            misses_f = table2[func] / 100 * total2
            miss_rate = misses_f / loads_f if loads_f > 0 else 0.0
            # print(f"Function: {func}, loads_f: {loads_f}, misses_f: {misses_f}, miss_rate: {miss_rate}")
            miss_rates[func] = miss_rate
        else:
            # print(f"Function: {func} missing data (loads: {func in table1}, misses: {func in table2}, total1: {total1}, total2: {total2})")
            miss_rates[func] = 0.0
    return miss_rates

def main():
    # Read config
    if len(sys.argv) != 2:
        print("Usage: python run_profile.py <config_file>")
        sys.exit(1)
    config_file = sys.argv[1]
    with open(config_file, "r") as f:
        config = json.load(f)

    input_params = config["input_params"]
    perf_events = config.get("perf_events", "cache-references,cache-misses")
    output_file = config.get("output_file_path", "binary_search_output.json")
    binary_path = config.get("binary_path", "./binary_search")

    # Only aligned functions (as in your current binary_search.cpp)
    function_patterns = {
        "binary_search_baseline": "binary_search_baseline_aligned",
        "binary_search_std": "binary_search_std_aligned",
        "binary_search_opt1_branchless": "binary_search_opt1_branchless_aligned",
        "binary_search_opt2_branchless2": "binary_search_opt2_branchless2_aligned",
        "binary_search_opt3_branchless3": "binary_search_opt3_branchless3_aligned",
        "binary_search_opt4_prefetch": "binary_search_opt4_prefetch_aligned",
        "binary_search_opt5_eytzinger": "binary_search_opt5_eytzinger_aligned",
        "binary_search_opt6_eytzinger_branchless": "binary_search_opt6_eytzinger_branchless_aligned",
        "binary_search_opt7_eytzinger_prefetch1": "binary_search_opt7_eytzinger_prefetch1_aligned",
        "binary_search_opt8_eytzinger_prefetch2": "binary_search_opt8_eytzinger_prefetch2_aligned",
        "binary_search_opt9_branch_removal": "binary_search_opt9_branch_removal_aligned"
    }

    # Use the last event as the "miss" event for reporting
    events = [e.strip() for e in perf_events.split(",")]
    miss_event = events[-1]

    # Prepare results structure
    results = {canonical: [] for canonical in function_patterns.values()}

    for input_param in input_params:
        config_filename = f"input_{input_param}.json"
        single_config = dict(config)
        single_config["input_params"] = [input_param]
        with open(config_filename, "w") as f:
            json.dump(single_config, f)
        print(f"\nProfiling input_param={input_param} ...")
        perf_data = run_perf_record(config_filename, perf_events, binary_path, input_param)
        if not perf_data:
            print(f"Skipping input_param={input_param} due to error.")
            os.remove(config_filename)
            continue
        report = run_perf_report(perf_data)
        if not report:
            print(f"Skipping input_param={input_param} due to error.")
            os.remove(perf_data)
            os.remove(config_filename)
            continue
        func_rates = parse_perf_report_miss_rate(report, events, function_patterns, input_param)
        # print(f"Computed per-function miss rates for input_param={input_param}: {func_rates}")  # Debug print
        for canonical in function_patterns.values():
            results[canonical].append(func_rates.get(canonical, 0.0))
        # Clean up
        os.remove(perf_data)
        os.remove(config_filename)

    # Compose output JSON
    output = {
        "input_param_meaning": config.get("input_param_meaning", "Array Size"),
        "input_params": input_params,
        "result": results,
        "test_name": config.get("test_name", "Binary Search"),
        "unit": "%"
    }
    with open(output_file, "w") as f:
        json.dump(output, f, indent=4)
    print(f"\nResults saved in: {output_file}")

if __name__ == "__main__":
    main()