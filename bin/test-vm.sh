#!/bin/bash

VM="$1"

DEBUG=0

COLOR_NC='\033[0m'
COLOR_GREEN='\033[0;32m'
COLOR_RED='\033[0;31m'
COLOR_BLUE='\033[0;34m'
COLOR_YELLOW='\033[0;33m'

FAIL=0
function _test {
    echo -ne " * $1($2):  "

    failure=0
    result=$("$VM" --print-json --register-values $2 $1 | jq $4)
    if [ "$result" == "$3" ]; then
      echo -e "${COLOR_GREEN}Success$COLOR_NC"
    else
      FAIL=1
      failure=1
      echo -e "${COLOR_RED}Failure$COLOR_NC expected $COLOR_YELLOW$3$COLOR_NC got $COLOR_YELLOW$result$COLOR_NC"
    fi

    if [[ $DEBUG -eq 1 || $failure -eq 1 ]]; then
      echo -e "     ${COLOR_BLUE}${VM} --print-json --register-values $2 $1 | jq -r '$4$COLOR_NC'"
    fi
}

     _test target/fibonacci.bin 0:0 0 ".cpu.registers.general[3]" \
  && _test target/fibonacci.bin 0:1 1 ".cpu.registers.general[3]" \
  && _test target/fibonacci.bin 0:16 987 ".cpu.registers.general[3]" \
  && _test target/fibonacci_rec.bin 0:16 987 ".cpu.registers.general[3]" \
  && _test target/fact.bin 0:5 120 ".cpu.registers.general[3]" \
  && _test target/jumps.bin 0:0 7 ".cpu.registers.general[0]"

exit $FAIL
