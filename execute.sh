#!/bin/bash

# Usage Function
Usage()
{
  echo "Usage: $(basename ${BASH_SOURCE[0]}) [OPTIONS]"
  echo " Optional Options:"
  # Just do a simple grep of this file so we can keep the command with the comment together
  grep ". )\ #" ${BASH_SOURCE[0]} | sed 's/# //' | awk -F ")" '{ printf "%-25s %s\n", $1, $2 }'
  exit 0
}

# Initialise the options
OPTS=$(getopt -o vh --long verbose,help -n "$(basename "$0")" -- "$@")
if [ $? != 0 ]; then echo; Usage; fi
eval set -- "$OPTS"

REDIRECT_OUTPUT="> /dev/null 2>&1"

# Go through all the options
if [[ $* > 1 ]] ; then
  while true;
  do
    case "$1" in
      -v|--verbose ) # Verbose output for every test case
        REDIRECT_OUTPUT=
        shift
        ;;

      -h|--help ) # Help
        shift
        Usage
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
tests=$( ls -d $DESKTOP_PREFIX/bin/*.test )
num_tests=${#tests[@]}
num_passes=0
num_fails=0

# Execute each test executable in turn
for i in $tests ; do
  test=$(basename $i)
  echo -e "${Bold}Executing: $test${Clear}"
  command="$i ${REDIRECT_OUTPUT}"
  eval $command

  # Check the test result
  if [ "$?" != "0" ]; then
    echo "$test Failed"
    ((num_fails++))
  else
    echo "$test Passed"
    ((num_passes++))
  fi
done

# Output the summary of test result
TestOutputColor=${Green}
if [[ ! "$num_passes" = "$num_tests" ]] ; then TestOutputColor=${Red}; fi
percent_passing=$(printf %.2f\\n "$((10000 * $num_passes / $num_tests ))e-2")
echo
echo -e "${Bold}Test Summary:${Clear}"
echo -e "  Total tests: $num_tests"
echo -e "  Number of test passes: ${Bold}$num_passes ($percent_passing%)${Clear}"
echo -e "  ${TestOutputColor}Number of test failures: ${Bold}$num_fails ${Clear}"

