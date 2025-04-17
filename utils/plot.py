import json
import matplotlib.pyplot as plt
import argparse
import numpy as np

# Expanded list of distinct, readable colors
DISTINCT_COLORS = [
    'b',  # blue
    'g',  # green
    'r',  # red
    'c',  # cyan
    'm',  # magenta
    'y',  # yellow
    'k',  # black
    'orange',
    'purple',
    'brown',
    'indigo',
    'teal',
    'violet',
    'olive',
    'turquoise',
    'chocolate',
    'gold',
    'salmon'
]

def load_data(file_path):
    with open(file_path, 'r') as f:
        return json.load(f)

def prepare_data(data):
    input_params = data['input_params']
    result = data['result']
    return input_params, list(result.keys()), list(result.values())

def generate_distinct_colors(n):
    """Generate distinct colors for the plot."""
    if n <= len(DISTINCT_COLORS):
        return DISTINCT_COLORS[:n]
    else:
        # If more colors are needed, cycle through the DISTINCT_COLORS list
        return [DISTINCT_COLORS[i % len(DISTINCT_COLORS)] for i in range(n)]

def plot_data(data, input_params, result_names, result_values, output_image_path):
    plt.figure(figsize=(12, 6))
    
    # Determine how many results there are
    num_results = len(result_values)
    colors = generate_distinct_colors(num_results)

    # Plot each result with its unique color
    for i, result in enumerate(result_values):
        plt.plot(input_params, result, label=result_names[i], marker='o', linewidth=2, color=colors[i])

    plt.xlabel(data['input_param_meaning'], fontsize=12)
    plt.ylabel(f"Execution Time ({data['unit']})", fontsize=12)
    plt.title(f"Performance Comparison: {data['test_name']}", pad=20)
    
    plt.legend(
        bbox_to_anchor=(1.05, 1),
        loc='upper left',
        fontsize=10,
        frameon=False,
        ncol=1
    )
    
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.tight_layout(rect=[0, 0, 0.85, 1])

    plt.savefig(output_image_path, bbox_inches='tight', dpi=300)
    plt.close()

def main(input_file, output_file):
    data = load_data(input_file)
    plot_data(data, *prepare_data(data), output_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input_file")
    parser.add_argument("output_file")
    args = parser.parse_args()
    main(args.input_file, args.output_file)