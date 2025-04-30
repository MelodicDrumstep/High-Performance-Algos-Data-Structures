#!/bin/bash

# Define the root directory
ROOT_DIR="$(pwd)"

# List of directories to check
DIRECTORIES=(
    "conditional_sum"
    "division"
    "factorization"
    "gcd"
    "matmul"
    "prefix_sum"
    "arg_min"
    "B_tree"
)

# Function to check compilation for a directory
check_compilation() {
    local dir=$1
    echo "Checking compilation for $dir..."
    
    # Change to the directory
    cd "$ROOT_DIR/$dir" || {
        echo "Error: Could not change to directory $dir"
        return 1
    }
    
    # Create build directory if it doesn't exist
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    # Change to build directory
    cd build || {
        echo "Error: Could not change to build directory in $dir"
        return 1
    }
    
    # Run CMake and make
    if cmake .. && make -j$(nproc); then
        echo "✅ Compilation successful for $dir"
        return 0
    else
        echo "❌ Compilation failed for $dir"
        return 1
    fi
}

# Main execution
echo "Starting compilation check for all directories..."
echo "----------------------------------------"

# Initialize counters
total_dirs=${#DIRECTORIES[@]}
successful=0
failed=0

# Check each directory
for dir in "${DIRECTORIES[@]}"; do
    if check_compilation "$dir"; then
        ((successful++))
    else
        ((failed++))
    fi
    echo "----------------------------------------"
done

# Print summary
echo "Compilation Summary:"
echo "Total directories checked: $total_dirs"
echo "Successful compilations: $successful"
echo "Failed compilations: $failed"

# Return appropriate exit code
if [ $failed -eq 0 ]; then
    echo "All compilations successful!"
    exit 0
else
    echo "Some compilations failed!"
    exit 1
fi 