#!/bin/bash

# Usage Function
Usage()
{
    echo "Usage: $(basename ${BASH_SOURCE[0]}) [OPTIONS]"
    echo " Optional Options:"
    # Just do a simple grep of this file so we can keep the command with the comment together
    grep ". )\ #" ${BASH_SOURCE[0]} | sed 's/# //' | awk -F ")" '{ printf "%-30s %s\n", $1, $2 }'
    exit 0
}

# Initialise the options
OPTS=$(getopt -o vhxt:d: --long directory:,verbose,help,xml,test: -n "$(basename "$0")" -- "$@")
if [ $? != 0 ]; then echo; Usage; fi
eval set -- "$OPTS"

REDIRECT_OUTPUT="> /dev/null 2>&1"
GENERATE_XML=
TEST_TO_EXECUTE=
export DALI_DISABLE_PARTIAL_UPDATE=1

dir=""

# Go through all the options
if [[ $* > 1 ]] ; then
    while true;
    do
        case "$1" in
            -d|--directory ) # Outputs captured images to this directory
                dir="--directory $2"
                shift 2
                ;;
            -v|--verbose ) # Verbose output for every test case
                REDIRECT_OUTPUT=
                shift
                ;;

            -h|--help ) # Help
                shift
                Usage
                ;;

            -x|--xml ) # Outputs visual-tests-results.xml with the test re
                GENERATE_XML=1
                shift
                ;;
            -t|--test ) # Executes a single test
                TEST_TO_EXECUTE="$2"
                shift 2
                ;;
            -- )
                shift
                ;;

            * )
                break
                ;;

        esac
    done
fi

# Formatting Codes
Bold="\e[1m"
Green='\e[0;32m'
Red='\e[0;31m'
Clear='\e[0m'

# Retrieve all the installed tests
scriptLocation=$(dirname ${BASH_SOURCE[0]})
tests=$(ls $scriptLocation/visual-tests)
num_tests=$(echo $tests | wc -w)
num_passes=0
num_fails=0

testOutput=""

DEBUG=""
#DEBUG=gdb --args

if [[ "$TEST_TO_EXECUTE" != "" ]] ; then
  tests=$TEST_TO_EXECUTE
  num_tests=1
fi

# Execute each test executable in turn
for i in $tests ; do
    test=$(basename $i).test
    dimensions=$($test --get-dimensions 2>/dev/null)
    command="timeout 3m xvfb-run -s \"-screen 0 $dimensions -fbdir /var/tmp\" $DEBUG $test --fb $dir ${REDIRECT_OUTPUT}"
    echo -e "${Bold}Executing: $command"
    # Run a second time if failed the first as it seems to fail incorrectly from time to time
    eval $command || eval $command

    percent=$?
    # Check the test result
    if [ "$percent" != "0" ]; then
        echo "$test Failed ($percent % match)"
        testOutput="$testOutput $test,Failed"
        ((num_fails++))
    else
        echo "$test Passed"
        testOutput="$testOutput $test,Passed"
        ((num_passes++))
    fi

    #read -p "Waiting..>" aline
done

# Output the summary of test result
TestOutputColor=${Green}
if [[ ! "$num_passes" = "$num_tests" ]] ; then TestOutputColor=${Red}; fi
percent_passing=$(printf %.2f\\n "$((10000 * $num_passes / $num_tests ))e-2")
percent_failing=$(printf %.2f\\n "$((10000 * $num_fails / $num_tests ))e-2")
echo
echo -e "${Bold}Test Summary:${Clear}"
echo -e "  Total tests: $num_tests"
echo -e "  Number of test passes: ${Bold}$num_passes ($percent_passing%)${Clear}"
echo -e "  ${TestOutputColor}Number of test failures: ${Bold}$num_fails ${Clear}"

# Create an XML file with all the output
if [[ "$GENERATE_XML" = "1" ]]
then
    xmlOutputFile=visual-tests-results.xml
    echo -e "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $xmlOutputFile
    echo -e "<visual_tests>" >> $xmlOutputFile
    echo -e "\t<summary>" >> $xmlOutputFile
    echo -e "\t\t<total_tests>$num_tests</total_tests>" >> $xmlOutputFile
    echo -e "\t\t<pass_tests>$num_passes</pass_tests>" >> $xmlOutputFile
    echo -e "\t\t<pass_rate>$percent_passing</pass_rate>" >> $xmlOutputFile
    echo -e "\t\t<fail_tests>$num_fails</fail_tests>" >> $xmlOutputFile
    echo -e "\t\t<fail_rate>$percent_failing</fail_rate>" >> $xmlOutputFile
    echo -e "\t</summary>" >> $xmlOutputFile
    echo -e "\t<tests>" >> $xmlOutputFile
    for test in $testOutput
    do
        testName=$(echo $test | cut -d, -f 1)
        testResult=$(echo $test | cut -d, -f 2)
        echo -e "\t\t<test>" >> $xmlOutputFile
        echo -e "\t\t\t<name>$testName</name>" >> $xmlOutputFile
        echo -e "\t\t\t<result>$testResult</result>" >> $xmlOutputFile
        echo -e "\t\t</test>" >> $xmlOutputFile
    done
    echo -e "\t</tests>" >> $xmlOutputFile
    echo -e "</visual_tests>" >> $xmlOutputFile
fi

# If we have failures, this will exit this script with 1 otherwise it'll be 0 (success)
[[ $num_fails -eq 0 ]]
