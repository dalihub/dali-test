#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Get the script name without extension for default directory naming
SCRIPT_NAME=$(basename "$0" .sh)

# Reference directory containing the baseline images
REFERENCE_DIR="$SCRIPT_DIR/reference"

# Usage Function
Usage()
{
    echo "Usage: $(basename ${BASH_SOURCE[0]}) [OPTIONS]"
    echo " Options:"
    # Just do a simple grep of this file so we can keep the command with the comment together
    grep ". )\ #" ${BASH_SOURCE[0]} | sed 's/# //' | awk -F ")" '{ printf "%-30s %s\n", $1, $2 }'
    exit 0
}

# Initialise the options
OPTS=$(getopt -o vhxd:n:t:s --long verbose,help,xml,directory:,no-logs,test:,skip-summary-output -n "$(basename "$0")" -- "$@")
if [ $? != 0 ]; then echo; Usage; fi
eval set -- "$OPTS"

# Default values
OUTPUT_DIR=""
NO_LOGS=false
GENERATE_XML=""
TEST_TO_EXECUTE=""
VERBOSE=""
SKIP_SUMMARY=false
INITIAL_TIMEOUT=10

# Go through all the options
if [[ $# -gt 1 ]] ; then
    while true;
    do
        case "$1" in
            -d|--directory ) # Directory to store output files (default: /tmp/{script-name}-timestamp)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            -n|--no-logs ) # Disable logging (capture.sh output redirected to /dev/null)
                NO_LOGS=true
                shift
                ;;
            -v|--verbose ) # Verbose output for every test case
                VERBOSE=1
                shift
                ;;
            -x|--xml ) # Generate dali-demo-run-results.xml with the test results
                GENERATE_XML=1
                shift
                ;;
            -t|--test ) # Execute a single test by name
                TEST_TO_EXECUTE="$2"
                shift 2
                ;;
            -s|--skip-summary-output ) # Skip displaying the test summary output
                SKIP_SUMMARY=true
                shift
                ;;
            -h|--help ) # Show this help message
                shift
                Usage
                ;;
            -- )
                shift
                break
                ;;
            * )
                break
                ;;
        esac
    done
fi

# Set default output directory if not provided
if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="/tmp/${SCRIPT_NAME}-$(date +%Y%m%d%H%M%S.%N)"
fi

# Create output directory (always needed for screenshots)
mkdir -p "$OUTPUT_DIR"

# Convert OUTPUT_DIR to absolute path to handle any directory changes
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"

# Create logs subdirectory (unless --no-logs is set)
if [ "$NO_LOGS" = false ]; then
    mkdir -p "$OUTPUT_DIR/logs"
fi

# Check if reference directory exists
if [ ! -d "$REFERENCE_DIR" ]; then
    echo "Error: Reference directory not found: $REFERENCE_DIR"
    exit 1
fi

# Formatting Codes
Bold="\e[1m"
Green='\e[0;32m'
Red='\e[0;31m'
Clear='\e[0m'

# Display pool management
DISPLAY_POOL_START=99
DISPLAY_POOL_SIZE=50
LOCK_DIR="$OUTPUT_DIR/.display_locks"

# Initialize lock directory
mkdir -p "$LOCK_DIR"

# Acquire a display from the pool using atomic mkdir for locking
acquire_display() {
    local display
    while true; do
        for ((i=0; i<DISPLAY_POOL_SIZE; i++)); do
            display=$((DISPLAY_POOL_START + i))
            lock_dir="$LOCK_DIR/display_$display"
            # Try to acquire lock atomically using mkdir
            if mkdir "$lock_dir" 2>/dev/null; then
                echo $display
                return 0
            fi
        done
        # No display available, wait a bit and retry
        sleep 0.1
    done
}

# Release a display back to the pool
release_display() {
    local display=$1
    rmdir "$LOCK_DIR/display_$display" 2>/dev/null
}

# Find all PNG files in the reference directory
test_files=()
for ref_image in "$REFERENCE_DIR"/*.png; do
    if [ -f "$ref_image" ]; then
        test_files+=("$ref_image")
    fi
done

if [ ${#test_files[@]} -eq 0 ]; then
    echo "No reference images found in $REFERENCE_DIR"
    exit 1
fi

num_tests=${#test_files[@]}
num_passes=0
num_fails=0
testOutput=""

# If a specific test is requested, filter the list
if [[ "$TEST_TO_EXECUTE" != "" ]] ; then
    filtered_files=()
    for ref_image in "${test_files[@]}"; do
        app_name=$(basename "$ref_image" .png)
        if [[ "$app_name" == "$TEST_TO_EXECUTE" ]]; then
            filtered_files+=("$ref_image")
        fi
    done
    if [ ${#filtered_files[@]} -eq 0 ]; then
        echo "Error: Test '$TEST_TO_EXECUTE' not found"
        exit 1
    fi
    test_files=("${filtered_files[@]}")
    num_tests=${#test_files[@]}
fi

# Arrays to track results
declare -a app_names
declare -a ref_images
declare -a failed_apps
declare -A app_results

# Build list of apps to test
for ref_image in "${test_files[@]}"; do
    app_name=$(basename "$ref_image" .png)
    app_names+=("$app_name")
    ref_images+=("$ref_image")
done

# Phase A: Parallel capture with wait-time 2s
echo -e "${Bold}Phase A: Capturing all tests in parallel (wait-time: ${INITIAL_TIMEOUT}s)${Clear}"

# Function to capture a single app with display management
capture_app() {
    local app_name=$1
    local display=$2
    local app_log_file=$3
    local no_logs_option=$4

    "$SCRIPT_DIR/capture.sh" -d "$OUTPUT_DIR" $no_logs_option --display $display --wait-time $INITIAL_TIMEOUT "$app_name" >> "$app_log_file" 2>&1

    # Release the display when done
    release_display "$display"
}

for i in "${!app_names[@]}"; do
    app_name="${app_names[$i]}"
    
    # Determine log file for this app
    if [ "$NO_LOGS" = true ]; then
      app_log_file=/dev/null
      no_logs_option=
    else
      app_log_file="$OUTPUT_DIR/logs/${app_name}.log"
      no_logs_option="-n"
    fi
    
    # Acquire a display from the pool (blocks until one is available)
    display=$(acquire_display)
    echo -e "  Starting: $app_name (display: $display)"

    # Run capture in background, will release display when done
    capture_app "$app_name" "$display" "$app_log_file" "$no_logs_option" &
done

# Wait for all captures to complete
wait
echo -e "  ${Green}All captures complete${Clear}"

# Phase B: Compare all captures
echo -e "${Bold}Phase B: Comparing all captures${Clear}"
for i in "${!app_names[@]}"; do
    app_name="${app_names[$i]}"
    ref_image="${ref_images[$i]}"
    
    if [ "$NO_LOGS" = true ]; then
      app_log_file=/dev/null
    else
      app_log_file="$OUTPUT_DIR/logs/${app_name}.log"
    fi
    
    dali-image-compare "$ref_image" "$OUTPUT_DIR/${app_name}.png" >> "$app_log_file" 2>&1
    compare_result=$?
    
    if [ $compare_result -eq 0 ]; then
        echo -e "  ${Green}Passed:${Clear} $app_name"
        app_results["$app_name"]="Passed"
        testOutput="$testOutput $app_name,Passed"
        ((num_passes++))
    else
        echo -e "  Failed: $app_name (will retry)"
        failed_apps+=("$app_name")
        app_results["$app_name"]="Failed"
    fi
done

# Phase C: Retry failures with wait-time 5s, then 10s
if [ ${#failed_apps[@]} -gt 0 ]; then
    echo -e "${Bold}Phase C: Retrying ${#failed_apps[@]} failed test(s)${Clear}"
    
    for app_name in "${failed_apps[@]}"; do
        retry_passed=false
        
        # Find index for this app
        for i in "${!app_names[@]}"; do
            if [ "${app_names[$i]}" = "$app_name" ]; then
                ref_image="${ref_images[$i]}"
                break
            fi
        done
        
        if [ "$NO_LOGS" = true ]; then
          app_log_file=/dev/null
          no_logs_option=
        else
          app_log_file="$OUTPUT_DIR/logs/${app_name}.log"
          no_logs_option="-n"
        fi
        
        for wait_time in 2 5 10; do
            if [ "$retry_passed" = true ]; then
                break
            fi
            
            display=$(acquire_display)
            echo -e "  Retrying: $app_name (wait-time: ${wait_time}s, display: $display)"
            "$SCRIPT_DIR/capture.sh" -d "$OUTPUT_DIR" $no_logs_option --display $display --wait-time $wait_time "$app_name" >> "$app_log_file" 2>&1
            release_display "$display"
            
            dali-image-compare "$ref_image" "$OUTPUT_DIR/${app_name}.png" >> "$app_log_file" 2>&1
            compare_result=$?
            
            if [ $compare_result -eq 0 ]; then
                echo -e "  ${Green}Passed on retry:${Clear} $app_name (wait-time: ${wait_time}s)"
                app_results["$app_name"]="Passed"
                testOutput="$testOutput $app_name,Passed"
                ((num_passes++))
                retry_passed=true
            else
                echo -e "  Failed with wait-time ${wait_time}s"
            fi
        done
        
        # If all retries failed, mark as failed
        if [ "$retry_passed" = false ]; then
            echo -e "  ${Red}Failed (all retries exhausted):${Clear} $app_name"
            app_results["$app_name"]="Failed"
            testOutput="$testOutput $app_name,Failed"
            ((num_fails++))
        fi
    done
fi

# Calculate percentages
percent_passing=$(printf "%.2f\n" "$((10000 * $num_passes / $num_tests ))e-2")
percent_failing=$(printf "%.2f\n" "$((10000 * $num_fails / $num_tests ))e-2")

# Output the summary of test results
if [ "$SKIP_SUMMARY" = false ]; then
    TestOutputColor=${Green}
    if [[ ! "$num_passes" = "$num_tests" ]] ; then TestOutputColor=${Red}; fi

    echo
    echo -e "${Bold}Test Summary:${Clear}"
    echo -e "  Total tests: $num_tests"
    echo -e "  Number of test passes: ${Bold}$num_passes ($percent_passing%)${Clear}"
    echo -e "  ${TestOutputColor}Number of test failures: ${Bold}$num_fails${Clear}"
    echo -e "  Output directory: $OUTPUT_DIR"
fi

# Create an XML file with all the output
if [[ "$GENERATE_XML" = "1" ]]
then
    xmlOutputFile="$OUTPUT_DIR/dali-demo-run-results.xml"
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > "$xmlOutputFile"
    echo "<dali_demo_tests>" >> "$xmlOutputFile"
    echo "  <summary>" >> "$xmlOutputFile"
    echo "    <total_tests>$num_tests</total_tests>" >> "$xmlOutputFile"
    echo "    <pass_tests>$num_passes</pass_tests>" >> "$xmlOutputFile"
    echo "    <pass_rate>$percent_passing</pass_rate>" >> "$xmlOutputFile"
    echo "    <fail_tests>$num_fails</fail_tests>" >> "$xmlOutputFile"
    echo "    <fail_rate>$percent_failing</fail_rate>" >> "$xmlOutputFile"
    echo "  </summary>" >> "$xmlOutputFile"
    echo "  <tests>" >> "$xmlOutputFile"
    for test in $testOutput; do
        testName=$(echo $test | cut -d, -f 1)
        testResult=$(echo $test | cut -d, -f 2)
        echo "    <test>" >> "$xmlOutputFile"
        echo "      <name>$testName</name>" >> "$xmlOutputFile"
        echo "      <result>$testResult</result>" >> "$xmlOutputFile"
        echo "    </test>" >> "$xmlOutputFile"
    done
    echo "  </tests>" >> "$xmlOutputFile"
    echo "</dali_demo_tests>" >> "$xmlOutputFile"
    echo ""
    echo "XML results written to: $xmlOutputFile"
fi

# If we have failures, this will exit this script with 1 otherwise it'll be 0 (success)
if [[ $num_fails -gt 0 ]]; then
    exit 1
fi

exit 0
