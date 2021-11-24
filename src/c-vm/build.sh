#!/bin/bash

DEBUG=0

COLOR_NC='\033[0m'
COLOR_GREEN='\033[0;32m'
COLOR_RED='\033[0;31m'
COLOR_BLUE='\033[0;34m'
COLOR_YELLOW='\033[0;33m'


function _test {
    echo -ne " * $1($2):  "

    failure=0
    result=$(cmake-build-debug/c_vm --print-json --register-values $2 $1 | jq $4)
    if [ "$result" == "$3" ]; then
      echo -e "${COLOR_GREEN}Success$COLOR_NC"
    else
      failure=1
      echo -e "${COLOR_RED}Failure$COLOR_NC expected $COLOR_YELLOW$3$COLOR_NC got $COLOR_YELLOW$result$COLOR_NC"
    fi

    if [[ $DEBUG -eq 1 || $failure -eq 1 ]]; then
      echo -e "     ${COLOR_BLUE}cmake-build-debug/c_vm --print-json --register-values $2 $1 | jq -r $4$COLOR_NC"
    fi
}

cmake --build cmake-build-debug \
  && ctest --test-dir cmake-build-debug --output-on-failure \
  && _test programs/fibonacci.bin 0:0 0 ".cpu.registers.general[3]" \
  && _test programs/fibonacci.bin 0:1 1 ".cpu.registers.general[3]" \
  && _test programs/fibonacci.bin 0:16 987 ".cpu.registers.general[3]" \
  && _test programs/store_load.bin 1:12 12 ".cpu.registers.general[2]"
