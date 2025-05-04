import json
import matplotlib.pyplot as plt
import argparse
import numpy as np
from matplotlib.ticker import ScalarFormatter

# Usage:
# Original log scale version
# python3 plot.py input.json output.png

# Modified linear scale version
# python3 plot.py input.json output.png --linear

# Modified version
# python3 plot.py input.json output.png --log-x-linear-y

# Common configuration and functions
DISTINCT_COLORS = [
    'b', 'g', 'r', 'c', 'm', 'y', 'k', 'orange',
    'purple', 'brown', 'indigo', 'teal', 'violet',
    'olive', 'turquoise', 'chocolate', 'gold', 'salmon',
    'darkgreen', 'darkblue', 'darkred', 'darkcyan', 'darkmagenta',
    'darkyellow', 'darkgray', 'lightgreen', 'lightblue', 'lightcoral',
    'lightcyan', 'lightpink', 'lightsalmon', 'lightseagreen', 'lightskyblue',
    'lightslategray', 'lightsteelblue', 'lime', 'maroon', 'mediumaquamarine',
    'mediumblue', 'mediumorchid', 'mediumpurple', 'mediumseagreen', 'mediumslateblue',
    'mediumspringgreen', 'mediumturquoise', 'mediumvioletred', 'midnightblue', 'navy'
]

def load_data(file_path):
    """Load JSON data from file"""
    with open(file_path, 'r') as f:
        return json.load(f)

def prepare_data(data):
    """Extract input parameters, result names and values from data"""
    if 'cache miss rate' in data:
        input_params = data['cache miss rate']['input_params']
        result = data['cache miss rate']['result']
    else:
        input_params = data['input_params']
        result = data['result']
    return input_params, list(result.keys()), list(result.values())

def generate_distinct_colors(n):
    """Generate distinct colors for the plot"""
    if n <= len(DISTINCT_COLORS):
        return DISTINCT_COLORS[:n]
    return [DISTINCT_COLORS[i % len(DISTINCT_COLORS)] for i in range(n)]

def setup_common_plot_elements(data, ax, input_params, result_names, result_values):
    """Configure elements common to both plot versions"""
    # Plot each result with its unique color
    colors = generate_distinct_colors(len(result_values))
    for i, result in enumerate(result_values):
        ax.plot(input_params, result, label=result_names[i], marker='o', linewidth=2, color=colors[i])
    
    # Set common labels and title
    ax.set_xlabel(data['input_param_meaning'], fontsize=12)
    
    # Set y-axis label based on unit and ylabel configuration
    if 'unit' in data and data['unit'] == 'percentage':
        ylabel = data.get('ylabel')
        ax.set_ylabel(ylabel, fontsize=12)
        # Set y-axis formatter to show three decimal places for percentage
        ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{x:.3f}%'))  # Show 3 decimal places
    else:
        ax.set_ylabel(f"Execution Time ({data.get('unit', 'ns')})", fontsize=12)
    
    ax.set_title(f"Performance Comparison: {data['test_name']}", pad=20)
    
    # Add legend
    ax.legend(
        bbox_to_anchor=(1.05, 1),
        loc='upper left',
        fontsize=10,
        frameon=False,
        ncol=1
    )
    
    # Add grid
    ax.grid(True, which="both", ls="--", alpha=0.5)

def create_log_scale_plot(data, input_params, result_names, result_values):
    """Create plot with logarithmic scale (original version)"""
    fig, ax = plt.subplots(figsize=(12, 6))
    setup_common_plot_elements(data, ax, input_params, result_names, result_values)
    
    # Set logarithmic scale
    ax.set_xscale('log')
    ax.set_yscale('log')
    
    # The y-axis percentage formatter is now set in setup_common_plot_elements
    return fig

def create_linear_scale_plot(data, input_params, result_names, result_values):
    """Create plot with linear scale and all x-axis labels (modified version)"""
    fig, ax = plt.subplots(figsize=(12, 6))
    setup_common_plot_elements(data, ax, input_params, result_names, result_values)
    
    # Set linear scale
    ax.set_xscale('linear')
    ax.set_yscale('linear')
    
    # Force ticks at every data point
    ax.set_xticks(input_params)
    ax.set_xticklabels([str(x) for x in input_params], rotation=45, ha='right')
    
    # Auto-adjust y-axis ticks
    ax.yaxis.set_major_locator(plt.MaxNLocator(integer=False, prune=None))
    
    # The y-axis percentage formatter is now set in setup_common_plot_elements
    return fig

def create_log_x_linear_y_plot(data, input_params, result_names, result_values):
    """Create plot with logarithmic scale on x-axis and linear scale on y-axis
    
    The x-axis will display ticks in scientific notation (10^1, 10^2, etc.)
    while the y-axis remains in linear scale.
    """
    fig, ax = plt.subplots(figsize=(12, 6))
    setup_common_plot_elements(data, ax, input_params, result_names, result_values)
    
    # Set logarithmic scale for x-axis and linear scale for y-axis
    ax.set_xscale('log')
    ax.set_yscale('linear')
    
    # Format x-axis ticks to show scientific notation
    ax.xaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'$10^{{{int(np.log10(x))}}}$'))
    
    # Auto-adjust y-axis ticks
    ax.yaxis.set_major_locator(plt.MaxNLocator(integer=False, prune=None))
    
    # The y-axis percentage formatter is now set in setup_common_plot_elements
    return fig

def plot_data(data, input_params, result_names, result_values, output_image_path, scale_type='log'):
    """Create and save the plot using specified scale type
    
    Args:
        data: Dictionary containing plot data
        input_params: List of input parameters
        result_names: List of result names
        result_values: List of result values
        output_image_path: Path to save the output image
        scale_type: Type of scale to use ('log', 'linear', or 'log_x_linear_y')
    """
    if scale_type == 'linear':
        fig = create_linear_scale_plot(data, input_params, result_names, result_values)
    elif scale_type == 'log_x_linear_y':
        fig = create_log_x_linear_y_plot(data, input_params, result_names, result_values)
    else:  # default to log scale
        fig = create_log_scale_plot(data, input_params, result_names, result_values)
    
    # Adjust layout and save
    fig.tight_layout(rect=[0, 0, 0.85, 1])
    fig.savefig(output_image_path, bbox_inches='tight', dpi=300)
    plt.close(fig)

def main(input_file, output_file, scale_type='log'):
    """Main function to load data and generate plot
    
    Args:
        input_file: Path to input JSON file
        output_file: Path to save output image
        scale_type: Type of scale to use ('log', 'linear', or 'log_x_linear_y')
    """
    data = load_data(input_file)
    plot_data(data, *prepare_data(data), output_file, scale_type)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate performance comparison plots.')
    parser.add_argument("input_file", help="Path to input JSON file")
    parser.add_argument("output_file", help="Path to save output image")
    parser.add_argument("--linear", action="store_true", 
                        help="Use linear scale with all x-axis labels")
    parser.add_argument("--log-x-linear-y", action="store_true",
                        help="Use logarithmic scale on x-axis and linear scale on y-axis")
    
    args = parser.parse_args()
    
    # Determine scale type based on arguments
    scale_type = 'log'  # default
    if args.linear:
        scale_type = 'linear'
    elif args.log_x_linear_y:
        scale_type = 'log_x_linear_y'
    
    main(args.input_file, args.output_file, scale_type)