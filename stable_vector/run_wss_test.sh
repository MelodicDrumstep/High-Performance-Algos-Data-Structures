#!/bin/bash

WSS_EXE_PATH="../../wss/wss.pl"

# Check if argument is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <SV|UM>"
    echo "  SV: Test StableVector"
    echo "  UM: Test UnorderedMap"
    exit 1
fi

# Check if argument is valid
if [ "$1" != "SV" ] && [ "$1" != "UM" ]; then
    echo "Error: Argument must be either 'SV' or 'UM'"
    exit 1
fi

# Check if wss.pl exists
if [ ! -f $WSS_EXE_PATH ]; then
    echo "Error: wss.pl not found. Please make sure wss submodule is initialized."
    exit 1
fi

# Make wss.pl executable
chmod +x $WSS_EXE_PATH

# Start wss_test in background
echo "Starting wss_test with $1..."
./wss_test "$1" &
TEST_PID=$!

# # Wait a moment for the process to start
# sleep 1

# Monitor the process with wss.pl
echo "Monitoring process $TEST_PID with wss.pl..."
echo "Press Ctrl+C to stop monitoring"

# Run wss.pl with cumulative output every 0.1 seconds
$WSS_EXE_PATH -C "$TEST_PID" 0.1

# Wait for the test process to finish
wait $TEST_PID 