#!/bin/bash

FILE="$1"
echo "/// registers count"
echo "pub const REG_COUNT: usize = 32;"
echo ""

for reg in `cat $FILE`; do
  REG_NAME=`echo "$reg" | cut -d',' -f1`
  REG_INDEX=`echo "$reg" | cut -d',' -f2`
  echo "pub const $REG_NAME: usize = $REG_INDEX;"
done