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
OPTS=$(getopt -o vhxd:n:t: --long verbose,help,xml,directory:,no-logs,test: -n "$(basename "$0")" -- "$@")
if [ $? != 0 ]; then echo; Usage; fi
eval set -- "$OPTS"

# Default values
OUTPUT_DIR=""
NO_LOGS=false
GENERATE_XML=""
TEST_TO_EXECUTE=""
VERBOSE=""

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

# Execute each test
for ref_image in "${test_files[@]}"; do
    # Extract app name from filename (remove path and .png extension)
    app_name=$(basename "$ref_image" .png)

    echo -e "${Bold}Executing: $app_name${Clear}"

    # Determine log file for this app
    app_log_file="$OUTPUT_DIR/logs/${app_name}.log"

    # Call capture.sh with the app name and output directory
    if [ "$NO_LOGS" = true ]; then
        "$SCRIPT_DIR/capture.sh" -d "$OUTPUT_DIR" -n "$app_name" >> /dev/null 2>&1
    else
        "$SCRIPT_DIR/capture.sh" -d "$OUTPUT_DIR" "$app_name" >> "$OUTPUT_DIR/logs/capture-sh.log" 2>&1
    fi

    # Compare captured image with reference using dali-image-compare
    if [ "$NO_LOGS" = true ]; then
        dali-image-compare "$ref_image" "$OUTPUT_DIR/${app_name}.png" >> /dev/null 2>&1
        compare_result=$?
    else
        dali-image-compare "$ref_image" "$OUTPUT_DIR/${app_name}.png" >> "$app_log_file" 2>&1
        compare_result=$?
    fi

    # Check result and update counters
    if [ $compare_result -eq 0 ]; then
        echo -e "  ${Green}Passed${Clear}"
        testOutput="$testOutput $app_name,Passed"
        ((num_passes++))
    else
        echo -e "  ${Red}Failed${Clear}"
        testOutput="$testOutput $app_name,Failed"
        ((num_fails++))
    fi
done

# Calculate percentages
percent_passing=$(printf "%.2f\n" "$((10000 * $num_passes / $num_tests ))e-2")
percent_failing=$(printf "%.2f\n" "$((10000 * $num_fails / $num_tests ))e-2")

# Output the summary of test results
TestOutputColor=${Green}
if [[ ! "$num_passes" = "$num_tests" ]] ; then TestOutputColor=${Red}; fi

echo
echo -e "${Bold}Test Summary:${Clear}"
echo -e "  Total tests: $num_tests"
echo -e "  Number of test passes: ${Bold}$num_passes ($percent_passing%)${Clear}"
echo -e "  ${TestOutputColor}Number of test failures: ${Bold}$num_fails${Clear}"
echo -e "  Output directory: $OUTPUT_DIR"

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
